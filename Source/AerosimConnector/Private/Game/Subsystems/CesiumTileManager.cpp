// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/Subsystems/CesiumTileManager.h"
#include "CesiumGeoreference.h"
#ifdef _WIN32
	#include "windows/WindowsWindow.h"
	#undef OPAQUE
#endif
#include "Cesium3DTileset.h"
#include "CesiumCameraManager.h"
#include "CesiumSunSky.h"
#include "Actors/DepthCaptureSensor.h"
#include "Components/ChildActorComponent.h"

void UCesiumTileManager::BeginPlay()
{
	GeoReference = ACesiumGeoreference::GetDefaultGeoreference(CurrentWorldReference);
	if (!IsValid(GeoReference))
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("Georeference is not detected"));
	}
	CesiumTileset = SpawnCesiumTileset();
	if (!IsValid(CesiumTileset))
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("CesiumTileset is not detected"));
	}
	CesiumSunSky = SpawnCesiumSunSky();
	if (!IsValid(CesiumSunSky))
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("CesiumSunSky is not detected"));
	}

	CesiumTileset->AttachToActor(GeoReference, FAttachmentTransformRules::KeepWorldTransform);
	CesiumTileset->SetTilesetSource(ETilesetSource::FromCesiumIon);
	CesiumTileset->SetIonAssetID(2275207);

	// Patch to prevent flickering tiles from Unreal occlusion culling issue
	// https://github.com/CesiumGS/cesium-unreal/issues/914
	FCustomDepthParameters CustomDepthParameters;
	CustomDepthParameters.RenderCustomDepth = true;
	CesiumTileset->SetCustomDepthParameters(CustomDepthParameters);
}

void UCesiumTileManager::EndPlay()
{
	// Delete default georeference created dynamically
	if (GeoReference != nullptr)
	{
		GeoReference->Destroy();
	}
}
bool UCesiumTileManager::LoadTileSet(double Lat, double Lon, double Height)
{
	UE_LOG(LogAerosimConnector, Warning, TEXT("Tile set to be loaded at %f %f"), Lat, Lon);

	if (!IsValid(GeoReference))
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("GeoReference Is not Valid"));
	}

	// Set Cesium World Origin Coordinates
	SetOriginLatitude(Lat);
	SetOriginLongitude(Lon);
	SetOriginHeight(Height);

	if (IsValid(SpectatorPawn))
	{
		SpectatorPawn->SetActorLocation(DefaultPawnStartLocation);
	}
	else
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("Spectator Pawn is not valid"));
	}

	UE_LOG(LogAerosimConnector, Warning, TEXT("Tile set loaded at %f %f"), Lat, Lon);

	return true;
}

bool UCesiumTileManager::SpawnAltitudeOffsetSensor()
{
	// Spawn a DepthCaptureActor pointing straight down
	FActorSpawnParameters SpawnInfo;
	DepthCaptureActor = CurrentWorldReference->SpawnActor<ADepthCaptureSensor>(
		ADepthCaptureSensor::StaticClass(), FVector(0, 0, 0), FRotator(-90, 0, 0), SpawnInfo);

	if (!IsValid(DepthCaptureActor))
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("Failed to spawn DepthCaptureActor"));
		return false;
	}

	return true;
}

void UCesiumTileManager::DeleteAltitudeOffsetSensor()
{
	// Destroy the DepthCaptureActor
	if (DepthCaptureActor)
		DepthCaptureActor->Destroy();
}

double UCesiumTileManager::MeasureAltitudeOffset()
{
	if (!IsValid(DepthCaptureActor))
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("DepthCaptureActor not in Scene"));
		return 0.0;
	}

	double AltitudeOffset = DepthCaptureActor->SampleHeight(1000);
	return AltitudeOffset;
}

FString UCesiumTileManager::RefactorPath(FString Path)
{
	// we make sure the file is formatted correctly, this let us just copy and past the route.

	// Prefix of the route in case it does not have it
	if (!Path.Contains("file:///"))
	{
		FString PrefixPathString = "file:///";
		Path.InsertAt(0, PrefixPathString);
	}

	FString From = TEXT("\\");
	FString To = "/";
	const TCHAR* Found = *From;
	const TCHAR* Replaced = *To;

	// Replace any backslash to forward slash, since it is what cesium uses when loading a tile from a url
	Path = Path.Replace(Found, Replaced, ESearchCase::Type::IgnoreCase);

	return Path;
}

ACesium3DTileset* UCesiumTileManager::SpawnCesiumTileset()
{
	FActorSpawnParameters SpawnInfo;

	CesiumTileset = CurrentWorldReference->SpawnActor<ACesium3DTileset>(
		ACesium3DTileset::StaticClass(), FVector(0, 0, 0), FRotator::ZeroRotator, SpawnInfo);

	return CesiumTileset;
}

ACesiumGeoreference* UCesiumTileManager::SpawnCesiumGeoReference()
{
	FActorSpawnParameters SpawnInfo;

	GeoReference = CurrentWorldReference->SpawnActor<ACesiumGeoreference>(
		ACesiumGeoreference::StaticClass(), FVector(0, 0, 0), FRotator::ZeroRotator, SpawnInfo);

	return GeoReference;
}

ACesiumCameraManager* UCesiumTileManager::SpawnCesiumCamera()
{
	FActorSpawnParameters SpawnInfo;

	CesiumCameraManager = CurrentWorldReference->SpawnActor<ACesiumCameraManager>(
		ACesiumCameraManager::StaticClass(), FVector(0, 0, 0), FRotator::ZeroRotator, SpawnInfo);

	return CesiumCameraManager;
}

ACesiumSunSky* UCesiumTileManager::SpawnCesiumSunSky()
{
	FActorSpawnParameters SpawnInfo;

	CesiumSunSky = CurrentWorldReference->SpawnActor<ACesiumSunSky>(
		ACesiumSunSky::StaticClass(), FVector(0, 0, 0), FRotator::ZeroRotator, SpawnInfo);

	return CesiumSunSky;
}

// Function to study and make work in PIE
void UCesiumTileManager::PlaceGeoreferenceOriginHere(APawn* PawnToMove, ACesiumGeoreference* GeoRef)
{
	// If this is PIE mode, ignore
	GeoRef->Modify();

	FVector ViewLocation = PawnToMove->GetActorLocation();
	FQuat ViewRotation = PawnToMove->GetActorRotation().Quaternion();

	ViewLocation =
		GeoRef->GetActorTransform().InverseTransformPosition(ViewLocation);
	ViewRotation =
		GeoRef->GetActorTransform().InverseTransformRotation(ViewRotation);

	FRotator NewViewRotation = GeoRef->TransformUnrealRotatorToEastSouthUp(
		ViewRotation.Rotator(),
		ViewLocation);

	// camera local space to ECEF
	FVector CameraEcefPosition =
		GeoRef->TransformUnrealPositionToEarthCenteredEarthFixed(ViewLocation);

	// Long/Lat/Height camera location, in degrees/meters (also our new target
	// georeference origin) When the location is too close to the center of the
	// earth, the result will be (0,0,0)
	GeoRef->SetOriginEarthCenteredEarthFixed(CameraEcefPosition);

	// TODO: check for degeneracy ?
	FVector cameraFront = NewViewRotation.RotateVector(FVector::XAxisVector);
	FVector cameraRight =
		FVector::CrossProduct(FVector::ZAxisVector, cameraFront).GetSafeNormal();
	FVector cameraUp =
		FVector::CrossProduct(cameraFront, cameraRight).GetSafeNormal();

	PawnToMove->SetActorRotation(
		FMatrix(cameraFront, cameraRight, cameraUp, FVector::ZeroVector)
			.Rotator());
	PawnToMove->SetActorLocation(
		GeoRef->GetActorTransform().TransformPosition(FVector::ZeroVector));
}

void UCesiumTileManager::SetOriginLatitude(double Latitude)
{
	GeoReference->SetOriginLatitude(Latitude);
}

void UCesiumTileManager::SetOriginLongitude(double Longitude)
{
	GeoReference->SetOriginLongitude(Longitude);
}

void UCesiumTileManager::SetOriginHeight(double Height)
{
	GeoReference->SetOriginHeight(Height);
}

void UCesiumTileManager::SetCesiumEnabled(bool bEnable)
{
	if(IsValid(CesiumTileset))
	{
		CesiumTileset->SetActorTickEnabled(bEnable);
		CesiumTileset->SetActorHiddenInGame(!bEnable);
		CesiumTileset->SetActorEnableCollision(bEnable);
	}
	else
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("CesiumTileset is not valid"));
	}

	if(IsValid(GeoReference))
	{
		GeoReference->SetActorTickEnabled(bEnable);
		GeoReference->SetActorHiddenInGame(!bEnable);
		GeoReference->SetActorEnableCollision(bEnable);
	}
	else
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("GeoReference is not valid"));
	}

	if(IsValid(CesiumCameraManager))
	{
		CesiumCameraManager->SetActorTickEnabled(bEnable);
		CesiumCameraManager->SetActorHiddenInGame(!bEnable);
		CesiumCameraManager->SetActorEnableCollision(bEnable);
	}
	else
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("CesiumCameraManager is not valid"));
	}
}

void UCesiumTileManager::SetCesiumWeatherEnabled(bool bEnable)
{
	CesiumSunSky->SetActorTickEnabled(bEnable);
	CesiumSunSky->SetActorHiddenInGame(!bEnable);
	CesiumSunSky->SetActorEnableCollision(bEnable);
}
