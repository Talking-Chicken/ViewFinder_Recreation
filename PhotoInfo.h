#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
// The .generated.h header MUST be the last include in the file!
#include "PhotoInfo.generated.h"

USTRUCT(BlueprintType)
struct FPhotoInfo
{
	GENERATED_BODY()

	/** Static mesh reference for the photo object */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photo Info")
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;

	/** Materials assigned to the captured mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photo Info")
	TArray<TObjectPtr<UMaterialInterface>> Materials;

	/** Toggles physics interaction on placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photo Info")
	bool bPhysics = false;

	/** Relative or world transform offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photo Info")
	FTransform Transform;

	/** Map of procedural plane cuts: Key = Normal Vector, Value = Position Vector */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photo Info")
	TMap<FVector, FVector> Slices;

	/** Array of valid vertices in frustum */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photo Info")
	TArray<FVector> ValidPoints;

	/** Counter for procedural cuts applied */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photo Info")
	int32 OldCuts = 0;
};