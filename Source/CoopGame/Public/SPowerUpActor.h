// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPowerupActor.generated.h"

UCLASS()
class COOPGAME_API ASPowerupActor : public AActor
{
	GENERATED_BODY()

protected:

	/* Time between powerup ticks */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
		float PowerupInterval;

	/* Total times we apply the powerup effect */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
		int32 TotalNrOfTicks;

	/* Total number of ticks applied */
	int32 TicksProcessed;

	FTimerHandle TimerHandle_PowerupTick;

	UPROPERTY(ReplicatedUsing = OnRep_PowerupActive)
		bool bIsPowerupActive;

public:
	// Sets default values for this actor's properties
	ASPowerupActor();

protected:

	UFUNCTION()
		void OnTickPowerup();

public:

	void ActivatePowerup(AActor* ActivatingActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
		void OnActivated(AActor* ActivatingActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
		void OnPowerupTicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
		void OnExpired();

	// Keep state of the powerup
	UFUNCTION()
		void OnRep_PowerupActive();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
		void OnPowerupStateChanged(bool bNewIsActive);
};
