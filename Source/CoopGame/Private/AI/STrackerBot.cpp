// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SHealthComponent.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "SCharacter.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASTrackerBot::ASTrackerBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);

	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::OnHealthChanged);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(150.0f);
	SphereComp->SetupAttachment(RootComponent);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	bUseVelocityChange = true;
	MovementForce = 1000.0f;
	RequiredDistanceToTarget = 100.0f;
	ExplosionBaseDamage = 50.0f;
	ExplosionRadius = 350.0f;
	SelfDamageInterval = 0.2f;
	RequiredDistanceForPowerUp = 300.0f;
	MaxPowerLevel = 4.0f;
	BasePowerLevel = 0.0f;
	PowerLevelUpdateInterval = 1.0f;
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		// Find initial move-to
		RefreshPath();
		PowerLevel = BasePowerLevel;
		GetWorldTimerManager().SetTimer(TimerHandle_CalculatePowerLevel, this, &ASTrackerBot::CalculatePowerLevel, PowerLevelUpdateInterval, true, 0.0f);
	}
}

FVector ASTrackerBot::FindNextPathPoint()
{
	AActor* BestTarget = nullptr;
	float NearestTargetDistance = FLT_MAX;

	// Hack to get player location, can be improved
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		if (TestPawn == nullptr || USHealthComponent::IsFriendly(TestPawn, this))
		{
			continue;
		}

		USHealthComponent* TestPawnHealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));

		if (TestPawnHealthComp && TestPawnHealthComp->GetHealth() > 0.0f)
		{
			float DistToPawn = GetDistanceTo(TestPawn);

			if (DistToPawn < NearestTargetDistance)
			{
				BestTarget = TestPawn;
				NearestTargetDistance = DistToPawn;
			}
		}

	}

	if (BestTarget)
	{
		UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);

		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);
		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ASTrackerBot::RefreshPath, 1.0f, false);

		if (NavPath && NavPath->PathPoints.Num() > 1)
		{
			// Return next point in path
			return NavPath->PathPoints[1];
		}
	}

	// Failed to find path
	return GetActorLocation();
}

void ASTrackerBot::OnHealthChanged(USHealthComponent* HealthComponent, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	// Explode on death
	if (Health <= 0)
	{
		SelfDestruct();
	}

	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}
}

void ASTrackerBot::SelfDestruct()
{
	if (bExploded)
	{
		return;
	}

	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	UGameplayStatics::SpawnSoundAtLocation(this, ExplosionSound, GetActorLocation());

	MeshComp->SetVisibility(false, true);
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (HasAuthority())
	{
		float ExplosionDamage = ExplosionBaseDamage + ExplosionBaseDamage * PowerLevel;
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, TArray<AActor*>(), this, GetInstigatorController());

		// Make the actor destroy itself after 2 seconds so the effects have time to play on clients
		SetLifeSpan(2.0f);
	}

}

void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 10.0f, GetInstigatorController(), this, nullptr);
}

void ASTrackerBot::CalculatePowerLevel()
{
	// Check for nearby trackerbots which will power up this bot
	TArray<AActor*> TrackerBotList;
	UGameplayStatics::GetAllActorsOfClass(this, ASTrackerBot::GetClass(), TrackerBotList);

	int BotsInRange = 0;

	for (AActor* Bot : TrackerBotList)
	{
		if (Bot != this)
		{
			float Dist = FVector::Dist(GetActorLocation(), Bot->GetActorLocation());
			if (Dist < RequiredDistanceForPowerUp)
			{
				BotsInRange++;
			}
		}
	}

	PowerLevel = FMath::Clamp((float)BotsInRange, BasePowerLevel, MaxPowerLevel);

	OnRep_PowerLevel();
}

void ASTrackerBot::OnRep_PowerLevel()
{
	// Update material to show the power level
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst)
	{
		float PowerLevelAlpha = PowerLevel / MaxPowerLevel;
		MatInst->SetScalarParameterValue("PowerLevelAlpha", PowerLevelAlpha);
	}

	UE_LOG(LogTemp, Log, TEXT("Power Level of %s: %s"), *GetName(), *FString::SanitizeFloat(PowerLevel));
}

void ASTrackerBot::RefreshPath()
{
	NextPathPoint = FindNextPathPoint();
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && !bExploded)
	{
		float DistanceToTarget = FVector::Dist(GetActorLocation(), NextPathPoint);

		if (DistanceToTarget <= RequiredDistanceToTarget)
		{
			RefreshPath();
		}
		else
		{
			// Keep moving towards next target
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();
			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);
		}
	}
}

void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (bStartedSelfDestruction || bExploded)
	{
		return;
	}

	ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
	if (PlayerPawn && !USHealthComponent::IsFriendly(this, OtherActor))
	{
		// Overlapped with a player, start self destruction sequence
		if (HasAuthority())
		{
			GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
		}

		UGameplayStatics::SpawnSoundAttached(SelfDestructWarningSound, RootComponent);

		bStartedSelfDestruction = true;
	}
}

void ASTrackerBot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASTrackerBot, PowerLevel);
}
