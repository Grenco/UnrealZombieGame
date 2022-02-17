// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerupActor.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASPowerupActor::ASPowerupActor()
{
	PowerupInterval = 0.0f;
	TotalNrOfTicks = 0;

	SetReplicates(true);

	bIsPowerupActive = false;
}

void ASPowerupActor::OnTickPowerup()
{
	if (!bIsPowerupActive)
	{
		return;
	}

	if (TicksProcessed >= TotalNrOfTicks)
	{
		bIsPowerupActive = false;
		OnRep_PowerupActive();

		// Delete timer
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);

		OnExpired();
		 
		return;
	}

	OnPowerupTicked();

	TicksProcessed++;
}

void ASPowerupActor::ActivatePowerup(AActor* ActivatingActor)
{
	OnActivated(ActivatingActor);

	bIsPowerupActive = true;
	OnRep_PowerupActive();

	if (PowerupInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ASPowerupActor::OnTickPowerup, PowerupInterval, true, 0.0f);
	}
	else
	{
		OnTickPowerup();
	}
}

void ASPowerupActor::OnRep_PowerupActive()
{
	OnPowerupStateChanged(bIsPowerupActive);
}

void ASPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerupActor, bIsPowerupActive);
}
