// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;

UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UCameraShakeBase> FireCameraShake;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	bool bIsAutomatic;

	/* Shots per minute */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;

	float TimeBetweenShots;

	FTimerHandle TimerHandle_TimeBetweenShots;

	float LastFireTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float MaxAmmo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float LowAmmo;

	float Ammo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	int MaxReloads;

	int Reloads;
	
public:	

protected:

	virtual void BeginPlay() override;

	virtual void Fire();

	void PlayFireEffects(FVector TracerEndPoint);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateAmmoHUD(float AmmoCount, int ReloadCount);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateNotificationText(const FString& Text);

	bool AmmoCheck();

public:	
	// Sets default values for this actor's properties
	ASWeapon();

	virtual void StartFire();

	virtual void StopFire();

	virtual void Reload();

};
