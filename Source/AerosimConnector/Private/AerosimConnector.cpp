// Copyright Epic Games, Inc. All Rights Reserved.

#include "AerosimConnector.h"
#include "Engine/Engine.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/SavePackage.h"
#include "HAL/FileManager.h"
#include "CesiumIonServer.h"

#define LOCTEXT_NAMESPACE "FAerosimConnectorModule"

void FAerosimConnectorModule::StartupModule()
{
	ModifyCesiumTokenDataAsset();
}

void FAerosimConnectorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FAerosimConnectorModule::ModifyCesiumTokenDataAsset()
{
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	// Ensure the asset registry is fully loaded
	AssetRegistry.SearchAllAssets(true);

	FSoftObjectPath AssetPath = FSoftObjectPath("/Game/CesiumSettings/CesiumIonServers/CesiumIonSaaS.CesiumIonSaaS");
	// Get Asset Data
	FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(AssetPath);
	if (!AssetData.IsValid())
	{
		UE_LOG(LogCore, Warning, TEXT("Failed to find UDataAsset: %s"), *AssetPath.ToString());
		return;
	}

	UObject* Asset = AssetData.GetAsset();
	if (!Asset)
	{
		UE_LOG(LogCore, Warning, TEXT("Failed to load UDataAsset: %s"), *AssetPath.ToString());
		return;
	}

	// Retrieve the Cesium token variable
	FString EnvVarValue = FPlatformMisc::GetEnvironmentVariable(TEXT("AEROSIM_CESIUM_TOKEN"));
	if (EnvVarValue.IsEmpty())
	{
		UE_LOG(LogCore, Warning, TEXT("Environment variable AEROSIM_CESIUM_TOKEN is not set or empty."));
		UE_LOG(LogCore, Log, TEXT("Reading command line for 'CesiumToken' argument."));
		if (!FParse::Value(FCommandLine::Get(), TEXT("CesiumToken="), EnvVarValue))
		{
			UE_LOG(LogCore, Error, TEXT("'CesiumToken' argument not found!"));
		}
	}

	if (UCesiumIonServer* DataAsset = Cast<UCesiumIonServer>(Asset))
	{
		DataAsset->DefaultIonAccessToken = EnvVarValue;
		// Mark as modified
		DataAsset->Modify();
	}
	else
	{
		UE_LOG(LogCore, Warning, TEXT("Failed to cast to UCesiumIonServer: %s"), *AssetPath.ToString());
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAerosimConnectorModule, AerosimConnector)
