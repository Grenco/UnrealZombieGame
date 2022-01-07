// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"

// Sets default values
ASProjectile::ASProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RootComponent = MeshComp;

	ProjectileComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileComp->InitialSpeed = 3000.f;
	ProjectileComp->MaxSpeed = 3000.f;
	ProjectileComp->bRotationFollowsVelocity = true;
	ProjectileComp->bShouldBounce = true;

	DamageRadius = 100.0f;
	BaseDamage = 100.0f;

	InitialLifeSpan = 1.0f;
}

// Called when the game starts or when spawned
void ASProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	//SetLifeSpan(1);
}

// Called every frame
void ASProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASProjectile::LifeSpanExpired()
{
	// Play the explosion particle effect
	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	}

	UGameplayStatics::ApplyRadialDamage(GetWorld(), BaseDamage, GetActorLocation(), DamageRadius, DamageType, TArray<AActor*>(), this, GetInstigatorController());

	DrawDebugSphere(GetWorld(), GetActorLocation(), DamageRadius, 20, FColor::Red, false, 2.0f, 0, 1.0f);

	Super::LifeSpanExpired();
}

