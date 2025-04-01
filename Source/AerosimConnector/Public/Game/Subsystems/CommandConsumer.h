#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Game/Subsystems/SceneGraph.h"
#include "CommandConsumer.generated.h"

class UCesiumTileManager;
class UActorRegistry;
class AAerosimGameMode;

USTRUCT(BlueprintType)
struct FMeasureAltitudeOffsetCommandParams
{
	GENERATED_BODY()

	UPROPERTY()
	FString UUID;

	UPROPERTY()
	double Lat = 0.0;

	UPROPERTY()
	double Lon = 0.0;

	UPROPERTY()
	double ExpectedAltitude = 0.0;
};

UCLASS()
class AEROSIMCONNECTOR_API UCommandConsumer : public UObject
{
	GENERATED_BODY()

public:
	UCommandConsumer() {};
	~UCommandConsumer() {};

	void SetActorRegistry(UActorRegistry* ActorRegistry);
	void SetCesiumTileManager(UCesiumTileManager* NewCesiumTileManager);

	void ProcessCommandsFromQueue(float DeltaSeconds);

	void UpdateSceneFromSceneGraph(const FString& SceneGraphJson);

	UPROPERTY()
	AAerosimGameMode* GameMode;

private:
	void ConfigureSceneCommand(TSharedPtr<FJsonObject> JsonObject);
	void SpawnActorCommand(TSharedPtr<FJsonObject> JsonObject);
	void SpawnActorByNameCommand(TSharedPtr<FJsonObject> JsonObject);
	void SetActorTransformCommand(TSharedPtr<FJsonObject> JsonObject);
	void SetActorTransformCommand(uint32 InstanceId, FVector Translation, FRotator Rotation);
	void DeleteActorCommand(TSharedPtr<FJsonObject> JsonObject);
	void SpawnSensorCommand(TSharedPtr<FJsonObject> JsonObject);
	void TransformSensorCommand(TSharedPtr<FJsonObject> JsonObject);
	void DeleteSensorCommand(TSharedPtr<FJsonObject> JsonObject);
	void AttachSensorCommand(TSharedPtr<FJsonObject> JsonObject);
	void PossessSensorCommand(TSharedPtr<FJsonObject> JsonObject);
	void LoadCoordinatesCommand(TSharedPtr<FJsonObject> JsonObject);
	void AttachSpectatorToSocketCommand(TSharedPtr<FJsonObject> JsonObject);
	void LoadUSDInActorCommand(TSharedPtr<FJsonObject> JsonObject);
	void AttachActorToActorWithSocketCommand(TSharedPtr<FJsonObject> JsonObject);
	void MeasureAltitudeOffsetCommand(TSharedPtr<FJsonObject> JsonObject);
	void VisualizeTrajectoryCommand(TSharedPtr<FJsonObject> JsonObject);
	void VisualizeTrajectorySettingsCommand(TSharedPtr<FJsonObject> JsonObject);
	void SetUserDefinedWaypointsCommand(TSharedPtr<FJsonObject> JsonObject);
	void VisualizeTrajectoryAddAheadTrajectory(TSharedPtr<FJsonObject> JsonObject);
	void LoadWeatherPreset(TSharedPtr<FJsonObject> JsonObject);
	void SetCesiumEnabled(TSharedPtr<FJsonObject> JsonObject);
	void SetCesiumWeatherEnabled(TSharedPtr<FJsonObject> JsonObject);

	// Scene graph functions

	void SpawnActorsIfNeeded(FSceneGraph& SceneGraph);
	void UpdateResourcesFromSceneGraph(const FSceneGraph& SceneGraph);
	void UpdateActorTransformsFromSceneGraph(const FSceneGraph& SceneGraph);
	void UpdateEffectorsFromSceneGraph(FSceneGraph& SceneGraph);
	void UpdatePFDsFromSceneGraph(const FSceneGraph& SceneGraph);
	void UpdateTrajectoryVisualizationSettingsFromSceneGraph(const FSceneGraph& SceneGraph);
	void UpdateTrajectoryVisualizationUserDefinedWaypointsFromSceneGraph(const FSceneGraph& SceneGraph);
	void UpdateTrajectoryVisualizationFutureTrajectoryWaypointsFromSceneGraph(const FSceneGraph& SceneGraph);

	UFUNCTION()
	void OnTilesetLoaded();

	UFUNCTION()
	void AdjustDepthSensorOffset();

	UFUNCTION()
	void MeasureAltitudeOffsetResponse();

	FTimerHandle DepthSensorDelayTimerHandle;
	double InitialDepthSensorOffset = 500.0;
	double CurrentDepthSensorOffset = 0.0;

	UPROPERTY()
	UActorRegistry* Registry;

	UPROPERTY(VisibleAnywhere)
	TMap<FString, uint32> ActorNameIdMap;

	UPROPERTY(VisibleAnywhere)
	TMap<FString, FEffectorInitialTransforms> EffectorInitTransformMap;

	UPROPERTY()
	UCesiumTileManager* CesiumTileManager;

	UPROPERTY()
	uint32 MAX_MESSAGES_PER_TICK = 500;

	UPROPERTY()
	bool bRealTimePacingMode = true;

	FDateTime LastTickTime;
	double LastConsumedTimestamp = -1.0;

	UPROPERTY()
	double COMMAND_BUFFER_MAX_SEC = 0.05;

	UPROPERTY()
	FMeasureAltitudeOffsetCommandParams CachedMeasureAltitudeOffsetCommandParams;

	FVector CachedOrigin;

	bool bFirstTime = true;
};
