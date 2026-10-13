#include "ProceduralBaseActor.h"
#include "Components/StaticMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "Materials/MaterialInterface.h"

AProceduralBaseActor::AProceduralBaseActor()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	PrimaryProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("PrimaryProceduralMesh"));
	PrimaryProceduralMesh->SetupAttachment(RootComponent);

	// Disable complex collision as simple for procedural slicing
	PrimaryProceduralMesh->bUseComplexAsSimpleCollision = false;

	TemplateStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TemplateStaticMesh"));
	TemplateStaticMesh->SetupAttachment(RootComponent);
}

void AProceduralBaseActor::BeginPlay()
{
	Super::BeginPlay();

	if (PrimaryProceduralMesh)
	{
		ProceduralMeshComps.AddUnique(PrimaryProceduralMesh);
	}

	SetupMesh();
}

void AProceduralBaseActor::SetupMesh()
{
	if (!PhotoInfo.StaticMesh || !TemplateStaticMesh || !PrimaryProceduralMesh)
	{
		return;
	}

	// 1. Assign source static mesh
	TemplateStaticMesh->SetStaticMesh(PhotoInfo.StaticMesh);

	// 2. Copy static mesh geometry into the procedural mesh component
	UKismetProceduralMeshLibrary::CopyProceduralMeshFromStaticMeshComponent(
		TemplateStaticMesh,
		0,
		PrimaryProceduralMesh,
		true // Create Collision
	);

	// 3. Destroy temporary static mesh component
	TemplateStaticMesh->DestroyComponent();

	// 4. Assign materials to procedural sections
	for (int32 Index = 0; Index < PhotoInfo.Materials.Num(); ++Index)
	{
		if (PhotoInfo.Materials[Index])
		{
			PrimaryProceduralMesh->SetMaterial(Index, PhotoInfo.Materials[Index]);
		}
	}

	ClearSmallCuts(5.0f);
}

TArray<UProceduralMeshComponent*> AProceduralBaseActor::SliceMesh(bool bAddCuts, FVector PlanePosition, FVector PlaneNormal)
{
	TArray<UProceduralMeshComponent*> OutReturnMeshes;

	if (bAddCuts)
	{
		FVector LocalNormal = GetActorTransform().InverseTransformVectorNoScale(PlaneNormal);
		FVector LocalPosition = GetActorTransform().InverseTransformPosition(PlanePosition);

		PhotoInfo.Slices.Add(LocalNormal, LocalPosition);
		PhotoInfo.OldCuts++;
	}

	TArray<UProceduralMeshComponent*> CurrentComps = ProceduralMeshComps;

	for (UProceduralMeshComponent* Comp : CurrentComps)
	{
		if (!Comp || Comp->GetOwner() != this)
		{
			continue;
		}

		UProceduralMeshComponent* OutOtherHalf = nullptr;

		// Corrected function call and enum specifier:
		UKismetProceduralMeshLibrary::SliceProceduralMesh(
			Comp,
			PlanePosition,
			PlaneNormal,
			true, // bCreateOtherHalf
			OutOtherHalf,
			EProcMeshSliceCapOption::CreateNewSectionForCap,
			nullptr // Cap Material
		);

		if (OutOtherHalf)
		{
			OutReturnMeshes.Add(OutOtherHalf);
			ProceduralMeshComps.AddUnique(OutOtherHalf);
		}
	}

	return OutReturnMeshes;
}

void AProceduralBaseActor::DestroyMesh(UProceduralMeshComponent* ProcMeshToDestroy)
{
	if (!ProcMeshToDestroy)
	{
		return;
	}

	ProceduralMeshComps.Remove(ProcMeshToDestroy);
	ProcMeshToDestroy->DestroyComponent();

	// Destroy parent actor if all procedural mesh parts are gone
	if (ProceduralMeshComps.Num() == 0)
	{
		Destroy();
	}
}

void AProceduralBaseActor::ClearSmallCuts(float MinSize)
{
	TArray<UProceduralMeshComponent*> CurrentComps = ProceduralMeshComps;

	for (UProceduralMeshComponent* Comp : CurrentComps)
	{
		if (!Comp) continue;

		FVector BoxExtent = Comp->Bounds.BoxExtent;

		// Destroy fragments smaller than threshold
		if (BoxExtent.X <= MinSize || BoxExtent.Y <= MinSize || BoxExtent.Z <= MinSize)
		{
			DestroyMesh(Comp);
		}
	}
}

void AProceduralBaseActor::SliceBasedOnPhoto(const FTransform& PhotoTransform, const FPhotoInfo& InPhotoInfo)
{
	// Replay saved photo slices in world space
	for (const TPair<FVector, FVector>& Slice : InPhotoInfo.Slices)
	{
		FVector WorldNormal = PhotoTransform.TransformVectorNoScale(Slice.Key);
		FVector WorldPosition = PhotoTransform.TransformPosition(Slice.Value);

		SliceMesh(false, WorldPosition, WorldNormal);
	}

	PhotoInfo.OldCuts += InPhotoInfo.Slices.Num();
}