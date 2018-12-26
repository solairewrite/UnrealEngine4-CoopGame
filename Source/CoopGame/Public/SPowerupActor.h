// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPowerupActor.generated.h"

UCLASS()
class COOPGAME_API ASPowerupActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASPowerupActor();

protected:

	// 时间单位(用Tick作为时间单位太消耗性能)
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
		float PowerupInterval;

	// 力量提升持续时间(Tick几次)
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
		int32 TotalNrOfTicks;

	FTimerHandle TimerHandle_PowerupTick;

	// 已经Tick的次数
	int32 TicksProcessed;

	UFUNCTION()
		void OnTickPowerup();

	// 这个bool专门用来绑定同步函数
	UPROPERTY(ReplicatedUsing = OnRep_PowerupActive)
		bool bIsPowerupActive;

	UFUNCTION()
		void OnRep_PowerupActive();

	// 这里,想用蓝图来实现上面的功能
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
		void OnPowerupStateChanged(bool bNewIsActive);

public:

	void ActivatePowerup(AActor* ActiveFor);

	// 可以在蓝图中实现
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
		void OnActivated(AActor* ActiveFor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
		void OnPowerupTicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
		void OnExpired(); // 到期,失效

};
