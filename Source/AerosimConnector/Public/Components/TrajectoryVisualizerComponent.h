#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "TrajectoryVisualizerComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class AEROSIMCONNECTOR_API UTrajectoryVisualizerComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UTrajectoryVisualizerComponent();

	inline bool IsEnabled() const { return bEnabled; }
	void Update(FVector Position);
	void UpdateUserDefinedWaypoints(const TArray<FVector>& Waypoints);
	void UpdateFutureTrajectory(const TArray<FVector>& Waypoints);
	void UpdateSettings(bool bDisplayFutureTrajectory, bool bDisplayPastWaypoints, bool bHighlightUserDefinedWaypoints, uint32 FutureWaypoints);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	bool bEnabled{ false };
	UPROPERTY(VisibleAnywhere)
	bool bDisplayFutureTrajectory{ false };
	UPROPERTY(VisibleAnywhere)
	bool bDisplayPastWaypoints{ false };
	UPROPERTY(VisibleAnywhere)
	bool bHighlightUserDefinedWaypoints{ false };
	UPROPERTY()
	uint32 NumberOfFutureWaypoints{ 1 };

private:
	int32 LastSplinePointIndex{ 0 };
	UPROPERTY()
	USplineComponent* SplineComponent{ nullptr };
	UPROPERTY()
	TArray<USplineMeshComponent*> SplineMeshes{};
	UPROPERTY()
	USplineComponent* FutureSplineComponent{ nullptr };
	UPROPERTY()
	TArray<USplineMeshComponent*> FutureSplineMeshes{};
	UPROPERTY()
	TArray<UStaticMeshComponent*> UserDefinedWaypoints{};

private:
	UPROPERTY()
	UMaterialInterface* SplineBaseMaterial;
	UPROPERTY()
	UMaterialInterface* WaypointBaseMaterial;
	UPROPERTY()
	UMaterialInstanceDynamic* SplineMaterial;
	UPROPERTY()
	UMaterialInstanceDynamic* WaypointMaterial;

	static UStaticMesh* CubeMesh;
	static UStaticMesh* SphereMesh;

private:
	void DrawUserDefinedWaypoints(const TArray<FVector>& Waypoints);
	void DrawSpline();
	void DrawFutureSpline();
};
