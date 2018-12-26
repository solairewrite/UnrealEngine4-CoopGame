// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

// 配合UPROPERTY(),创建一个蓝图事件,通过.Broadcast触发
// FOnHealthChangedSignature是代理名字
// 通过这个宏定义的变量,可以AddDynamic
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, USHealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

// 设置ClassCroup,在蓝图中分类
UCLASS(ClassGroup = (COOP), meta = (BlueprintSpawnableComponent))
class COOPGAME_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "HealthComponent")
		float Health;

	UFUNCTION()
		void OnRep_Health(float OldHealth); // 可以将原来的值作为参数

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthComponent")
		float DefaultHealth;

	// AddDynamic代理方法,参数通过不断Ctrl+LM找源头获得
	UFUNCTION()
		void HandleTakeAnyDamage(AActor*DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:

	UPROPERTY(BlueprintAssignable, Category = "Events") // 可以作为事件,在蓝图中使用
		FOnHealthChangedSignature OnHealthChanged;

	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
		void Heal(float HealAmount);
};
