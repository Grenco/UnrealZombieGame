// Fill out your copyright notice in the Description page of Project Settings.


#include "SGrenadeLauncher.h"
#include "Kismet/GameplayStatics.h"
#include "SProjectile.h"

void ASGrenadeLauncher::Fire()
{
	AActor* Player = GetOwner();

	if (Player && ProjectileClass)
	{
		FVector EyeLocation;
		FRotator EyeRotation;

		Player->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		//Spawn the grenade and set up the projectile motion
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		//FRotator MuzzleRotation = MeshComp->GetSocketRotation(MuzzleSocketName);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		APawn* ProjectileInstigator = Cast<APawn>(Player);

		if (ProjectileInstigator)
		{
			SpawnParams.Instigator = ProjectileInstigator;
		}

		GetWorld()->SpawnActor<ASProjectile>(ProjectileClass, MuzzleLocation, EyeRotation, SpawnParams);
	}
}
