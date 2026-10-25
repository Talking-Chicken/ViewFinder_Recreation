#include "ProceduralBaseActor.h"
#include "Components/StaticMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "Materials/MaterialInterface.h"

AProceduralBaseActor::AProceduralBaseActor()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;
	DefaultSceneRoot->SetMobility(EComponentMobility::Movable);

	PrimaryProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("PrimaryProceduralMesh"));
	PrimaryProceduralMesh->SetupAttachment(RootComponent);
	PrimaryProceduralMesh->SetMobility(EComponentMobility::Movable);

	PrimaryProceduralMesh->bUseComplexAsSimpleCollision = false;

	TemplateStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TemplateStaticMesh"));
	TemplateStaticMesh->SetupAttachment(RootComponent);
	TemplateStaticMesh->SetMobility(EComponentMobility::Movable);
}

void AProceduralBaseActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetupMesh();
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

	TemplateStaticMesh->SetStaticMesh(PhotoInfo.StaticMesh);

	UKismetProceduralMeshLibrary::CopyProceduralMeshFromStaticMeshComponent(
		TemplateStaticMesh,
		0,
		PrimaryProceduralMesh,
		true
	);

	TemplateStaticMesh->DestroyComponent();

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

		// Avoid adding duplicate slices within tolerance
		if (!AlmostContains(PhotoInfo.Slices, LocalNormal, LocalPosition))
		{
			PhotoInfo.Slices.Add(LocalNormal, LocalPosition);
			PhotoInfo.OldCuts++;
		}
	}

	TArray<UProceduralMeshComponent*> CurrentComps = ProceduralMeshComps;

	for (UProceduralMeshComponent* Comp : CurrentComps)
	{
		if (!Comp || Comp->GetOwner() != this)
		{
			continue;
		}

		UProceduralMeshComponent* OutOtherHalf = nullptr;

		UKismetProceduralMeshLibrary::SliceProceduralMesh(
			Comp,
			PlanePosition,
			PlaneNormal,
			true,
			OutOtherHalf,
			EProcMeshSliceCapOption::CreateNewSectionForCap,
			nullptr
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

		if (BoxExtent.X <= MinSize || BoxExtent.Y <= MinSize || BoxExtent.Z <= MinSize)
		{
			DestroyMesh(Comp);
		}
	}
}

void AProceduralBaseActor::SliceBasedOnPhoto(const FTransform& PhotoTransform, const FPhotoInfo& InPhotoInfo)
{
	for (const TPair<FVector, FVector>& Slice : InPhotoInfo.Slices)
	{
		FVector WorldNormal = PhotoTransform.TransformVectorNoScale(Slice.Key);
		FVector WorldPosition = PhotoTransform.TransformPosition(Slice.Value);

		SliceMesh(false, WorldPosition, WorldNormal);
	}

	PhotoInfo.OldCuts += InPhotoInfo.Slices.Num();
}

// --- NEW FUNCTION IMPLEMENTATIONS ---

FVector AProceduralBaseActor::GetRelativeBoundsOrigin(UProceduralMeshComponent* ProcMeshComp) const
{
	if (!ProcMeshComp)
	{
		return FVector::ZeroVector;
	}

	// Converts component world-space bounds center to local space relative to Actor
	return GetActorTransform().InverseTransformPosition(ProcMeshComp->Bounds.Origin);
}

void AProceduralBaseActor::DetermineValidPoints(const TArray<FPlane>& FrustumPlanes)
{
	PhotoInfo.ValidPoints.Empty();

	const FTransform& ActorXform = GetActorTransform();

	for (UProceduralMeshComponent* Comp : ProceduralMeshComps)
	{
		if (!Comp) continue;

		const int32 NumSections = Comp->GetNumSections();
		for (int32 SecIdx = 0; SecIdx < NumSections; ++SecIdx)
		{
			FProcMeshSection* Section = Comp->GetProcMeshSection(SecIdx);
			if (!Section) continue;

			for (const FProcMeshVertex& Vertex : Section->ProcVertexBuffer)
			{
				FVector WorldPoint = ActorXform.TransformPosition(Vertex.Position);
				bool bInsideFrustum = true;

				// Check point against all camera frustum planes
				for (const FPlane& Plane : FrustumPlanes)
				{
					if (Plane.PlaneDot(WorldPoint) < 0.0f) // Point is behind plane
					{
						bInsideFrustum = false;
						break;
					}
				}

				if (bInsideFrustum)
				{
					PhotoInfo.ValidPoints.AddUnique(Vertex.Position);
				}
			}
		}
	}
}

bool AProceduralBaseActor::AlmostContains(const TMap<FVector, FVector>& SliceMap, const FVector& Normal, const FVector& Position, float Tolerance) const
{
	for (const TPair<FVector, FVector>& Pair : SliceMap)
	{
		if (Pair.Key.Equals(Normal, Tolerance) && Pair.Value.Equals(Position, Tolerance))
		{
			return true;
		}
	}
	return false;
}

void AProceduralBaseActor::ClearExtraSlices(float Tolerance)
{
	TMap<FVector, FVector> CleanedSlices;

	for (const TPair<FVector, FVector>& Slice : PhotoInfo.Slices)
	{
		if (!AlmostContains(CleanedSlices, Slice.Key, Slice.Value, Tolerance))
		{
			CleanedSlices.Add(Slice.Key, Slice.Value);
		}
	}

	PhotoInfo.Slices = CleanedSlices;
}