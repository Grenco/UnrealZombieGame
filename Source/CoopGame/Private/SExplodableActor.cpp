// Fill out your copyright notice in the Description page of Project Settings.


#include "SExplodableActor.h"
#include "Components/SHealthComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"

// Sets default values
ASExplodableActor::ASExplodableActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody); // Set to physics body to let radial force component of other instances affect it

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(RootComponent);
	RadialForceComp->Radius = 500.0f;
	RadialForceComp->bAutoActivate = false;
	RadialForceComp->bIgnoreOwningActor = true;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->ImpulseStrength = 1000.0f;
	RadialForceComp->Falloff = ERadialImpulseFalloff::RIF_Linear;

	BaseDamage = 100.0f;

	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ASExplodableActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		HealthComp->OnHealthChanged.AddDynamic(this, &ASExplodableActor::OnHealthChanged);
	}
}

void ASExplodableActor::OnHealthChanged(USHealthComponent* HealthComponent, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0 && !bHasExploded)
	{
		// Explode
		bHasExploded = true;

		// Apply upwards force to barrel
		MeshComp->AddImpulse(FVector::UpVector * 700.0f, NAME_None, true);

		// Apply radial impulse to to everything around it
		RadialForceComp->SetWorldLocation(GetActorLocation());
		RadialForceComp->FireImpulse();

		// Apply radial damage
		UGameplayStatics::ApplyRadialDamage(GetWorld(), BaseDamage, GetActorLocation(), RadialForceComp->Radius, nullptr, TArray<AActor*>(), this, GetInstigatorController());

		MulticastExplode();
	}
	MulticastShowDamageVisuals(Health);
}

void ASExplodableActor::MulticastExplode_Implementation()
{
	if (ExplosionEffect)
	{
		// Play explosion effect
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	}

	UGameplayStatics::SpawnSoundAtLocation(this, ExplosionSound, GetActorLocation());

	MeshComp->SetMaterial(0, ExplodedMaterial);
}

bool ASExplodableActor::MulticastExplode_Validate()
{
	return true;
}

void ASExplodableActor::MulticastShowDamageVisuals_Implementation(float NewHealth)
{
	ShowDamageVisuals(NewHealth);
}

void ASExplodableActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASExplodableActor, bHasExploded);
}
