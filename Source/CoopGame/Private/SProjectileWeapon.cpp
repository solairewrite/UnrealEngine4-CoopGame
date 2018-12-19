// Fill out your copyright notice in the Description page of Project Settings.

#include "SProjectileWeapon.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void ASProjectileWeapon::Fire()
{
	// todo 伤害
	AActor* MyOwner = GetOwner();
	if (MyOwner && ProjectileClass)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, SpawnParams);

		if (FireSound) // 音效
		{
			UGameplayStatics::PlaySound2D(this, FireSound);
		}

		LastFireTime = GetWorld()->TimeSeconds;
	}
}

void ASProjectileWeapon::StartFire()
{
	// 连续射击可能会累加,因为时间一样所以看不出?
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASProjectileWeapon::Fire, TimeBetweenShots, false, FirstDelay);
}
