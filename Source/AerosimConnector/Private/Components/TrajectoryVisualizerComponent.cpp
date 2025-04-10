#include "Components/TrajectoryVisualizerComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

UStaticMesh* UTrajectoryVisualizerComponent::CubeMesh = nullptr;
UStaticMesh* UTrajectoryVisualizerComponent::SphereMesh = nullptr;

UTrajectoryVisualizerComponent::UTrajectoryVisualizerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTrajectoryVisualizerComponent::BeginPlay()
{
	Super::BeginPlay();

	SplineBaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/aerosim-assets/Materials/Instances/BaseColors/MI_Pink"));
	if (!IsValid(SplineBaseMaterial))
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("Failed to load SplineBaseMaterial"));
		return;
	}

	WaypointBaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/aerosim-assets/Materials/Instances/BaseColors/MI_Green"));
	if (!IsValid(WaypointBaseMaterial))
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("Failed to load WaypointBaseMaterial"));
		return;
	}

	SplineMaterial = UMaterialInstanceDynamic::Create(SplineBaseMaterial, this);
	if (!IsValid(SplineMaterial))
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("Failed to create SplineMaterial"));
		return;
	}
	SplineMaterial->SetVectorParameterValue("BaseColor", FLinearColor(1.0f, 0.0f, 1.0f, 1.0f));

	WaypointMaterial = UMaterialInstanceDynamic::Create(WaypointBaseMaterial, this);
	if (!IsValid(WaypointMaterial))
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("Failed to create WaypointMaterial"));
		return;
	}
	WaypointMaterial->SetVectorParameterValue("BaseColor", FLinearColor(0.0f, 1.0f, 0.0f, 1.0f));
}

void UTrajectoryVisualizerComponent::Update(FVector Position)
{
	if (!bEnabled)
	{
		UE_LOG(LogAerosimConnector, VeryVerbose, TEXT("Actor: %s -> Has not enabled the Trajectory Visualizer Component"), *GetName());
		return;
	}
	if (!SplineComponent)
	{
		SplineComponent = NewObject<USplineComponent>(this, TEXT("SplineComponent"));
		SplineComponent->SetMobility(EComponentMobility::Movable);
		SplineComponent->RegisterComponentWithWorld(GetWorld());
		SplineComponent->SetVisibility(false);
		SplineComponent->bDrawDebug = false;
	}
	SplineComponent->AddSplinePoint(Position, ESplineCoordinateSpace::World, true);
	SplineComponent->UpdateSpline();
	DrawSpline();
}

void UTrajectoryVisualizerComponent::UpdateUserDefinedWaypoints(const TArray<FVector>& Waypoints)
{
	if (!bEnabled)
	{
		UE_LOG(LogAerosimConnector, VeryVerbose, TEXT("Actor: %s -> Has not enabled the Trajectory Visualizer Component"), *GetName());
		return;
	}
	if (UserDefinedWaypoints.Num() > 0)
	{
		return;
	}
	SplineMeshes.Empty();
	if (!SplineComponent)
	{
		SplineComponent = NewObject<USplineComponent>(this, TEXT("SplineComponent"));
		SplineComponent->SetMobility(EComponentMobility::Movable);
		SplineComponent->RegisterComponentWithWorld(GetWorld());
		SplineComponent->SetVisibility(false);
		SplineComponent->bDrawDebug = false;
	}
	DrawUserDefinedWaypoints(Waypoints);
}

void UTrajectoryVisualizerComponent::UpdateFutureTrajectory(const TArray<FVector>& Waypoints)
{
	if (!bEnabled)
	{
		UE_LOG(LogAerosimConnector, VeryVerbose, TEXT("Actor: %s -> Has not enabled the Trajectory Visualizer Component"), *GetName());
		return;
	}

	if (Waypoints.Num() < 2)
	{
		return;
	}

	for (USplineMeshComponent* SplineMesh : FutureSplineMeshes)
	{
		if (IsValid(SplineMesh))
		{
			SplineMesh->DestroyComponent();
		}
	}

	if (!FutureSplineComponent)
	{
		FutureSplineComponent = NewObject<USplineComponent>(this, TEXT("FutureSplineComponent"));
		FutureSplineComponent->SetMobility(EComponentMobility::Movable);
		FutureSplineComponent->RegisterComponentWithWorld(GetWorld());
		FutureSplineComponent->SetVisibility(false);
		FutureSplineComponent->bDrawDebug = false;
	}

	FutureSplineComponent->ClearSplinePoints();
	FutureSplineComponent->SetSplinePoints(Waypoints, ESplineCoordinateSpace::World, false);
	FutureSplineComponent->UpdateSpline();
	DrawFutureSpline();
}

void UTrajectoryVisualizerComponent::UpdateSettings(bool ShowFutureWaypoints, bool ShowBehindWaypoints, bool ShowUserDefinedWaypoints, uint32 FutureWaypoints)
{
	bDisplayFutureTrajectory = ShowFutureWaypoints;
	bDisplayPastWaypoints = ShowBehindWaypoints;
	bHighlightUserDefinedWaypoints = ShowUserDefinedWaypoints;
	NumberOfFutureWaypoints = FutureWaypoints;
	bEnabled = (bDisplayFutureTrajectory || bDisplayPastWaypoints);
}

void UTrajectoryVisualizerComponent::DrawSpline()
{
	if (!bEnabled)
	{
		UE_LOG(LogAerosimConnector, VeryVerbose, TEXT("Actor: %s -> Has not enabled the Trajectory Visualizer Component"), *GetName());
		return;
	}
	const int32 NumPoints = SplineComponent->GetNumberOfSplinePoints();

	if (NumPoints < 2 || LastSplinePointIndex >= NumPoints - 1)
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("Not enough points to draw Spline"));
		return;
	}

	USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
	if (!IsValid(SplineMesh))
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("Failed to create SplineMesh"));
		return;
	}
	SplineMesh->SetMobility(EComponentMobility::Movable);
	SplineMesh->RegisterComponentWithWorld(GetWorld());

	FVector StartPos, StartTangent, EndPos, EndTangent;
	SplineComponent->GetLocationAndTangentAtSplinePoint(LastSplinePointIndex, StartPos, StartTangent, ESplineCoordinateSpace::World);
	SplineComponent->GetLocationAndTangentAtSplinePoint(LastSplinePointIndex + 1, EndPos, EndTangent, ESplineCoordinateSpace::World);
	SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);

	if (!IsValid(CubeMesh))
	{
		CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		if (!IsValid(CubeMesh))
		{
			UE_LOG(LogAerosimConnector, Error, TEXT("Failed to load Cube Mesh"));
			return;
		}
	}

	SplineMesh->SetStaticMesh(CubeMesh);
	if (IsValid(SplineMaterial))
	{
		SplineMesh->SetMaterial(0, SplineMaterial);
	}

	SplineMesh->SetStartScale(FVector2D(0.1f, 0.1f));
	SplineMesh->SetEndScale(FVector2D(0.1f, 0.1f));
	SplineMesh->SetVisibility(true);
	SplineMesh->SetHiddenInGame(false);
	SplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SplineMesh->SetMobility(EComponentMobility::Movable);

	SplineMeshes.Add(SplineMesh);
	LastSplinePointIndex++;
}

void UTrajectoryVisualizerComponent::DrawFutureSpline()
{
	if (!bEnabled)
	{
		UE_LOG(LogAerosimConnector, VeryVerbose, TEXT("Actor: %s -> Has not enabled the Trajectory Visualizer Component"), *GetName());
		return;
	}

	if (!IsValid(CubeMesh))
	{
		CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		if (!IsValid(CubeMesh))
		{
			UE_LOG(LogAerosimConnector, Error, TEXT("Failed to load Cube Mesh"));
			return;
		}
	}

	const int32 NumPoints = FutureSplineComponent->GetNumberOfSplinePoints();
	for (int32 PointIndex = 0; PointIndex < NumPoints - 1; ++PointIndex)
	{
		USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
		if (!IsValid(SplineMesh))
		{
			UE_LOG(LogAerosimConnector, Error, TEXT("Failed to create SplineMesh at index %d"), PointIndex);
			continue;
		}

		SplineMesh->SetMobility(EComponentMobility::Movable);
		SplineMesh->RegisterComponentWithWorld(GetWorld());

		FVector StartPos, StartTangent, EndPos, EndTangent;
		FutureSplineComponent->GetLocationAndTangentAtSplinePoint(PointIndex, StartPos, StartTangent, ESplineCoordinateSpace::World);
		FutureSplineComponent->GetLocationAndTangentAtSplinePoint(PointIndex + 1, EndPos, EndTangent, ESplineCoordinateSpace::World);
		SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);

		SplineMesh->SetStaticMesh(CubeMesh);
		if (IsValid(SplineMaterial))
		{
			SplineMesh->SetMaterial(0, SplineMaterial);
		}

		SplineMesh->SetStartScale(FVector2D(0.1f, 0.1f));
		SplineMesh->SetEndScale(FVector2D(0.1f, 0.1f));
		SplineMesh->SetVisibility(true);
		SplineMesh->SetHiddenInGame(false);
		SplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SplineMesh->SetMobility(EComponentMobility::Movable);
		FutureSplineMeshes.Add(SplineMesh);
	}
}

void UTrajectoryVisualizerComponent::DrawUserDefinedWaypoints(const TArray<FVector>& Waypoints)
{
	for (const FVector& Waypoint : Waypoints)
	{
		UStaticMeshComponent* SphereComponent = NewObject<UStaticMeshComponent>(this);

		if (!IsValid(SphereComponent))
		{
			UE_LOG(LogAerosimConnector, Error, TEXT("Failed to create SphereComponent"));
			return;
		}
		SphereComponent->SetWorldLocation(Waypoint);
		SphereComponent->RegisterComponentWithWorld(GetWorld());

		if (!IsValid(SphereMesh))
		{
			SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
			if (!IsValid(SphereMesh))
			{
				UE_LOG(LogAerosimConnector, Error, TEXT("Failed to load Sphere Mesh"));
				return;
			}
		}
		if (IsValid(SphereMesh))
		{
			SphereComponent->SetStaticMesh(SphereMesh);
		}
		if (IsValid(WaypointMaterial))
		{
			SphereComponent->SetMaterial(0, WaypointMaterial);
		}

		SphereComponent->SetWorldScale3D(FVector(1.0f));
		SphereComponent->SetSimulatePhysics(false);
		SphereComponent->SetVisibility(true);
		SphereComponent->SetHiddenInGame(false);
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		SphereComponent->RegisterComponent();
		UserDefinedWaypoints.Add(SphereComponent);
	}
}
