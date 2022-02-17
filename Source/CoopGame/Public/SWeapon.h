// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;

USTRUCT()
// Contains information of a single hitscan weapon linetrace
struct FHitScanTrace
{
	GENERATED_BODY()

public:
	UPROPERTY()
		TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
		FVector_NetQuantize TraceTo;
};

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

	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
		float DamageBoost;

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

	/* Half-angle of bullet spread (degrees) */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
		float BulletSpread;

	float TimeBetweenShots;

	FTimerHandle TimerHandle_TimeBetweenShots;

	float LastFireTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		float MaxAmmo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		float LowAmmo;

	UPROPERTY(ReplicatedUsing = OnRep_Ammo)
		float Ammo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		int MaxReloads;

	UPROPERTY(ReplicatedUsing = OnRep_Reloads)
		int Reloads;

	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
		FHitScanTrace HitScanTrace;

public:

protected:

	virtual void BeginPlay() override;

	virtual void Fire();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerFire();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerReload();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerAddReloads(int Amount);

	void PlayFireEffects(FVector TracerEndPoint);

	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	UFUNCTION(BlueprintImplementableEvent)
		void CreateWeaponUI();

	UFUNCTION(BlueprintImplementableEvent)
		void UpdateAmmoHUD(float AmmoCount, int ReloadCount);

	UFUNCTION(BlueprintImplementableEvent)
		void UpdateNotificationText(const FString& Text);

	UFUNCTION()
		void OnRep_HitScanTrace();

	UFUNCTION()
		void OnRep_Ammo();

	UFUNCTION()
		void OnRep_Reloads();

	bool AmmoCheck();

	void UseAmmo(float Amount = 1.0f);

public:
	// Sets default values for this actor's properties
	ASWeapon();

	virtual void StartFire();

	virtual void StopFire();

	virtual void Reload();

	virtual void SetOwner(AActor* NewOwner) override;

	// Add reloads to the weapon, default value fully reloads
	UFUNCTION(BlueprintCallable, Category = "Weapon")
		void AddReloads(int Amount = 0);
};
