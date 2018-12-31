// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "SGameMode.h"


// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	DefaultHealth = 100;
	bIsDead = false;

	TeamNum = 255;

	SetIsReplicated(true);
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			// HandleTakeAnyDamage必须UFUNCTION()
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
		}
	}

	Health = DefaultHealth;
}

void USHealthComponent::OnRep_Health(float OldHealth)
{
	float Damage = Health - OldHealth;

	OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

void USHealthComponent::HandleTakeAnyDamage(AActor*DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f || bIsDead)
	{
		return;
	}

	// 允许自爆
	if (DamageCauser != DamagedActor && IsFriendly(DamagedActor, DamageCauser))
	{
		return;
	}

	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

	//UE_LOG(LogTemp, Warning, TEXT("掉血: %s"), *FString::SanitizeFloat(Health));

	// 触发蓝图事件OnHealthChanged
	// 通过父组件的OnTakeAnyDamage.AddDynamic方法,进入HandleTakeAnyDamage()
	// 再通过Broadcast方法,触发(增加此组件的)HealthComp->OnHealthChanged.AddDynamic绑定的方法
	// 最开始触发的源头是角色OnTakeAnyDamage
	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	bIsDead = Health <= 0.0f;

	if (bIsDead)
	{
		ASGameMode* GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->onActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}

}

// 标记为const,禁止修改任何变量,例如Health++非法
float USHealthComponent::GetHealth() const
{
	return Health;
}

void USHealthComponent::Heal(float HealAmount)
{
	if (HealAmount <= 0 || Health <= 0)
	{
		return;
	}

	Health = FMath::Clamp(Health + HealAmount, 0.0f, DefaultHealth);

	// SanitizeFloat,去掉小数点后面的0
	UE_LOG(LogTemp, Warning, TEXT("加血: %s (+%s)"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(HealAmount));

	OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
}

bool USHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{
	if (ActorA == nullptr || ActorB == nullptr)
	{
		return true;
	}

	USHealthComponent* HealthCompA = Cast<USHealthComponent>(ActorA->GetComponentByClass(USHealthComponent::StaticClass()));
	USHealthComponent* HealthCompB = Cast<USHealthComponent>(ActorB->GetComponentByClass(USHealthComponent::StaticClass()));

	if (HealthCompA == nullptr || HealthCompB == nullptr)
	{
		return true;
	}

	return HealthCompA->TeamNum == HealthCompB->TeamNum;
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 设置同步属性
	DOREPLIFETIME(USHealthComponent, Health);
}
