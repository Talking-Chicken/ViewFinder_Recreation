#pragma once

#include "CoreMinimal.h"
#include "ProceduralBaseActor.h" // Inherit from your base C++ class
#include "ProceduralPhysicsActor.generated.h"

UCLASS()
class VIEWFINDER_API AProceduralPhysicsActor : public AProceduralBaseActor
{
	GENERATED_BODY()

public:
	AProceduralPhysicsActor();

protected:
	virtual void BeginPlay() override;

public:
	// Override SetupMesh to enable physics after procedural geometry generation
	virtual void SetupMesh() override;
};