// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SGameState.generated.h"


// 声明枚举
UENUM(BlueprintType)
enum class EWaveState :uint8
{
	WaitingToStart,

	WaveInProgress,

	// 不再生成Bot,等待玩家杀死机器人
	WaitingToComplete,

	WaveComplete,

	GameOver,
};

/**
 * GameMode只在服务器端存在,创建GameStateBase,同步游戏状态到客户端
 */
UCLASS()
class COOPGAME_API ASGameState : public AGameStateBase
{
	GENERATED_BODY()

protected:

	UFUNCTION()
		void OnRep_WaveState(EWaveState OldState);

	UFUNCTION(BlueprintImplementableEvent, Category = "GameState")
		void WaveStateChanged(EWaveState NewState, EWaveState OldState);

	// ReplicatedUsing,能给绑定的函数传入参数,改变之前的值
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WaveState, Category = "GameState")
		EWaveState WaveState;

public:

	void SetWaveState(EWaveState NewState);


};
