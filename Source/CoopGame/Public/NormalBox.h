// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NormalBox.generated.h"

class UStaticMeshComponent;

UCLASS()
class COOPGAME_API ANormalBox : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANormalBox();

protected:

	UPROPERTY(VisibleAnywhere, Category = "Components")
		UStaticMeshComponent* MeshComp;

};
