// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SRayGun.generated.h"

/**
 * 
 */

class UParticleSystemComponent;

UCLASS()
class COOPGAME_API ASRayGun : public ASWeapon
{
	GENERATED_BODY()

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float DPS;

	float Damage;

	float FireTime;

	bool bIsFiring;

	UParticleSystemComponent* TracerComp;

public:

	ASRayGun();

	virtual void StartFire() override;

	virtual void StopFire() override;

protected:
	virtual void Fire() override;

	virtual void Tick(float DeltaSeconds) override;

};
