// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Subsystems/ActorRegistry.h"
#include "Game/Subsystems/CommandConsumer.h"
#include "GameFramework/GameModeBase.h"

#include "AerosimGameMode.generated.h"

/**
 *
 */
class UCommandConsumer;
class UActorRegistry;
class UCesiumTileManager;
class ACesiumGeoreference;
class ACesium3DTileset;
class ACesiumCameraManager;
class ACesiumSunSky;
class UAerosimDataTracker;
class AAerosimHUD;
class AAerosimWeather;

UCLASS()
class AEROSIMCONNECTOR_API AAerosimGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	inline APawn* GetSpectatorPawn() { return SpectatorPawn; };
	inline const APawn* GetSpectatorPawn() const { return SpectatorPawn; };

	UPROPERTY()
	AAerosimHUD* AerosimHUD;

	UFUNCTION(BlueprintCallable)
	UActorRegistry* GetActorRegistry() { return ActorRegistry; }

	UFUNCTION(BlueprintCallable)
	UAerosimDataTracker* GetAerosimDataTracker() { return AerosimDataTracker; }

	UFUNCTION(BlueprintCallable)
	AAerosimWeather* GetAerosimWeather() { return AerosimWeather; }

	UFUNCTION(BlueprintCallable)
	UCesiumTileManager* GetCesiumTileManager() { return CesiumTileManager; }
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	FString InstanceID;

	UPROPERTY()
	bool bIsMessageHandlerInitialized = false;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	TSubclassOf<UActorRegistry> RegistryClass;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	UDataAsset* WeatherDataAsset;

	UPROPERTY()
	UActorRegistry* ActorRegistry;

	UPROPERTY()
	UAerosimDataTracker* AerosimDataTracker;

	UPROPERTY()
	UCommandConsumer* CommandConsumer;

	UPROPERTY()
	UCesiumTileManager* CesiumTileManager;

	UPROPERTY()
	APawn* SpectatorPawn;

	UPROPERTY()
	AAerosimWeather* AerosimWeather;

	UPROPERTY(EditDefaultsOnly, Category = "Coords")
	double InitialLat = 33.93651939935984; // LAX

	UPROPERTY(EditDefaultsOnly, Category = "Coords")
	double InitialLon = -118.41269814369221; // LAX

	UPROPERTY(EditDefaultsOnly, Category = "Coords")
	double InitialHeight = 0.0; // LAX

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	int InitialIDForAlreadySpawnedActors = 7000;
};
