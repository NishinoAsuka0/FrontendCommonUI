// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/WeaponColliderComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

// =====================================================================================
// UWeaponColliderComponent
// =====================================================================================

UWeaponColliderComponent::UWeaponColliderComponent()
{
	// 默认关闭碰撞：只有在 AnimNotify → EnableWeaponCollision 期间才生效
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);

	// 只对 Pawn 和 PhysicsBody 做 Overlap 检测
	SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody, ECollisionResponse::ECR_Overlap);
}

void UWeaponColliderComponent::BeginPlay()
{
	Super::BeginPlay();

	// 确保运行时初始状态为关闭碰撞（构造只在 CDO/Editor 中跑，Spawned 时可能被蓝图覆盖）
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	bCollisionActive = false;

	OnComponentBeginOverlap.AddDynamic(this, &UWeaponColliderComponent::OnWeaponOverlapBegin);
}

void UWeaponColliderComponent::EnableWeaponCollision()
{
	bCollisionActive = true;
	HitTargetsThisSwing.Reset();
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void UWeaponColliderComponent::DisableWeaponCollision()
{
	bCollisionActive = false;
	HitTargetsThisSwing.Reset();
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UWeaponColliderComponent::OnWeaponOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// 1. 碰撞未激活 → 丢弃
	if (!bCollisionActive)
	{
		return;
	}

	// 2. 无效目标 / 自伤过滤
	if (!OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	// 3. 本次挥砍已命中 → 丢弃（防重复）
	if (HitTargetsThisSwing.Contains(OtherActor))
	{
		return;
	}

	HitTargetsThisSwing.Add(OtherActor);
	OnWeaponHit.Broadcast(OtherActor, SweepResult);
}


// =====================================================================================
// AnimNotify — 启用武器碰撞
// =====================================================================================

void UAnimNotify_EnableWeaponCollision::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	UWeaponColliderComponent* WeaponCollider =
		Owner->FindComponentByClass<UWeaponColliderComponent>();
	if (WeaponCollider)
	{
		WeaponCollider->EnableWeaponCollision();
	}
}


// =====================================================================================
// AnimNotify — 禁用武器碰撞
// =====================================================================================

void UAnimNotify_DisableWeaponCollision::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	UWeaponColliderComponent* WeaponCollider =
		Owner->FindComponentByClass<UWeaponColliderComponent>();
	if (WeaponCollider)
	{
		WeaponCollider->DisableWeaponCollision();
	}
}
