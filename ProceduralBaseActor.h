#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "PhotoInfo.h"
#include "ProceduralBaseActor.generated.h"

class UStaticMeshComponent;
class UProceduralMeshComponent;
class UMaterialInterface;

UCLASS()
class VIEWFINDER_API AProceduralBaseActor : public AActor
{
	GENERATED_BODY()

public:
	AProceduralBaseActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProceduralMeshComponent> PrimaryProceduralMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> TemplateStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "Viewfinder")
	FPhotoInfo PhotoInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Viewfinder")
	TArray<TObjectPtr<UProceduralMeshComponent>> ProceduralMeshComps;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Viewfinder|Procedural")
	virtual void SetupMesh();

	UFUNCTION(BlueprintCallable, Category = "Viewfinder|Procedural")
	TArray<UProceduralMeshComponent*> SliceMesh(bool bAddCuts, FVector PlanePosition, FVector PlaneNormal);

	UFUNCTION(BlueprintCallable, Category = "Viewfinder|Procedural")
	void DestroyMesh(UProceduralMeshComponent* ProcMeshToDestroy);

	UFUNCTION(BlueprintCallable, Category = "Viewfinder|Procedural")
	void ClearSmallCuts(float MinSize = 5.0f);

	UFUNCTION(BlueprintCallable, Category = "Viewfinder|Procedural")
	void SliceBasedOnPhoto(const FTransform& PhotoTransform, const FPhotoInfo& InPhotoInfo);

	// --- Frustum & Boundary Utilities ---

	/** Calculates a component's bounding box center relative to the Actor's local origin */
	UFUNCTION(BlueprintCallable, Category = "Viewfinder|Procedural")
	FVector GetRelativeBoundsOrigin(UProceduralMeshComponent* ProcMeshComp) const;

	/** Filters procedural mesh vertices against view frustum planes and stores valid points in PhotoInfo */
	UFUNCTION(BlueprintCallable, Category = "Viewfinder|Procedural")
	void DetermineValidPoints(const TArray<FPlane>& FrustumPlanes);

	/** Helper to check if a slice plane (Normal & Position) already exists within floating-point tolerance */
	UFUNCTION(BlueprintPure, Category = "Viewfinder|Procedural")
	bool AlmostContains(const TMap<FVector, FVector>& SliceMap, const FVector& Normal, const FVector& Position, float Tolerance = 0.1f) const;

	/** Removes redundant or nearly identical duplicate slice planes from PhotoInfo */
	UFUNCTION(BlueprintCallable, Category = "Viewfinder|Procedural")
	void ClearExtraSlices(float Tolerance = 0.1f);
};