// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplodableActor.generated.h"

class URadialForceComponent;
class UHealthComponent;
class UParticleSystem;
class USoundCue;

UCLASS()
class COOPGAME_API ASExplodableActor : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
		UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
		URadialForceComponent* RadialForceComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USHealthComponent* HealthComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
		UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explodable")
		float BaseDamage;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Explodable")
		bool bHasExploded;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Materials")
		UMaterialInterface* ExplodedMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
		USoundCue* ExplosionSound;

public:
	// Sets default values for this actor's properties
	ASExplodableActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
		void OnHealthChanged(USHealthComponent* HealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void MulticastExplode();

	UFUNCTION(BlueprintImplementableEvent)
		void ShowDamageVisuals(float NewHealth);

	UFUNCTION(NetMulticast, Unreliable)
		void MulticastShowDamageVisuals(float NewHealth);

public:

};
