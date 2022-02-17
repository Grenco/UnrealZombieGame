// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

class USHealthComponent;
class USphereComponent;
class USoundCue;

UCLASS()
class COOPGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		USphereComponent* SphereComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		USHealthComponent* HealthComp;

	// Next point in navigation path
	FVector NextPathPoint;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		float RequiredDistanceToTarget;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
		bool bUseVelocityChange;

	// Dynamic material to pulse on damage
	UMaterialInstanceDynamic* MatInst;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
		UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
		float ExplosionBaseDamage;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
		float ExplosionRadius;

	bool bExploded;

	FTimerHandle TimerHandle_SelfDamage;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
		float SelfDamageInterval;

	bool bStartedSelfDestruction;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
		USoundCue* SelfDestructWarningSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
		USoundCue* ExplosionSound;

	UPROPERTY(ReplicatedUsing = OnRep_PowerLevel)
		float PowerLevel;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
		float MaxPowerLevel;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
		float BasePowerLevel;

	// Distance to ally trackerbots needed to increase the power level
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
		float RequiredDistanceForPowerUp;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
		float PowerLevelUpdateInterval;

	FTimerHandle TimerHandle_CalculatePowerLevel;

	FTimerHandle TimerHandle_RefreshPath;

public:
	// Sets default values for this pawn's properties
	ASTrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FVector FindNextPathPoint();

	UFUNCTION()
		void OnHealthChanged(USHealthComponent* HealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	void SelfDestruct();

	void DamageSelf();

	void CalculatePowerLevel();

	UFUNCTION()
	void OnRep_PowerLevel();

	void RefreshPath();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

};
