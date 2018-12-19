// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "CoopGame.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

// 编辑器控制台命令,修改后可能要重启虚幻,才能编译生效
static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"), // 控制台调用命令
	DebugWeaponDrawing, // 控制台修改的变量
	TEXT("Draw Debug Lines for Weapons"),
	ECVF_Cheat
);

// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target"; // UE4中,P_SmokeTrial的Target.Distribution.ParameterName

	BaseDamage = 20.0f;

	RateOfFire = 600;

	// Actor是否同步到客户端
	SetReplicates(true);

	// Epic默认更新频率不高,会导致另一个客户端显示的开枪延迟
	// 可能有热加载问题,需要重启虚幻
	// 如果你发现某个值改变之后像没改变一样,可以查看BP,看看是不是没有变
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f; // 点击跳转,可以发现默认值是2.0
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;
}

// 追踪世界,从眼睛到十字准星
void ASWeapon::Fire()
{
	// 如果不是服务器,发送请求到服务器,执行这段代码
	if (Role < ROLE_Authority)
	{
		ServerFire();
		//return;
	}

	AActor* MyOwner = GetOwner(); // 对应Scharacter的SetOwner
	if (MyOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		// 粒子效果终点
		FVector TracerEndPoint = TraceEnd;

		// 子弹打到什么表面
		EPhysicalSurface SurfaceType = SurfaceType_Default;

		FHitResult Hit;
		// 返回bool是否碰撞
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			// 伤害处理
			AActor* HitActor = Hit.GetActor();

			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			float ActualDamage = BaseDamage;
			if (SurfaceType == SURFACE_FLESH_VULNERABLE)
			{
				ActualDamage *= 4.0f;
			}

			// 只在服务器端计算伤害
			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);

			PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

			TracerEndPoint = Hit.ImpactPoint;

		}
		//UE_LOG(LogTemp, Warning, TEXT("DebugWeaponDrawing %i"), DebugWeaponDrawing);
		if (DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
		}

		PlayFireEffects(TracerEndPoint);

		if (Role == ROLE_Authority)
		{
			// 每次复制,执行OnRep_HitScanTrace
			// UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
		}

		// 记录上一次射击的时间,冷却时间内无法射击
		LastFireTime = GetWorld()->TimeSeconds;
	}
}

void ASWeapon::OnRep_HitScanTrace()
{
	// Play cosmetic FX
	PlayFireEffects(HitScanTrace.TraceTo);

	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

// 服务器方法,实现的时候在后面加上 _Implementation
void ASWeapon::ServerFire_Implementation()
{
	Fire();
}
// 服务器方法,需要实现,一般return true就行
bool ASWeapon::ServerFire_Validate()
{
	return true; // 反外挂
}

void ASWeapon::StartFire()
{
	// 射击开始时间是 上一次射击时间+冷却时间-当前时间
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	// 定时任务,FTimerHandle,代理,回调,时间间隔,是否循环,延迟
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::PlayFireEffects(FVector TraceEnd)
{

	if (FireSound) // 音效
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	if (MuzzleEffect) // 枪口特效
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect) // 轨迹特效
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			// 让粒子效果向一个方向移动
			TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
		}
	}

	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner) // 镜头抖动
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC)
		{
			// 镜头抖动
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}

void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_FLESH_DEFAULT:
		//SelectedEffect = DefaultImpactEffect; // 注释掉后,打到身体上也显示喷血效果
		//break;
	case SURFACE_FLESH_VULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}

	if (SelectedEffect)
	{
		// 通过SocketName获取枪口位置
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		// 获取射击方向, Normalize() 单位化, Rotation() 转为旋转
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 设置同步属性,使用CONDITION,避免发起的客户端复制
	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}