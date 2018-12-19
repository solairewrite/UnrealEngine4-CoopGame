// Fill out your copyright notice in the Description page of Project Settings.

#include "NormalBox.h"


// Sets default values
ANormalBox::ANormalBox()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	// 允许被击飞
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;

	SetReplicates(true);
	SetReplicateMovement(true);

}

