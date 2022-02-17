// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickupActor.generated.h"

class USphereComponent;
class UDecalComponent;
class ASPowerupActor;

UCLASS()
class COOPGAME_API ASPickupActor : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Components")
		USphereComponent* SphereComp;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
		UDecalComponent* DecalComp;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
		TArray<TSubclassOf<ASPowerupActor>> PowerupClassList;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
		bool bRandomisePowerupSpawn;

	ASPowerupActor* PowerupInstance;

	int PowerupClassIndex;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
	float CooldownDuration;

	FTimerHandle TimerHandle_RespawnTimer;

public:
	// Sets default values for this actor's properties
	ASPickupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void Respawn();

public:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

};
