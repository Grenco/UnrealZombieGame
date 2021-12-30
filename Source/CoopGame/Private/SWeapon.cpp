// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "DrawDebugHelpers.h"

// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
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

		FVector EndPoint = StartPoint + Direction.Vector() * 10000;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Player);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true; // Calculates hit triangle on mesh instead of just the mesh, more precise but more expensive

		if (GetWorld()->LineTraceSingleByChannel(Hit, StartPoint, EndPoint, ECC_Visibility, QueryParams))
		{
			// It hit something! Process damage

		}

		DrawDebugLine(GetWorld(),StartPoint, EndPoint, FColor::White, false, 1.0f, 0, 1.0f);
	}

}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

