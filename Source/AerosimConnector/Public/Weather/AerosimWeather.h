// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "USDStageActor.h"
#include "ObjectLabel.h"

#include "AerosimWeather.generated.h"

class UExponentialHeightFogComponent;
class UDirectionalLightComponent;
class USkyLightComponent;
class UVolumetricCloudComponent;
class USkyAtmosphereComponent;
class UWeatherMapDataAsset;

UCLASS(Blueprintable)
class AEROSIMCONNECTOR_API AAerosimWeather : public AActor
{
	GENERATED_BODY()

public:
	AAerosimWeather(
	const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* SceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UExponentialHeightFogComponent* ExponentialHeightFogComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USkyLightComponent* SkyLightComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UVolumetricCloudComponent* VolumetricCloudComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USkyAtmosphereComponent* SkyAtmosphereComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	FString WeatherPresetName;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Weather")
	void SaveWeatherPresetData();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Weather")
	void LoadWeatherPresetData();

	UFUNCTION(BlueprintCallable, Category = "Weather")
	void LoadWeatherPresetDataByName(const FString& WeatherPreset);

	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetWeatherDataAsset(UWeatherMapDataAsset* WeatherDataAsset);

private:
	UFUNCTION()
	void ApplyLoadedWeatherPreset(const FString& JsonString);

	void InitializeReferences();

	UPROPERTY()
	UWeatherMapDataAsset* WeatherMapDataAsset;

};
