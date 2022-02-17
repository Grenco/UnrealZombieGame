// Fill out your copyright notice in the Description page of Project Settings.


#include "SPickupActor.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "SPowerUpActor.h"
#include "SCharacter.h"

// Sets default values
ASPickupActor::ASPickupActor()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(75.0f);
	SetRootComponent(SphereComp);

	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetupAttachment(RootComponent);
	DecalComp->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	DecalComp->DecalSize = FVector(64.0f, 75.0f, 75.0f);

	SetReplicates(true);

	CooldownDuration = 10.0f;
}

// Called when the game starts or when spawned
void ASPickupActor::BeginPlay()
{
	Super::BeginPlay();

	PowerupClassIndex = -1;

	if (HasAuthority())
	{
		Respawn();
	}
}

void ASPickupActor::Respawn()
{
	if (PowerupClassList.Num() < 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("PowerupClass is empty in %s. Please update your Blueprint"), *GetName());
		return;
	}

	if (bRandomisePowerupSpawn)
	{
		PowerupClassIndex = FMath::RandRange(0, PowerupClassList.Num() - 1);
	}
	else
	{
		PowerupClassIndex = (PowerupClassIndex + 1) % PowerupClassList.Num();
	}

	TSubclassOf<ASPowerupActor> PowerupClass = PowerupClassList[PowerupClassIndex];

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PowerupInstance = GetWorld()->SpawnActor<ASPowerupActor>(PowerupClass, GetTransform(), SpawnParams);
}

void ASPickupActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	// Check if the overlapping actor is a player character
	if (!Cast<ASCharacter>(OtherActor))
	{
		return;
	}

	// Grant a powerup to the player if available
	if (HasAuthority() && PowerupInstance)
	{
		PowerupInstance->ActivatePowerup(OtherActor);
		PowerupInstance = nullptr;

		// Set timer to respawn
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &ASPickupActor::Respawn, CooldownDuration);
	}
}

