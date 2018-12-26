// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickupActor.generated.h"

class USphereComponent;
class UDecalComponent;
class ASPowerupActor;

UCLASS()
class COOPGAME_API ASPickupActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASPickupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		UDecalComponent* DecalComp; // 贴花,地面投影

	// 用于创建PowerUpInstance,每一个实例都需要设置
	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
		TSubclassOf<ASPowerupActor> PowerUpClass;

	ASPowerupActor* PowerUpInstance;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
		float CooldownDuration; // 重新生成时间

	FTimerHandle TimerHandle_RespawnTimer;

	void Respawn();


public:

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

};
