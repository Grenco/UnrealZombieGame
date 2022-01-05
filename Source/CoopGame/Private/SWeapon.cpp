// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame/CoopGame.h"
#include "TimerManager.h"

/* Used to toggle whether debug shapes/lines should be shown in the game for weapons */
static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"), 
	DebugWeaponDrawing, 
	TEXT("Draw Debug Lines for Weapons"), 
	ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "BeamEnd";

	BaseDamage = 20.0f;

	RateOfFire = 600.0f;

	bIsAutomatic = true;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	TimeBetweenShots = 60 / RateOfFire;
}

void ASWeapon::Fire()
{
	// Trace the world, from pawn eyes to crosshair location
	FHitResult Hit;

	AActor* Player = GetOwner();

	if (Player)
	{
		FVector StartPoint;
		FRotator Direction;
		Player->GetActorEyesViewPoint(StartPoint, Direction); // Gets the camera position and rotation

		FVector ShotDirection = Direction.Vector();

		FVector EndPoint = StartPoint + (ShotDirection * 10000);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Player);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true; // Calculates hit triangle on mesh instead of just the mesh, more precise but more expensive
		QueryParams.bReturnPhysicalMaterial = true;

		FVector TracerEndPoint = EndPoint; // Particle target parameter

		if (GetWorld()->LineTraceSingleByChannel(Hit, StartPoint, EndPoint, COLLISION_WEAPON, QueryParams))
		{
			// It hit something! Process damage
			AActor* HitActor = Hit.GetActor();

			TracerEndPoint = Hit.ImpactPoint;

			EPhysicalSurface HitSurface = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			// Set the damage based on what was hit
			float ActualDamage = BaseDamage;
			if (HitSurface == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 5.0f;
			}
			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, Player->GetInstigatorController(), this, DamageType);

			// Play a hit effect based on what was hit
			UParticleSystem* SelectedEffect = nullptr;
			switch (HitSurface)
			{
			case SURFACE_FLESHDEFAULT:
			case SURFACE_FLESHVULNERABLE:
				SelectedEffect = FleshImpactEffect;
				break;
			default:
				SelectedEffect = DefaultImpactEffect;
				break;
			}

			if (SelectedEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, TracerEndPoint, Hit.ImpactNormal.Rotation());
			}

		}

		if (DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), StartPoint, TracerEndPoint, FColor::White, false, 1.0f, 0, 1.0f);
		}

		PlayFireEffects(TracerEndPoint);

		LastFireTime = GetWorld()->TimeSeconds;
	}

}

void ASWeapon::StartFire()
{
	float TimeSinceLastShot = GetWorld()->TimeSeconds - LastFireTime;
	float FireDelay = FMath::Max(TimeBetweenShots - TimeSinceLastShot, 0.0f);

	// This sets up a timer to call the fire function at a given interval
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, bIsAutomatic, FireDelay);
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::PlayFireEffects(FVector TracerEndPoint)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}

	APawn* MyOwner = GetOwner<APawn>();
	if (MyOwner)
	{
		APlayerController* PC = MyOwner->GetController<APlayerController>();
		if (PC)
		{
			PC->ClientStartCameraShake(FireCameraShake);
		}
	}
}

