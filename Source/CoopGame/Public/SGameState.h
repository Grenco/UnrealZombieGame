// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SGameState.generated.h"

UENUM(BlueprintType)
enum class EWaveState : uint8
{
	// Default state
	WaitingToStart,
	// Getting ready to start the next wave
	PreparingNextWave,
	// Spawning new enemies
	WaveInProgress,
	// No longer spawning new enemies, waiting for players to kill remaining enemies
	WaitingToComplete,
	// All enemies in current wave have been killed
	WaveComplete,
	// All players have died
	GameOver
};

/**
 * 
 */
UCLASS()
class COOPGAME_API ASGameState : public AGameStateBase
{
	GENERATED_BODY()

protected:

	UPROPERTY(ReplicatedUsing = OnRep_WaveState, BlueprintReadOnly, Category = "GameState")
		EWaveState WaveState;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameState")
		int32 WaveCount;

protected:
	
	UFUNCTION()
	void OnRep_WaveState(EWaveState OldState);

	UFUNCTION(BlueprintImplementableEvent, Category = "GameState")
	void WaveStateChanged(EWaveState NewState, EWaveState OldState);

public:

	void SetWaveState(EWaveState NewState);

	void SetWaveCount(int32 NewWaveCount);

};
