// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BaseCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
// FInputActionValue 的完整定义（头文件里只前向声明，cpp 这里才需要实体类型来调 .Get<>()）
#include "InputActionValue.h"

// 武器碰撞 + 攻击动画
#include "Character/WeaponColliderComponent.h"
#include "Animation/AnimMontage.h"

// [DEBUG] 项目自带的屏幕 + Log 双输出工具
#include "FrontendDebugHelper.h"


// =====================================================================================
// 构造函数
// =====================================================================================
ABaseCharacter::ABaseCharacter()
{
	// 关闭 Tick：本类基础逻辑不依赖每帧；如派生类需要可单独打开
	PrimaryActorTick.bCanEverTick = false;

	// ---------------- Mesh 摆位 ----------------
	// UE 默认 Capsule 中心在 (0, 0, 0)，半高 88（默认 Capsule HalfHeight）。
	// 而 Skeletal Mesh 的根骨通常在脚底原点。
	// 沿 Z 偏移 −90 让脚底刚好对齐 Capsule 底部（贴地不悬空、不嵌地）。
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	// UE 角色 Capsule 的「正前」是 +X；多数 Skeletal Mesh 资产建模时朝向是 −Y。
	// 旋转 Yaw −90° 让模型正面对齐 Capsule 正前方，否则模型会侧着走路。
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	// ---------------- 默认旋转跟随策略 ----------------
	// 初始为「自由模式」：身体朝速度方向旋转，Yaw 不跟随 Controller。
	// 玩家通过 ToggleStrafeMode() 切到「八向模式」时再翻转。
	bUseControllerRotationPitch = false;  // Pitch 永远不跟，避免身体上下倾斜
	bUseControllerRotationYaw   = false;  // 自由模式下 Yaw 由速度方向控制
	bUseControllerRotationRoll  = false;  // Roll 永远不跟，避免身体侧翻

	// ---------------- 移动组件参数 ----------------
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();

	// 自由模式核心：身体每帧旋转到当前速度方向，配合 RotationRate 控制旋转平滑度
	MoveComp->bOrientRotationToMovement = true;

	// 旋转速率：仅 Yaw 540°/s ≈ 1/3 秒转 180°，转身够快但不会瞬移
	MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f);

	// 跳跃初速 700 cm/s，配合默认重力 980 大约能跳 25 cm 高
	MoveComp->JumpZVelocity = 700.f;

	// 空中输入响应力度，0 = 完全无控制，1 = 与地面相同；0.35 是 UE 默认值
	MoveComp->AirControl = 0.35f;

	// 行走最大速度（cm/s）。500 ≈ 普通跑步速度
	MoveComp->MaxWalkSpeed = 500.f;

	// 模拟摇杆推得很轻时的最低速度。低于此值视为 0（防止微小漂移）
	MoveComp->MinAnalogWalkSpeed = 20.f;

	// 行走停下的减速度：值越大刹车越快。2000 ≈ 0.25 秒从满速停下
	MoveComp->BrakingDecelerationWalking = 2000.f;

	// [DEBUG] 不依赖 GEngine：构造函数会被 CDO 加载和 SpawnActor 各跑一次，
	//        在 Output Log 搜 "[BC.Ctor]" 能直接证明 ABaseCharacter 类有没有被实例化
	UE_LOG(LogTemp, Warning, TEXT("[BC.Ctor] ABaseCharacter constructed, class=%s"), *GetClass()->GetName());
}

// =====================================================================================
// Strafe（八向）模式切换
// =====================================================================================

void ABaseCharacter::ToggleStrafeMode()
{
	// 翻转：用 IsStrafeMode 取当前态，再传入 ApplyStrafeMode 应用反向态
	ApplyStrafeMode(!IsStrafeMode());
}

bool ABaseCharacter::IsStrafeMode() const
{
	// 以 bUseControllerRotationYaw 作为「八向模式」的唯一判定来源。
	// 不引入额外的 bool 字段，避免和移动组件的标志位脱节。
	return bUseControllerRotationYaw;
}

void ABaseCharacter::ApplyStrafeMode(bool bEnable)
{
	// 八向模式 = Yaw 跟相机；自由模式 = 身体朝速度方向。两者必须互斥。
	bUseControllerRotationYaw = bEnable;

	// 防御式判空：理论上 GetCharacterMovement() 不会为 null（ACharacter 默认创建），
	// 但若极端情况（如组件被外部销毁）下为 null 直接解引用会崩溃。
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		// !bEnable：八向开 → 关掉「朝速度方向」；八向关 → 打开「朝速度方向」
		MoveComp->bOrientRotationToMovement = !bEnable;
	}
}

// =====================================================================================
// 基础移动输入
// =====================================================================================

void ABaseCharacter::Move(const FInputActionValue& Value)
{
	// EnhancedInput 通过模板方法 Get<T>() 取实际值。
	// 约定 IA_Move 的 ValueType = Axis2D，对应 FVector2D：
	//   X = 右轴（+D / −A），Y = 前轴（+W / −S）
	const FVector2D Axis = Value.Get<FVector2D>();

	// 早退条件：
	//  1) Controller 为空：角色尚未 Possess 或已被 Unpossess，调用 GetControlRotation() 不安全
	//  2) Axis 接近零：完全没有输入，省一次矩阵构造和两次 AddMovementInput 调用
	if (!Controller || Axis.IsNearlyZero())
	{
		return;
	}

	// 关键：只取 Yaw 分量。
	// 若直接用 ControlRotation，Forward 向量会因 Pitch 含 Z 分量；
	// 玩家向上看 45° 时按 W 会推角色斜向上飞（不是想要的行为）。
	// 把 Pitch / Roll 清零后再算 Forward / Right，能保证地面移动恒在水平面内。
	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);

	// 由旋转构造 3x3 旋转矩阵；GetUnitAxis 直接拿到对应轴的单位向量
	const FRotationMatrix RotMatrix(YawRotation);

	// X 轴 = 相机水平 Forward，乘 Y 轴值（前后）
	AddMovementInput(RotMatrix.GetUnitAxis(EAxis::X), Axis.Y);
	// Y 轴 = 相机水平 Right，乘 X 轴值（左右）
	AddMovementInput(RotMatrix.GetUnitAxis(EAxis::Y), Axis.X);
	// 注意：AddMovementInput 内部会累加并归一化，所以斜向（W+D）合速度不会超过单方向上限
}

void ABaseCharacter::Look(const FInputActionValue& Value)
{
	// Controller 为空时无人接收 Yaw / Pitch 输入，直接早退
	if (!Controller)
	{
		// [DEBUG] 调试完删
		DebugHelper::Print(TEXT("[BC.Look] Controller=NULL，输入被丢弃"), 3000, FColor::Red);
		return;
	}

	// 约定 IA_Look 的 ValueType = Axis2D：
	//   X = Yaw 增量（左右）；Y = Pitch 增量（上下）
	const FVector2D Axis = Value.Get<FVector2D>();

	// [DEBUG] 调试完删：打印输入值 + 调用前后 ControlRotation 变化，确认 Yaw/Pitch 是否被累加
	const FRotator Before = Controller->GetControlRotation();

	// AddControllerYawInput / AddControllerPitchInput 会乘以 InputYaw/PitchScale_DEPRECATED
	// 以及鼠标灵敏度等系数。零输入时是空操作，无需早退。
	AddControllerYawInput(Axis.X);
	DebugHelper::Print(FString::Printf(
		TEXT("[BC.Look] Axis=%s"),
		*Axis.ToString()),
		3001, FColor::Green);
	AddControllerPitchInput(Axis.Y);

	// [DEBUG] 调试完删
	const FRotator After = Controller->GetControlRotation();
	DebugHelper::Print(FString::Printf(
		TEXT("[BC.Look] Axis=%s | CtrlRot Yaw %.2f→%.2f Pitch %.2f→%.2f"),
		*Axis.ToString(), Before.Yaw, After.Yaw, Before.Pitch, After.Pitch),
		3001, FColor::Green);
}

// =====================================================================================
// Pitch 视角夹紧（重写 APawn::AddControllerPitchInput）
// =====================================================================================
void ABaseCharacter::AddControllerPitchInput(float Val)
{
	// 早退 1：零输入直接放行，避免无意义计算
	// 早退 2：Controller 为空时，Super 内部也会早退；统一交给父类处理
	if (Val == 0.f || !Controller)
	{
		Super::AddControllerPitchInput(Val);
		return;
	}

	// ControlRotation.Pitch 原始范围是 [0, 360)，例如低头 10° 时是 10、抬头 10° 时是 350。
	// NormalizeAxis 把它映射到 [-180, 180]，方便和 MinLookPitch / MaxLookPitch 比较。
	const float CurrentPitch = FRotator::NormalizeAxis(Controller->GetControlRotation().Pitch);

	// UE 默认约定：+Val = 抬头（Pitch 数值减小）；-Val = 低头（Pitch 数值增大）
	// 若项目里 IA_Look 的 Modifier 反转了 Y 轴，把下面两行的 ">" 和 "<" 互换即可。
	const bool bWillLookUp   = (Val > 0.f);  // 抬头方向（向 MinLookPitch 移动）
	const bool bWillLookDown = (Val < 0.f);  // 低头方向（向 MaxLookPitch 移动）

	// 已抬到上限，禁止继续抬头；但反向（低头）输入仍允许，用户可以把视角拉回
	if (bWillLookUp && CurrentPitch <= MinLookPitch)
	{
		return;
	}
	// 已低到下限，禁止继续低头；反向（抬头）输入仍允许
	if (bWillLookDown && CurrentPitch >= MaxLookPitch)
	{
		return;
	}

	// 范围内：正常累加输入
	// 注意：AddControllerPitchInput 不会立即修改 ControlRotation，而是把 Val 累加到
	//      RotationInput.Pitch 缓冲，下一帧 PlayerCameraManager::ProcessViewRotation 才会应用。
	//      因此一帧之内大跨度输入可能产生 1-2° 过冲，下一次 Look 触发本函数时会被屏蔽，不持续超界。
	Super::AddControllerPitchInput(Val);
}

// =====================================================================================
// [DEBUG] BeginPlay 自检：确认蓝图里 SpringArm / Camera 配置正确
// =====================================================================================
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 绑定武器碰撞命中委托
	if (WeaponCollider)
	{
		WeaponCollider->OnWeaponHit.AddDynamic(this, &ABaseCharacter::OnAttackHit);
	}

	USpringArmComponent* SA  = FindComponentByClass<USpringArmComponent>();
	UCameraComponent*    Cam = FindComponentByClass<UCameraComponent>();

	if (!SA)
	{
		DebugHelper::Print(TEXT("[BC.BeginPlay] SpringArm 未找到！蓝图里没加，或类型不对"), 2001, FColor::Red);
	}
	else
	{
		// 关键关注：bUsePawnControlRotation 必须为 true，否则相机不会跟 ControlRotation 转
		// Inherit Pitch/Yaw/Roll 必须保持 true（默认值），否则父级 Capsule 转动也不传给 SpringArm
		DebugHelper::Print(FString::Printf(
			TEXT("[BC.BeginPlay] SpringArm  bUsePawnCtrlRot=%d  Inherit P/Y/R=%d/%d/%d  ArmLen=%.0f"),
			SA->bUsePawnControlRotation ? 1 : 0,
			SA->bInheritPitch ? 1 : 0, SA->bInheritYaw ? 1 : 0, SA->bInheritRoll ? 1 : 0,
			SA->TargetArmLength),
			2002, FColor::Cyan);
	}

	if (!Cam)
	{
		DebugHelper::Print(TEXT("[BC.BeginPlay] Camera 未找到！蓝图里没加，或类型不对"), 2003, FColor::Red);
	}
	else
	{
		// 关键关注：bUsePawnControlRotation 必须为 false（由 SpringArm 接管），否则 Camera 会和 SpringArm 抢旋转
		// AttachParent 应该是 SpringArm，否则相机不会被 SpringArm 末端 Socket 带着转
		DebugHelper::Print(FString::Printf(
			TEXT("[BC.BeginPlay] Camera     bUsePawnCtrlRot=%d  AttachParent=%s"),
			Cam->bUsePawnControlRotation ? 1 : 0,
			Cam->GetAttachParent() ? *Cam->GetAttachParent()->GetName() : TEXT("NULL")),
			2004, FColor::Cyan);
	}
}

void ABaseCharacter::Jump()
{
	Super::Jump();
	bIsJumping = true;
}

void ABaseCharacter::StopJumping()
{
	Super::StopJumping();
	bIsJumping = false;
}

void ABaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	StopJumping();
}

// =====================================================================================
// 战斗 — 攻击
// =====================================================================================

void ABaseCharacter::LightAttack()
{
	if (bIsAttacking || !LightAttackMontage)
	{
		return;
	}

	bIsAttacking = true;

	PlayAnimMontage(LightAttackMontage);

	// Montage 结束后通过 FOnMontageEnded 委托重置门控
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ABaseCharacter::OnAttackMontageEnded);
	GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate, LightAttackMontage);
}

void ABaseCharacter::HeavyAttack()
{
	if (bIsAttacking || !HeavyAttackMontage)
	{
		return;
	}

	bIsAttacking = true;

	PlayAnimMontage(HeavyAttackMontage);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ABaseCharacter::OnAttackMontageEnded);
	GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate, HeavyAttackMontage);
}

void ABaseCharacter::OnAttackHit(AActor* HitActor, const FHitResult& HitResult)
{
	// 空实现：派生类或蓝图可重写以添加具体命中逻辑（伤害、击退等）
}

void ABaseCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
}
