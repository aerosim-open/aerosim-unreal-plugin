#include "Game/AerosimGameMode.h"

#include <thread>

#include "Game/Subsystems/ActorRegistry.h"
#include "Game/Subsystems/CommandConsumer.h"
#include "Game/Subsystems/CesiumTileManager.h"

#include "Util/MessageHandler.h"
#include "Misc/Paths.h"
#include <cstring>

#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void AAerosimGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	AerosimHUD = Cast<AAerosimHUD>(UGameplayStatics::GetPlayerController(this, 0)->GetHUD());
	AerosimDataTracker = NewObject<UAerosimDataTracker>();

	if (!IsValid(AerosimDataTracker))
	{
		UE_LOG(LogCore, Error, TEXT("Data Tracker is not valid"));
	}

	if (IsValid(RegistryClass))
	{
		ActorRegistry = NewObject<UActorRegistry>(this, RegistryClass);
		ActorRegistry->SetWorldReference(GetWorld());
		ActorRegistry->RegisterInitialActors(InitialIDForAlreadySpawnedActors);
	}

	CesiumTileManager = NewObject<UCesiumTileManager>(this, UCesiumTileManager::StaticClass());
	if (IsValid(CesiumTileManager))
	{
		SpectatorPawn = GetGameState<AGameStateBase>()->PlayerArray[0]->GetPawn();
		CesiumTileManager->SetSpectatorPawn(SpectatorPawn);
		CesiumTileManager->SetWorldReference(GetWorld());
		CesiumTileManager->BeginPlay();
		CesiumTileManager->LoadTileSet(InitialLat, InitialLon, InitialHeight);
	}

	CommandConsumer = NewObject<UCommandConsumer>(this, UCommandConsumer::StaticClass());
	if (IsValid(CommandConsumer))
	{
		CommandConsumer->SetActorRegistry(ActorRegistry);
		CommandConsumer->SetCesiumTileManager(CesiumTileManager);
		CommandConsumer->GameMode = this;
	}

	if (FParse::Value(FCommandLine::Get(), TEXT("InstanceID="), InstanceID))
	{
		UE_LOG(LogCore, Warning, TEXT("InstanceID: %s"), *InstanceID);
	}
	else
	{
		InstanceID = TEXT("0"); // default instance ID
	}

	AerosimWeather = World->SpawnActor<AAerosimWeather>(AAerosimWeather::StaticClass());
	if (!IsValid(AerosimWeather))
	{
		UE_LOG(LogCore, Error, TEXT("Weather is not valid"));
	}
	else
	{
		AerosimWeather->SetWeatherDataAsset(Cast<UWeatherMapDataAsset>(WeatherDataAsset));
	}

	// // Debug logging of Rust prints to a log file
	// FString LogDirectory = FPaths::ProjectLogDir();
	// FString LogFile = LogDirectory + TEXT("rust_logger_") + InstanceID + TEXT(".log");
	// FTCHARToUTF8 LogFileUtf8(*LogFile);
	// initialize_logger(LogFileUtf8.Get());

	// Initialize the message handler with the renderer instance ID and start the
	// polling thread to be ready to receive orchestrator commands
	bIsMessageHandlerInitialized = initialize_message_handler(TCHAR_TO_UTF8(*InstanceID));

	if (bIsMessageHandlerInitialized)
	{
		start_message_handler();
	}
	else
	{
		UE_LOG(LogCore, Error, TEXT("Failed to initialize message handler."));
	}
}

void AAerosimGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CommandConsumer->ProcessCommandsFromQueue(DeltaSeconds);
}

void AAerosimGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (IsValid(CesiumTileManager))
	{
		CesiumTileManager->EndPlay();
	}

	end_message_handler();
}
