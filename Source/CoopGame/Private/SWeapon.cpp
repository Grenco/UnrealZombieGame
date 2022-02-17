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
#include "Net/UnrealNetwork.h"

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
	DamageBoost = 0.0f;

	RateOfFire = 600.0f;
	bIsAutomatic = true;
	BulletSpread = 2.0f;

	MaxAmmo = 40.0f;
	LowAmmo = 5.0f;
	MaxReloads = 5;

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void ASWeapon::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);

	CreateWeaponUI();
	AmmoCheck();
}

void ASWeapon::AddReloads(int Amount)
{
	if (!HasAuthority())
	{
		ServerAddReloads(Amount);
		return;
	}

	if (Amount == 0)
	{
		Reloads = MaxReloads;
		OnRep_Reloads();
		return;
	}

	Reloads = FMath::Min(Reloads + Amount, MaxReloads);
	OnRep_Reloads();
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	TimeBetweenShots = 60 / RateOfFire;

	Ammo = MaxAmmo;
	Reloads = MaxReloads;
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
}

void ASWeapon::ServerReload_Implementation()
{
	Reload();
}

bool ASWeapon::ServerReload_Validate()
{
	return true;
}

void ASWeapon::Fire()
{
	if (!HasAuthority())
	{
		ServerFire();
		//return;
	}

	// Trace the world, from pawn eyes to crosshair location
	FHitResult Hit;

	AActor* Player = GetOwner();

	if (Player && Ammo > 0.0f)
	{
		FVector StartPoint;
		FRotator Direction;
		Player->GetActorEyesViewPoint(StartPoint, Direction); // Gets the camera position and rotation

		FVector ShotDirection = Direction.Vector();

		// Add bullet spread
		float HalfRad = FMath::DegreesToRadians(BulletSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

		FVector EndPoint = StartPoint + (ShotDirection * 10000);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Player);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true; // Calculates hit triangle on mesh instead of just the mesh, more precise but more expensive
		QueryParams.bReturnPhysicalMaterial = true;

		FVector TracerEndPoint = EndPoint; // Particle target parameter
		EPhysicalSurface HitSurface = SurfaceType_Default;

		if (GetWorld()->LineTraceSingleByChannel(Hit, StartPoint, EndPoint, COLLISION_WEAPON, QueryParams))
		{
			// It hit something! Process damage
			AActor* HitActor = Hit.GetActor();

			TracerEndPoint = Hit.ImpactPoint;

			HitSurface = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			// Set the damage based on what was hit
			float ActualDamage = BaseDamage;
			if (HitSurface == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 5.0f;
			}

			ActualDamage += BaseDamage * DamageBoost;

			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, Player->GetInstigatorController(), Player, DamageType);

			PlayImpactEffects(HitSurface, TracerEndPoint);
		}

		if (DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), StartPoint, TracerEndPoint, FColor::White, false, 1.0f, 0, 1.0f);
		}

		PlayFireEffects(TracerEndPoint);

		if (HasAuthority())
		{
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = HitSurface;
		}

		LastFireTime = GetWorld()->TimeSeconds;

		UseAmmo();
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

bool ASWeapon::AmmoCheck()
{
	bool bHasAmmo = Ammo > 0.0f;
	bool bCanReload = Reloads > 0;
	bool bLowAmmo = Ammo <= LowAmmo;

	if (!bHasAmmo && bCanReload)
	{
		Reload();
		UpdateNotificationText("");
	}
	else if (!bHasAmmo)
	{
		UpdateNotificationText("No Ammo");
	}
	else if (bLowAmmo && bCanReload)
	{
		UpdateNotificationText("Press R to Reload");
	}
	else if (bLowAmmo)
	{
		UpdateNotificationText("Low Ammo");
	}
	else
	{
		UpdateNotificationText("");
	}
	UpdateAmmoHUD(Ammo, Reloads);

	return bHasAmmo;
}

void ASWeapon::UseAmmo(float Amount)
{
	if (HasAuthority())
	{
		Ammo -= Amount;
		AmmoCheck(); // This shouldn't really be called here, but it doesn't call from the OnRep function for the listen server when ammo gets updated, just clients
	}
}

void ASWeapon::Reload()
{
	if (!HasAuthority())
	{
		ServerReload();
		return;
	}

	if (Reloads > 0)
	{
		Ammo = MaxAmmo;
		Reloads--;
		AmmoCheck(); // This shouldn't really be called here, but it doesn't call from the OnRep function for the listen server when ammo gets updated, just clients
	}
}

void ASWeapon::ServerAddReloads_Implementation(int Amount)
{
	AddReloads(Amount);
}

bool ASWeapon::ServerAddReloads_Validate(int Amount)
{
	return true;
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

void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	// Play a hit effect based on what was hit
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
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
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

void ASWeapon::OnRep_HitScanTrace()
{
	// Play cosmetic FX
	PlayFireEffects(HitScanTrace.TraceTo);

	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

void ASWeapon::OnRep_Ammo()
{
	AmmoCheck();
}

void ASWeapon::OnRep_Reloads()
{
	AmmoCheck();
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, Ammo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ASWeapon, Reloads, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}
