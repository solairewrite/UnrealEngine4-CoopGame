// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework//PawnMovementComponent.h"
#include "SWeapon.h"
#include "Components/CapsuleComponent.h"
#include "CoopGame.h"
#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASCharacter::ASCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	// 禁止子弹打到CapsuleComponent,只允许打到Mesh
	// 可以观察PlayerPawn -> CapsuleComponent -> Collision Presets -> Weapon: Block
	// 热加载有时会失效,需要重启虚幻
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	// 通过代码加载组件HealthComp,代理在PlayerPawn中,点击AddComp加载组件
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	//CameraComp->bUsePawnControlRotation = true;
	CameraComp->SetupAttachment(SpringArmComp);

	ZoomedFOV = 65.0f;
	ZoomInterpSpeed = 20;

	WeaponAttachSocketName = "WeaponSocket";
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultFOV = CameraComp->FieldOfView;
	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);

	// 生成默认武器
	CurrentWeaponIndex = -1;
	SwitchWeapons(0);

}

void ASCharacter::MoveForward(float Value)
{
	// 控制玩家向前移动
	AddMovementInput(GetActorForwardVector()*Value);
}

void ASCharacter::MoveRight(float Value)
{
	// 控制玩家向右移动
	AddMovementInput(GetActorRightVector()*Value);
}

void ASCharacter::BeginCrouch()
{
	Crouch();
}

void ASCharacter::EndCrouch()
{
	UnCrouch();
}

void ASCharacter::BeginZoom()
{
	bWantsToZoom = true;
}

void ASCharacter::EndZoom()
{
	bWantsToZoom = false;
}

void ASCharacter::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void ASCharacter::StopFire()
{
	//UE_LOG(LogTemp, Warning, TEXT("Stop Fire CurrentWeaponIndex %i"), CurrentWeaponIndex)
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void ASCharacter::OnHealthChanged(USHealthComponent* HealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !bDied) // 死亡
	{
		bDied = true; // 死亡动画,在蓝图中通过bDied设置
		// 立刻停止移动
		GetMovementComponent()->StopMovementImmediately();
		// 禁用胶囊体组件
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// 使Pawn与控制器分离,等待销毁
		DetachFromControllerPendingDestroy();
		// 10s后,销毁Pawn
		SetLifeSpan(10.0f);
	}
}

void ASCharacter::SwitchToFirstWeapon()
{
	SwitchWeapons(0);
}

void ASCharacter::SwitchToSecondWeapon()
{
	SwitchWeapons(1);
}

void ASCharacter::SwitchWeapons(int WeaponIndex)
{
	if (CurrentWeaponIndex == WeaponIndex)
	{
		UE_LOG(LogTemp, Warning, TEXT("当前武器==武器[%i],不需要切换"), WeaponIndex);
		return;
	}

	// 服务器运行
	if (Role == ROLE_Authority)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (CurrentWeapon)
		{
			CurrentWeapon->Destroy();
		}

		TSubclassOf<ASWeapon> NewWeaponClass;
		switch (WeaponIndex)
		{
		case 0:
			NewWeaponClass = StarterWeaponClass;
			break;
		case 1:
			NewWeaponClass = SecondWeaponClass;
			break;
		default:
			break;
		}
		CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(NewWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (CurrentWeapon) // 设置武器拥有者,添加武器
		{
			CurrentWeaponIndex = WeaponIndex;
			CurrentWeapon->SetOwner(this); // 对应SWaepon::Fire()的GetOwner
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
			UE_LOG(LogTemp, Warning, TEXT("设置武器[%i]"), WeaponIndex);
		}
	}
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;

	// 差值计算
	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);

	CameraComp->SetFieldOfView(NewFOV);
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// 绑定输入控制
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);
	// 鼠标移动视角
	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);
	// 蹲伏
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);
	// 跳跃
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	// 右键缩放
	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);
	// 射击
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);
	// 切换武器
	PlayerInputComponent->BindAction("FirstWeapon", IE_Pressed, this, &ASCharacter::SwitchToFirstWeapon);
	PlayerInputComponent->BindAction("SecondWeapon", IE_Pressed, this, &ASCharacter::SwitchToSecondWeapon);
}

FVector ASCharacter::GetPawnViewLocation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 设置同步属性
	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bDied);
}
