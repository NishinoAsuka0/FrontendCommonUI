// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

// 前向声明，避免在头文件包含 EnhancedInput 模块（减少编译耦合，速度更快）
struct FInputActionValue;
struct FHitResult;
class UWeaponColliderComponent;
class UAnimMontage;

/**
 * 项目角色基类。
 *
 * 职责：
 *  - 默认采用「自由移动模式」（身体朝速度方向旋转，不可侧步/后退面朝相机）。
 *  - 提供 ToggleStrafeMode() 在「自由模式」与「八向模式」之间切换。
 *      · 自由模式：bOrientRotationToMovement = true、bUseControllerRotationYaw = false
 *      · 八向模式：bOrientRotationToMovement = false、bUseControllerRotationYaw = true
 *  - 提供基础移动接口（Move / Look）作为虚函数，便于派生类按需重写。
 *  - 跳跃直接复用 ACharacter::Jump / StopJumping（父类已经是 virtual），不重复封装。
 *
 * 输入接入约定：F / WASD / 鼠标 / 空格 等键位绑定放在 BP_FrontendController 的
 * EnhancedInput 中，由蓝图把 InputAction 事件路由到本类的 Move / Look /
 * ToggleStrafeMode / Jump / StopJumping。
 */
UCLASS()
class FRONTENDUI_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	/**
	 * 构造函数：
	 *  - 配置 Mesh 偏移和旋转，使骨骼网格脚底贴地、朝向 Capsule 正前。
	 *  - 默认走「自由模式」，方便没接入切换逻辑前也能体验普通第三人称移动。
	 *  - 配置 CharacterMovement 的常用参数（跳跃力度、空中控制、最大速度等）。
	 */
	ABaseCharacter();

protected:
	// [DEBUG] 一次性自检 SpringArm / Camera 配置，定位"鼠标移动相机不跟随"问题，调试完可删
	virtual void BeginPlay() override;
	
	virtual void Jump() override;
	
	virtual void StopJumping() override;
	virtual void Landed(const FHitResult& Hit) override;

public:

	/**
	 * 切换 Strafe（八向）模式。
	 *
	 * 调用一次：从当前模式翻转到另一种。可在蓝图（如按 F 时）调用。
	 * 内部最终落到 ApplyStrafeMode(bool)。
	 *
	 * 设计上保证幂等：连续奇数次调用进入相反模式，偶数次回到原模式，
	 * 不维护额外状态字段，避免双源同步问题。
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ToggleStrafeMode();

	/**
	 * 查询当前是否处于八向模式。
	 *
	 * 直接以 bUseControllerRotationYaw 作为「真相源」，避免和单独维护的
	 * bool 标志位脱节。bUseControllerRotationYaw == true 即八向模式。
	 *
	 * @return true = 八向模式（身体跟随相机 Yaw）；false = 自由模式（身体跟随速度方向）。
	 */
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsStrafeMode() const;
	
	UFUNCTION(BlueprintCallable, Category = "Movement")
	FORCEINLINE bool IsJumping() const { return bIsJumping; }

	/**
	 * 平面移动。
	 *
	 * 输入约定（与 EnhancedInput IA_Move 的 Vector2D 对应）：
	 *   Value.X = 右轴（+D / −A）
	 *   Value.Y = 前轴（+W / −S）
	 *
	 * 参考系：取 Controller 的 ControlRotation 仅 Yaw 分量，构造水平面 Forward / Right
	 * 向量后调用 AddMovementInput()。这样无论玩家向上 / 下看，前进方向都保持水平，
	 * 不会推动角色飞起或钻地。
	 *
	 * 设为 virtual：派生类（如带特殊移动机制的子角色）可覆盖，必要时调 Super 叠加默认行为。
	 *
	 * @param Value EnhancedInput 触发时的轴值，期望类型为 FVector2D。
	 */
	virtual void Move(const FInputActionValue& Value);

	/**
	 * 视角输入（鼠标 / 摇杆）。
	 *
	 * 输入约定（与 EnhancedInput IA_Look 的 Vector2D 对应）：
	 *   Value.X = Yaw 增量（左右）
	 *   Value.Y = Pitch 增量（上下）
	 *
	 * 调用 AddControllerYawInput / AddControllerPitchInput 修改 ControlRotation。
	 * 灵敏度、Y 轴反转等由 IA Modifier 调整，C++ 不硬编码。
	 *
	 * 设为 virtual：与 Move 同理，便于派生类自定义。
	 *
	 * @param Value EnhancedInput 触发时的轴值，期望类型为 FVector2D。
	 */
	virtual void Look(const FInputActionValue& Value);

	// =========================================================================
	// 战斗 — 攻击
	// =========================================================================

	/** 轻攻击：播放轻攻击蒙太奇，AnimNotify 会在伤害窗口内开关武器碰撞 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void LightAttack();

	/** 重攻击：播放重攻击蒙太奇，AnimNotify 会在伤害窗口内开关武器碰撞 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void HeavyAttack();

	/** 武器碰撞组件引用（蓝图需在 Components 面板中挂载 UWeaponColliderComponent） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UWeaponColliderComponent> WeaponCollider;
	
	

protected:
	/** 攻击命中回调（由 WeaponCollider->OnWeaponHit 委托触发），派生类/蓝图可重写 */
	UFUNCTION()
	virtual void OnAttackHit(AActor* HitActor, const FHitResult& HitResult);

private:
	/** 轻攻击蒙太奇（蓝图 Details 面板中赋值） */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> LightAttackMontage;

	/** 重攻击蒙太奇（蓝图 Details 面板中赋值） */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> HeavyAttackMontage;

	/** 是否正在攻击动作中（门控，防止连点和重入） */
	bool bIsAttacking = false;

	/** Montage 播放结束后重置 bIsAttacking */
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/**
	 * 重写 APawn::AddControllerPitchInput 以实施 Pitch 视角夹紧。
	 *
	 * UE 默认输入方向约定：+Val = 抬头（ControlRotation.Pitch 减小）。
	 * 实现策略：调用 Super 前判断「当前 Pitch 是否已到边界 + 本次输入方向是否会越界」，
	 *         越界则直接丢弃；反向输入（把视角拉回范围）始终放行。
	 *
	 * 注意：此函数被 Look() 间接调用，也可能被动画/AI 等其他系统调用，
	 *      所有调用方的 Pitch 输入都会被统一夹紧。
	 *
	 * @param Val 输入增量（已经过 EnhancedInput Modifier 处理）。
	 */
	virtual void AddControllerPitchInput(float Val) override;

	/**
	 * Pitch 抬头上限（normalized 到 -180~180 范围内的下界）。
	 *
	 * 例如 -60 表示玩家最多向上看 60°（对应原始 ControlRotation.Pitch = 300）。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Camera|LookLimit", meta = (ClampMin = "-89.0", ClampMax = "89.0"))
	float MinLookPitch = -60.f;

	/**
	 * Pitch 低头下限（normalized 到 -180~180 范围内的上界）。
	 *
	 * 例如 20 表示玩家最多向下看 20°（对应原始 ControlRotation.Pitch = 20）。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Camera|LookLimit", meta = (ClampMin = "-89.0", ClampMax = "89.0"))
	float MaxLookPitch = 20.f;

private:
	/**
	 * 内部统一入口：写入两个标志位。
	 *
	 * bUseControllerRotationYaw 与 bOrientRotationToMovement 必须互斥：
	 * 若同时为 true，UE 行为不可预期（角色一帧朝相机一帧朝速度）。
	 * 集中在此处理，避免别处误触发。
	 *
	 * @param bEnable true = 启用八向模式；false = 启用自由模式。
	 */
	void ApplyStrafeMode(bool bEnable);
	
	bool bIsJumping = false;
};
