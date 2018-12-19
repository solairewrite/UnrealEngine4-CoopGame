// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

class USHealthComponent;
class UStaticMeshComponent;
class URadialForceComponent;
class UParticleSystem;

UCLASS()
class COOPGAME_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

protected:

	UPROPERTY(VisibleAnywhere, Category = "Components")
		USHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		URadialForceComponent* RadialForceComp;

	UFUNCTION()
		void OnHealthChanged(USHealthComponent* OwingHealthComp, float Health, float HealthDalta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	// 同步属性绑定方法,当属性改变时,同步执行方法
	UPROPERTY(ReplicatedUsing = OnRep_Exploded)
		bool bExploded;

	UFUNCTION()
		void OnRep_Exploded();

	// 爆炸时,向上的推力
	UPROPERTY(EditDefaultsOnly, Category = "FX")
		float ExplosionImpulse;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
		UParticleSystem* ExplosionEffect;

	// 爆炸后的Mesh
	UPROPERTY(EditDefaultsOnly, Category = "FX")
		UMaterialInterface* ExplodedMaterial;

};
