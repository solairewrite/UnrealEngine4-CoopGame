// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SProjectileWeapon.generated.h"

/**
 *
 */
UCLASS()
class COOPGAME_API ASProjectileWeapon : public ASWeapon
{
	GENERATED_BODY()

protected:

	// 单纯的射击功能
	virtual void Fire() override;

	// 射击计算冷却时间
	virtual void StartFire() override;

	UPROPERTY(EditDefaultsOnly, Category = "ProjectileWeapon")
		TSubclassOf<AActor> ProjectileClass;

};
