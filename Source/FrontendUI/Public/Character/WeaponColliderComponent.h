// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "WeaponColliderComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnWeaponHitSignature,
	AActor*, HitActor,
	const FHitResult&, HitResult
);

/**
 * 武器碰撞检测组件。
 *
 * 职责：
 *  - 管理武器碰撞体的启用/禁用（由 AnimNotify 触发）。
 *  - 在碰撞启用期间检测 Overlap 事件，过滤后广播命中委托。
 *  - 每次挥砍独立跟踪已命中目标，防止一帧内重复命中同一 Actor。
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class FRONTENDUI_API UWeaponColliderComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UWeaponColliderComponent();

	/** 每段攻击动画的伤害窗口开始时调用（由 AnimNotify 驱动） */
	UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
	void EnableWeaponCollision();

	/** 每段攻击动画的伤害窗口结束时调用（由 AnimNotify 驱动） */
	UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
	void DisableWeaponCollision();

	/** 目标命中事件：仅在碰撞体激活时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Combat|Weapon")
	FOnWeaponHitSignature OnWeaponHit;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnWeaponOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	bool bCollisionActive = false;

	/** 本次挥砍已命中的目标集合，Disable 时清空，防止同一刀多次命中同一目标 */
	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<AActor>> HitTargetsThisSwing;
};


/**
 * AnimNotify：启用武器碰撞。
 * 拖到攻击 Montage 的伤害窗口起始帧即可，无需配置任何属性。
 */
UCLASS(EditInlineNew, meta = (DisplayName = "Enable Weapon Collision"))
class FRONTENDUI_API UAnimNotify_EnableWeaponCollision : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};


/**
 * AnimNotify：禁用武器碰撞。
 * 拖到攻击 Montage 的伤害窗口结束帧即可，无需配置任何属性。
 */
UCLASS(EditInlineNew, meta = (DisplayName = "Disable Weapon Collision"))
class FRONTENDUI_API UAnimNotify_DisableWeaponCollision : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};

