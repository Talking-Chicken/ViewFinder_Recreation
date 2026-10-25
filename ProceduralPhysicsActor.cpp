#include "ProceduralPhysicsActor.h"
#include "ProceduralMeshComponent.h"

AProceduralPhysicsActor::AProceduralPhysicsActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AProceduralPhysicsActor::BeginPlay()
{
	Super::BeginPlay();
}

void AProceduralPhysicsActor::SetupMesh()
{
	// 1. Call base class logic first to generate procedural geometry
	Super::SetupMesh();

	// 2. Enable physics on generated procedural components, but only in gameplay mode
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		if (PrimaryProceduralMesh)
		{
			PrimaryProceduralMesh->SetSimulatePhysics(true);
			PrimaryProceduralMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
		}

		for (UProceduralMeshComponent* Comp : ProceduralMeshComps)
		{
			if (Comp)
			{
				Comp->SetSimulatePhysics(true);
				Comp->SetCollisionProfileName(TEXT("PhysicsActor"));
			}
		}
	}
}