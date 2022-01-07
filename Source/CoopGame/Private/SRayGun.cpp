// Fill out your copyright notice in the Description page of Project Settings.


#include "SRayGun.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame/CoopGame.h"

ASRayGun::ASRayGun() 
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bIsFiring = false;

	DPS = 50.0f;

	MaxAmmo = 30.0f;
	LowAmmo = 5.0f;
	MaxReloads = 0;
}

void ASRayGun::Fire()
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
		//QueryParams.bReturnPhysicalMaterial = true;

		FVector TracerEndPoint = EndPoint; // Particle target parameter

		if (GetWorld()->LineTraceSingleByChannel(Hit, StartPoint, EndPoint, COLLISION_WEAPON, QueryParams))
		{
			// It hit something! Process damage
			AActor* HitActor = Hit.GetActor();

			TracerEndPoint = Hit.ImpactPoint;

			EPhysicalSurface HitSurface = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			UGameplayStatics::ApplyPointDamage(HitActor, Damage, ShotDirection, Hit, Player->GetInstigatorController(), this, DamageType);
		}

		if (TracerComp)
		{
			FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
			TracerComp->SetWorldLocationAndRotation(MuzzleLocation, Direction);
			TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
		else if (TracerEffect)
		{
			FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
			TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		}
	}
}

void ASRayGun::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsFiring && Ammo > 0.0f)
	{
		Damage = DPS * DeltaSeconds;
		Fire();

		Ammo -= DeltaSeconds;
		// If ammo has ran out while firing, stop firing
		if (!AmmoCheck())
		{
			StopFire();
		}
	}
}

void ASRayGun::StartFire()
{
	bIsFiring = true;
}

void ASRayGun::StopFire()
{
	bIsFiring = false;
	
	if (TracerComp)
	{
		TracerComp->DestroyComponent();
	}
}
