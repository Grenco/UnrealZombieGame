// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASPlayerState : public APlayerState
{
	GENERATED_BODY()

protected:
	
	UFUNCTION(BlueprintCallable, Category = "PlayerState")
		void AddScore(float ScoreDelta = 1.0f);

};
