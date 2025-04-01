#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "SceneGraph.generated.h"

USTRUCT(BlueprintType)
struct FTransformSceneGraph
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	FVector Position = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	FVector Scale = FVector::OneVector;
};

USTRUCT(BlueprintType)
struct FActorState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor State")
	FTransformSceneGraph Pose;
};

USTRUCT(BlueprintType)
struct FActorProperties
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Properties")
	FString ActorName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Properties")
	FString ActorAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Properties")
	FString Parent;
};

USTRUCT(BlueprintType)
struct FSensorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
	FString SensorName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
	FString SensorType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
	FVector2D Resolution = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
	float TickRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
	float FOV = 90.0f; // default FOVAngle from Unreal's USceneCaptureComponent2D

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
	float NearClip = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
	float FarClip = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
	TEnumAsByte<ECameraProjectionMode::Type> ProjectionMode = ECameraProjectionMode::Type::Perspective;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
	float OrthoWidth = 1536.0f; // DEFAULT_ORTHOWIDTH from Unreal's EngineDefines.h

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
	bool bCaptureEnabled = true;
};

USTRUCT(BlueprintType)
struct FEffectorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effector")
	FString EffectorID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effector")
	FString USDPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effector")
	FTransformSceneGraph Transform;
};

USTRUCT(BlueprintType)
struct FEffectorInitialTransforms
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effector")
	TMap<FString, FTransformSceneGraph> EffectorInitialTransformMap;
};

USTRUCT(BlueprintType)
struct FPrimaryFlightDisplayData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PrimayFlightDisplay")
	double AirspeedKts = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PrimayFlightDisplay")
	double TrueAirspeedKts = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PrimayFlightDisplay")
	double AltitudeFt = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PrimayFlightDisplay")
	double TargetAltitudeFt = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PrimayFlightDisplay")
	double AltimeterPressureSettingInHg = 29.92;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PrimayFlightDisplay")
	double VerticalSpeedFpm = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PrimayFlightDisplay")
	double PitchDeg = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PrimayFlightDisplay")
	double RollDeg = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PrimayFlightDisplay")
	double SideSlipFps2 = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PrimayFlightDisplay")
	double HeadingDeg = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PrimayFlightDisplay")
	double HsiCourseSelectHeadingDeg = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PrimayFlightDisplay")
	double HsiCourseDeviationDeg = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PrimayFlightDisplay")
	int32 HsiMode = 0;
};

// This struct is need as TMap<, Tarray> is not supported
USTRUCT(BlueprintType)
struct FEntityComponentList
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	TArray<FString> Components;
};

// This struct is need as TMap<, Tarray> is not supported
USTRUCT(BlueprintType)
struct FEffectorList
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effectors")
	TArray<FEffectorData> Effectors;
};

USTRUCT(BlueprintType)
struct FTrajectoryVisualizationSettingsData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory Visualization")
	bool DisplayFutureTrajectory = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory Visualization")
	bool DisplayPastTrajectory = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory Visualization")
	bool HighlightUserDefinedWaypoints = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory Visualization")
	int NumberOfFutureWaypoints = 1;
};

USTRUCT(BlueprintType)
struct FTrajectoryVisualizationWaypointsData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trajectory Visualization")
	TArray<FVector> Waypoints;
};

USTRUCT(BlueprintType)
struct FEntityComponents
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	TMap<FString, FActorProperties> ActorProperties;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	TMap<FString, FActorState> ActorStates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	TMap<FString, FSensorData> Sensors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	TMap<FString, FEffectorList> Effectors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	TMap<FString, FPrimaryFlightDisplayData> PrimaryFlightDisplays;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	TMap<FString, FTrajectoryVisualizationSettingsData> TrajectoryVisualizationSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	TMap<FString, FTrajectoryVisualizationWaypointsData> TrajectoryVisualizationUserDefinedWaypoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
	TMap<FString, FTrajectoryVisualizationWaypointsData> TrajectoryVisualizationFutureTrajectoryWaypoints;
};

USTRUCT(BlueprintType)
struct FWeather
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	FString Preset;
};

USTRUCT(BlueprintType)
struct FViewportConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	FString ActiveViewport;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	FString RendererInstanceID;
};

USTRUCT(BlueprintType)
struct FResources
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	FVector Origin = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	FWeather Weather;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	FViewportConfig ViewportConfig;
	
	bool bResourcesSet = false;
};

USTRUCT(BlueprintType)
struct FSceneGraph
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SceneGraph")
	FResources Resources;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SceneGraph")
	TMap<FString, FEntityComponentList> Entities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SceneGraph")
	FEntityComponents Components;
};
