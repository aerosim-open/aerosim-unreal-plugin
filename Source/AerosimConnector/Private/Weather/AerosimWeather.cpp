// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Weather/AerosimWeather.h"
#include "Weather/WeatherMapDataAsset.h"
#include "Components/SceneComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

FLinearColor JsonToColor(TSharedPtr<FJsonObject> JsonColor)
{
	if (!JsonColor.IsValid())
		return FLinearColor::White; // Default color if invalid

	FLinearColor Color;
	Color.R = JsonColor->GetNumberField("R");
	Color.G = JsonColor->GetNumberField("G");
	Color.B = JsonColor->GetNumberField("B");
	Color.A = JsonColor->GetNumberField("A");
	return Color;
}

TSharedPtr<FJsonObject> ColorToJson(const FLinearColor& Color)
{
	TSharedPtr<FJsonObject> JsonColor = MakeShareable(new FJsonObject());
	JsonColor->SetNumberField("R", Color.R);
	JsonColor->SetNumberField("G", Color.G);
	JsonColor->SetNumberField("B", Color.B);
	JsonColor->SetNumberField("A", Color.A);
	return JsonColor;
}

AAerosimWeather::AAerosimWeather(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	RootComponent = SceneComponent;

	ExponentialHeightFogComponent = CreateDefaultSubobject<UExponentialHeightFogComponent>("ExponentialHeightFogComponent");
	ExponentialHeightFogComponent->SetupAttachment(RootComponent);

	VolumetricCloudComponent = CreateDefaultSubobject<UVolumetricCloudComponent>("VolumetricCloudComponent");
	VolumetricCloudComponent->SetupAttachment(RootComponent);
}

void AAerosimWeather::BeginPlay()
{
	Super::BeginPlay();
	InitializeReferences();
}

void AAerosimWeather::SaveWeatherPresetData()
{
	InitializeReferences();
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	// Save Exponential Height Fog properties
	JsonObject->SetNumberField("FogDensity", ExponentialHeightFogComponent->FogDensity);
	JsonObject->SetNumberField("FogHeightFalloff", ExponentialHeightFogComponent->FogHeightFalloff);
	JsonObject->SetNumberField("SecondFogDensity", ExponentialHeightFogComponent->SecondFogData.FogDensity);
	JsonObject->SetNumberField("SecondFogHeightFalloff", ExponentialHeightFogComponent->SecondFogData.FogHeightFalloff);
	JsonObject->SetNumberField("SecondFogHeightOffset", ExponentialHeightFogComponent->SecondFogData.FogHeightFalloff);
	JsonObject->SetNumberField("FogMaxOpacity", ExponentialHeightFogComponent->FogMaxOpacity);
	JsonObject->SetNumberField("StartDistance", ExponentialHeightFogComponent->StartDistance);
	JsonObject->SetNumberField("FogCutoffDistance", ExponentialHeightFogComponent->FogCutoffDistance);

	JsonObject->SetNumberField("DirectionalInScatteringExponent", ExponentialHeightFogComponent->DirectionalInscatteringExponent);
	JsonObject->SetNumberField("DirectionalInScatteringStartDistance", ExponentialHeightFogComponent->DirectionalInscatteringStartDistance);
	JsonObject->SetObjectField("DirectionalInScatteringColor", ColorToJson(ExponentialHeightFogComponent->DirectionalInscatteringLuminance));

	JsonObject->SetBoolField("bVolumetricFog", ExponentialHeightFogComponent->bEnableVolumetricFog);
	JsonObject->SetNumberField("VolumetricFogScatteringDistribution", ExponentialHeightFogComponent->VolumetricFogScatteringDistribution);
	JsonObject->SetObjectField("VolumetricFogAlbedo", ColorToJson(ExponentialHeightFogComponent->VolumetricFogAlbedo));
	JsonObject->SetObjectField("VolumetricFogEmissive", ColorToJson(ExponentialHeightFogComponent->VolumetricFogEmissive));
	JsonObject->SetNumberField("VolumetricFogExtinctionScale", ExponentialHeightFogComponent->VolumetricFogExtinctionScale);
	JsonObject->SetNumberField("VolumetricFogViewDistance", ExponentialHeightFogComponent->VolumetricFogDistance);
	JsonObject->SetNumberField("VolumetricFogNearInDistance", ExponentialHeightFogComponent->VolumetricFogNearFadeInDistance);
	JsonObject->SetNumberField("VolumetricFogStaticLightingScatteringIntensity", ExponentialHeightFogComponent->VolumetricFogStaticLightingScatteringIntensity);

	// Save Volumetric Cloud properties
	JsonObject->SetStringField("CloudMaterial", VolumetricCloudComponent->Material->GetPathName());

	JsonObject->SetNumberField("LayerBottomAltitude", VolumetricCloudComponent->LayerBottomAltitude);
	JsonObject->SetNumberField("LayerHeight", VolumetricCloudComponent->LayerHeight);
	JsonObject->SetNumberField("TracingStartMaxDistance", VolumetricCloudComponent->TracingStartMaxDistance);
	JsonObject->SetNumberField("TracingMaxDistance", VolumetricCloudComponent->TracingMaxDistance);

	JsonObject->SetBoolField("UsePerSampleAtmosphericLightTransmitannce", VolumetricCloudComponent->bUsePerSampleAtmosphericLightTransmittance);
	JsonObject->SetNumberField("SkyLightCloudBottomOcclusion", VolumetricCloudComponent->SkyLightCloudBottomOcclusion);
	JsonObject->SetNumberField("ViewSampleCountScale", VolumetricCloudComponent->ViewSampleCountScale);
	JsonObject->SetNumberField("ReflectionViewSampleCountScaleValue", VolumetricCloudComponent->ReflectionViewSampleCountScaleValue);
	JsonObject->SetNumberField("ShadowViewSampleCountScale", VolumetricCloudComponent->ShadowViewSampleCountScale);
	JsonObject->SetNumberField("ShadowReflectionViewSampleCountScaleValue", VolumetricCloudComponent->ShadowReflectionViewSampleCountScaleValue);
	JsonObject->SetNumberField("ShadowTracingDistance", VolumetricCloudComponent->ShadowTracingDistance);
	JsonObject->SetNumberField("StopTracingTransmittanceThreshold", VolumetricCloudComponent->StopTracingTransmittanceThreshold);

	// Save Sky Atmosphere properties
	JsonObject->SetNumberField("RayleighScatteringScale", SkyAtmosphereComponent->RayleighScatteringScale);
	JsonObject->SetNumberField("RayleighExponentialDistribution", SkyAtmosphereComponent->RayleighExponentialDistribution);
	JsonObject->SetObjectField("RayleighScattering", ColorToJson(SkyAtmosphereComponent->RayleighScattering));

	JsonObject->SetNumberField("MieScatteringScale", SkyAtmosphereComponent->MieScatteringScale);
	JsonObject->SetObjectField("MieScattering", ColorToJson(SkyAtmosphereComponent->MieScattering));
	JsonObject->SetNumberField("MieAbsorptionScale", SkyAtmosphereComponent->MieAbsorptionScale);
	JsonObject->SetObjectField("MieAbsorption", ColorToJson(SkyAtmosphereComponent->MieAbsorption));
	JsonObject->SetNumberField("MieAnisotropy", SkyAtmosphereComponent->MieAnisotropy);
	JsonObject->SetNumberField("MieExponentialDistribution", SkyAtmosphereComponent->MieExponentialDistribution);

	// Convert to string
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// Save to data asset
	if (WeatherMapDataAsset->WeatherMap.Contains(WeatherPresetName))
	{
		WeatherMapDataAsset->WeatherMap[WeatherPresetName] = OutputString;
	}
	else
	{
		WeatherMapDataAsset->WeatherMap.Add(WeatherPresetName, OutputString);
	}
}

void AAerosimWeather::LoadWeatherPresetData()
{
	LoadWeatherPresetDataByName(WeatherPresetName);
}

void AAerosimWeather::LoadWeatherPresetDataByName(const FString& WeatherPreset)
{
	InitializeReferences();

	if (WeatherMapDataAsset->WeatherMap.Contains(WeatherPreset))
	{
		ApplyLoadedWeatherPreset(WeatherMapDataAsset->WeatherMap[WeatherPreset]);
	}
	else
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("Weather preset not found"));
	}
}

void AAerosimWeather::InitializeReferences()
{
	if (SkyAtmosphereComponent != nullptr && SkyLightComponent != nullptr)
	{
		return;
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACesiumSunSky::StaticClass(), FoundActors);

	if (FoundActors.Num() > 0)
	{
		ACesiumSunSky* SunSky = Cast<ACesiumSunSky>(FoundActors[0]);
		if (SunSky)
		{
			SkyLightComponent = SunSky->SkyLight;
			SkyAtmosphereComponent = SunSky->SkyAtmosphere;
		}
	}
}

void AAerosimWeather::ApplyLoadedWeatherPreset(const FString& JsonString)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		// Load and apply settings
		ExponentialHeightFogComponent->FogDensity = JsonObject->GetNumberField("FogDensity");
		ExponentialHeightFogComponent->SetFogHeightFalloff(JsonObject->GetNumberField("FogHeightFalloff"));
		ExponentialHeightFogComponent->SecondFogData.FogDensity = JsonObject->GetNumberField("SecondFogDensity");
		ExponentialHeightFogComponent->SecondFogData.FogHeightFalloff = JsonObject->GetNumberField("SecondFogHeightFalloff");
		ExponentialHeightFogComponent->SecondFogData.FogHeightOffset = JsonObject->GetNumberField("SecondFogHeightOffset");
		ExponentialHeightFogComponent->FogMaxOpacity = JsonObject->GetNumberField("FogMaxOpacity");
		ExponentialHeightFogComponent->StartDistance = JsonObject->GetNumberField("StartDistance");
		ExponentialHeightFogComponent->FogCutoffDistance = JsonObject->GetNumberField("FogCutoffDistance");

		ExponentialHeightFogComponent->DirectionalInscatteringExponent = JsonObject->GetNumberField("DirectionalInScatteringExponent");
		ExponentialHeightFogComponent->DirectionalInscatteringStartDistance = JsonObject->GetNumberField("DirectionalInScatteringStartDistance");
		ExponentialHeightFogComponent->DirectionalInscatteringLuminance = JsonToColor(JsonObject->GetObjectField("DirectionalInScatteringColor"));

		ExponentialHeightFogComponent->bEnableVolumetricFog = JsonObject->GetBoolField("bVolumetricFog");
		ExponentialHeightFogComponent->VolumetricFogScatteringDistribution = JsonObject->GetNumberField("VolumetricFogScatteringDistribution");
		ExponentialHeightFogComponent->VolumetricFogAlbedo = JsonToColor(JsonObject->GetObjectField("VolumetricFogAlbedo")).ToFColor(false);
		ExponentialHeightFogComponent->VolumetricFogEmissive = JsonToColor(JsonObject->GetObjectField("VolumetricFogEmissive"));
		ExponentialHeightFogComponent->VolumetricFogExtinctionScale = JsonObject->GetNumberField("VolumetricFogExtinctionScale");
		ExponentialHeightFogComponent->VolumetricFogDistance = JsonObject->GetNumberField("VolumetricFogViewDistance");
		ExponentialHeightFogComponent->VolumetricFogNearFadeInDistance = JsonObject->GetNumberField("VolumetricFogNearInDistance");
		ExponentialHeightFogComponent->VolumetricFogStaticLightingScatteringIntensity = JsonObject->GetNumberField("VolumetricFogStaticLightingScatteringIntensity");

		VolumetricCloudComponent->LayerBottomAltitude = JsonObject->GetNumberField("LayerBottomAltitude");
		VolumetricCloudComponent->LayerHeight = JsonObject->GetNumberField("LayerHeight");
		VolumetricCloudComponent->TracingStartMaxDistance = JsonObject->GetNumberField("TracingStartMaxDistance");
		VolumetricCloudComponent->TracingMaxDistance = JsonObject->GetNumberField("TracingMaxDistance");

		FString MaterialPath = JsonObject->GetStringField("CloudMaterial");
		if (MaterialPath == "None")
		{
			VolumetricCloudComponent->SetMaterial(nullptr);
		}
		else
		{
			VolumetricCloudComponent->SetMaterial(Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *JsonObject->GetStringField("CloudMaterial"))));
		}
		VolumetricCloudComponent->bUsePerSampleAtmosphericLightTransmittance = JsonObject->GetBoolField("UsePerSampleAtmosphericLightTransmitannce");
		VolumetricCloudComponent->SkyLightCloudBottomOcclusion = JsonObject->GetNumberField("SkyLightCloudBottomOcclusion");
		VolumetricCloudComponent->ViewSampleCountScale = JsonObject->GetNumberField("ViewSampleCountScale");
		VolumetricCloudComponent->ReflectionViewSampleCountScaleValue = JsonObject->GetNumberField("ReflectionViewSampleCountScaleValue");
		VolumetricCloudComponent->ShadowViewSampleCountScale = JsonObject->GetNumberField("ShadowViewSampleCountScale");
		VolumetricCloudComponent->ShadowReflectionViewSampleCountScaleValue = JsonObject->GetNumberField("ShadowReflectionViewSampleCountScaleValue");
		VolumetricCloudComponent->ShadowTracingDistance = JsonObject->GetNumberField("ShadowTracingDistance");
		VolumetricCloudComponent->StopTracingTransmittanceThreshold = JsonObject->GetNumberField("StopTracingTransmittanceThreshold");

		SkyAtmosphereComponent->RayleighScatteringScale = JsonObject->GetNumberField("RayleighScatteringScale");
		SkyAtmosphereComponent->RayleighExponentialDistribution = JsonObject->GetNumberField("RayleighExponentialDistribution");
		SkyAtmosphereComponent->RayleighScattering = JsonToColor(JsonObject->GetObjectField("RayleighScattering"));

		SkyAtmosphereComponent->MieScatteringScale = JsonObject->GetNumberField("MieScatteringScale");
		SkyAtmosphereComponent->MieScattering = JsonToColor(JsonObject->GetObjectField("MieScattering"));
		SkyAtmosphereComponent->MieAbsorptionScale = JsonObject->GetNumberField("MieAbsorptionScale");
		SkyAtmosphereComponent->MieAbsorption = JsonToColor(JsonObject->GetObjectField("MieAbsorption"));
		SkyAtmosphereComponent->MieAnisotropy = JsonObject->GetNumberField("MieAnisotropy");
		SkyAtmosphereComponent->MieExponentialDistribution = JsonObject->GetNumberField("MieExponentialDistribution");
	}
}

void AAerosimWeather::SetWeatherDataAsset(UWeatherMapDataAsset* WeatherDataAsset)
{
	WeatherMapDataAsset = WeatherDataAsset;
}
