// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "CesiumTileManager.generated.h"

class ACesiumGeoreference;
class ACesium3DTileset;
class ACesiumCameraManager;
class ACesiumSunSky;
class ADepthCaptureSensor;
class UWorld;

UCLASS()
class AEROSIMCONNECTOR_API UCesiumTileManager : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void BeginPlay();

	UFUNCTION()
	void EndPlay();

	UFUNCTION()
	bool LoadTileSet(double Lat, double Long, double Height);

	UFUNCTION()
	bool SpawnAltitudeOffsetSensor();

	UFUNCTION()
	void DeleteAltitudeOffsetSensor();
	
	UFUNCTION()
	double MeasureAltitudeOffset();

	UFUNCTION()
	void SetCesiumJsonPath(FString CesiumPath) { CesiumJsonPath = CesiumPath; }

	UFUNCTION()
	FString GetCesiumJsonPath() { return CesiumJsonPath; }

	UFUNCTION()
	void SetCesiumIonId(int32 IonId) { CesiumIonID = IonId; }

	UFUNCTION()
	int32 GetCesiumIonId() { return CesiumIonID; }

	UFUNCTION()
	void SetSpectatorPawn(APawn* NewSpectator) { SpectatorPawn = NewSpectator; }

	UFUNCTION()
	void SetWorldReference(UWorld* NewWorldReference) { CurrentWorldReference = NewWorldReference; }

	UFUNCTION()
	FString RefactorPath(FString Path);

	UFUNCTION()
	ACesium3DTileset* SpawnCesiumTileset();

	UFUNCTION()
	ACesiumGeoreference* SpawnCesiumGeoReference();

	UFUNCTION()
	ACesiumCameraManager* SpawnCesiumCamera();

	UFUNCTION()
	ACesiumSunSky* SpawnCesiumSunSky();

	UFUNCTION()
	void PlaceGeoreferenceOriginHere(APawn* PawnToMove, ACesiumGeoreference* GeoRef);

	// Set origin latitude and longitude values in a GeoReference Actor
	UFUNCTION()
	void SetOriginLatitude(double Latitude);

	UFUNCTION()
	void SetOriginLongitude(double Longitude);

	UFUNCTION()
	void SetOriginHeight(double Height);

	UFUNCTION()
	ACesium3DTileset* GetCesiumTileset() { return CesiumTileset; }

	UFUNCTION(BlueprintCallable)
	void SetCesiumEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable)
	void SetCesiumWeatherEnabled(bool bEnabled);
private:
	UPROPERTY()
	ACesiumGeoreference* GeoReference;
	UPROPERTY()
	ACesium3DTileset* CesiumTileset;
	UPROPERTY()
	ACesiumCameraManager* CesiumCameraManager;
	UPROPERTY()
	ACesiumSunSky* CesiumSunSky;
	UPROPERTY()
	ADepthCaptureSensor* DepthCaptureActor;
	UPROPERTY()
	APawn* SpectatorPawn;
	UPROPERTY()
	UWorld* CurrentWorldReference;

	UPROPERTY(EditDefaultsOnly, Category = "Cesium Tile Loader", meta = (ToolTip = "Formatted to include cesium's file:/// starter in the path. Must end with .json"))
	FString CesiumJsonPath = "";

	UPROPERTY(EditDefaultsOnly, Category = "Cesium Tile Loader", meta = (ToolTip = "Cesium id asset"))
	int32 CesiumIonID = 2275207; // Google photorealistic 3D Cesium ID

	// Set initial pawn location's starting location to be a little above the local
	// UE origin that is generally set for ground level of the Cesium tile origin
	UPROPERTY()
	FVector DefaultPawnStartLocation = { 0.0f, 0.0f, 500.0f };
};
