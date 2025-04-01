#include "Game/Subsystems/CommandConsumer.h"

#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/GameModeBase.h"

#include "Game/Subsystems/ActorRegistry.h"
#include "Util/MessageHandler.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#include "Cesium3DTileset.h"
#include "Actors/AerosimActor.h"
#include "Game/AerosimGameMode.h"
#include "Game/Subsystems/CesiumTileManager.h"
#include "HUD/PFDWidget.h"
#include "Misc/Variant.h"
#include "Game/Subsystems/PayloadProcessor.h"
#include "Game/Subsystems/AerosimDataTracker.h"
#include "Actors/CameraSensor.h"
#include "Weather/AerosimWeather.h"

static void PrintJsonObject(const TSharedPtr<FJsonObject>& JsonObject)
{
	if (!JsonObject.IsValid())
	{
		UE_LOG(LogCore, Warning, TEXT("Invalid FJsonObject"));
		return;
	}

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	if (FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer))
	{
		UE_LOG(LogCore, Log, TEXT("PrintJsonObject"));
		UE_LOG(LogCore, Log, TEXT("%s"), *JsonString);
	}
	else
	{
		UE_LOG(LogCore, Error, TEXT("Failed to read FJsonObject"));
	}
}

void UCommandConsumer::SetActorRegistry(UActorRegistry* ActorRegistry)
{
	Registry = ActorRegistry;
}

void UCommandConsumer::SetCesiumTileManager(UCesiumTileManager* NewCesiumTileManager)
{
	CesiumTileManager = NewCesiumTileManager;
}

void UCommandConsumer::UpdateSceneFromSceneGraph(const FString& SceneGraphJson)
{
	if (bFirstTime)
	{
		notify_scene_graph_loaded();
		bFirstTime = false;
	}

	UE_LOG(LogCore, Verbose, TEXT("UpdateSceneFromSceneGraph: %s"), *SceneGraphJson);

	FSceneGraph CurrentSceneGraph;
	UPayloadProcessor::ParseJson(SceneGraphJson, CurrentSceneGraph);

	SpawnActorsIfNeeded(CurrentSceneGraph);
	UpdateResourcesFromSceneGraph(CurrentSceneGraph);
	UpdateActorTransformsFromSceneGraph(CurrentSceneGraph);
	UpdateEffectorsFromSceneGraph(CurrentSceneGraph);
	UpdatePFDsFromSceneGraph(CurrentSceneGraph);
	UpdateTrajectoryVisualizationSettingsFromSceneGraph(CurrentSceneGraph);
	UpdateTrajectoryVisualizationUserDefinedWaypointsFromSceneGraph(CurrentSceneGraph);
	UpdateTrajectoryVisualizationFutureTrajectoryWaypointsFromSceneGraph(CurrentSceneGraph);
}

void UCommandConsumer::SpawnActorsIfNeeded(FSceneGraph& SceneGraph)
{
	for (auto& Entity : SceneGraph.Entities)
	{
		if (!ActorNameIdMap.Contains(Entity.Key))
		{
			FString ActorType = SceneGraph.Components.ActorProperties[Entity.Key].ActorAsset;
			uint32 NewActorId = Registry->RegisterActorByName(ActorType, FVector(0, 0, 0), FRotator(0, 0, 0));
			ActorNameIdMap.Add(Entity.Key, NewActorId);

			AActor* SpawnedActor = Registry->GetActor(NewActorId);
			if (!IsValid(SpawnedActor))
			{
				UE_LOG(LogCore, Error, TEXT("Failed to spawn actor"));
				continue;
			}

			if (ActorNameIdMap.Contains(SceneGraph.Components.ActorProperties[Entity.Key].Parent))
			{
				SpawnedActor->AttachToActor(Registry->GetActor(ActorNameIdMap[SceneGraph.Components.ActorProperties[Entity.Key].Parent]), FAttachmentTransformRules::KeepWorldTransform);
			}

			// Process sensor actors
			bool bIsSensor = false;
			if (ActorType == "sensors/cameras/rgb_camera")
			{
				bIsSensor = true;
				ACameraSensor* CameraSensor = Cast<ACameraSensor>(SpawnedActor);
				bool bCaptureEnabled = SceneGraph.Components.Sensors[Entity.Key].bCaptureEnabled;
				CameraSensor->SetCaptureEnabled(bCaptureEnabled);
				FVector2D Resolution = SceneGraph.Components.Sensors[Entity.Key].Resolution;
				CameraSensor->GetRenderTarget()->InitCustomFormat(Resolution.X, Resolution.Y, EPixelFormat::PF_B8G8R8A8, true);
				ECameraProjectionMode::Type ProjectionMode = SceneGraph.Components.Sensors[Entity.Key].ProjectionMode;
				CameraSensor->GetSceneCaptureActor()->GetCaptureComponent2D()->ProjectionType = ProjectionMode;
				float OrthoWidth = SceneGraph.Components.Sensors[Entity.Key].OrthoWidth;
				CameraSensor->GetSceneCaptureActor()->GetCaptureComponent2D()->OrthoWidth = OrthoWidth;
				float FOV = SceneGraph.Components.Sensors[Entity.Key].FOV;
				CameraSensor->GetSceneCaptureActor()->GetCaptureComponent2D()->FOVAngle = FOV;
				float TickRate = SceneGraph.Components.Sensors[Entity.Key].TickRate;
				CameraSensor->SetActorTickInterval(TickRate);
			}
			else if (ActorType == "sensors/depth_sensor")
			{
				bIsSensor = true;
				// TODO
			}

			AAerosimActor* AerosimActor = Cast<AAerosimActor>(SpawnedActor);
			if (IsValid(AerosimActor))
			{
				AerosimActor->ActorInstanceId = NewActorId;

				if (!bIsSensor)
				{
					// Set up PFD flight display widgets for non-sensor actors
					AerosimActor->SetWidgetID(NewActorId);

					UAerosimDataTracker* DataTracker = GameMode->GetAerosimDataTracker();
					if (DataTracker)
					{
						DataTracker->Airspeeds.Add(NewActorId, 0);
						DataTracker->TrueAirspeeds.Add(NewActorId, 0);
						DataTracker->Altitudes.Add(NewActorId, 0);
						DataTracker->TargetAltitudes.Add(NewActorId, 0);
						DataTracker->Pressures.Add(NewActorId, 0);
						DataTracker->VerticalSpeeds.Add(NewActorId, 0);
						DataTracker->Pitchs.Add(NewActorId, 0);
						DataTracker->BankAngles.Add(NewActorId, 0);
						DataTracker->Slips.Add(NewActorId, 0);
						DataTracker->Headings.Add(NewActorId, 0);
						DataTracker->NeedleHeadings.Add(NewActorId, 0);
						DataTracker->Deviations.Add(NewActorId, 0);
						DataTracker->NeedleModes.Add(NewActorId, 0);
					}
				}
			}
			else
			{
				UE_LOG(LogCore, Error, TEXT("Failed to cast spawned actor to AAerosimActor"));
			}
		}
	}
}

void UCommandConsumer::UpdateResourcesFromSceneGraph(const FSceneGraph& SceneGraph)
{
	if (SceneGraph.Resources.bResourcesSet)
	{
		if (CachedOrigin != SceneGraph.Resources.Origin)
		{
			CachedOrigin = SceneGraph.Resources.Origin;
			CesiumTileManager->LoadTileSet(CachedOrigin.X, CachedOrigin.Y, CachedOrigin.Z);
		}

		if (SceneGraph.Resources.Weather.Preset.Len() > 0)
		{
			if (IsValid(GameMode))
			{
				AAerosimWeather* AerosimWeather = GameMode->GetAerosimWeather();
				if (IsValid(AerosimWeather))
				{
					AerosimWeather->LoadWeatherPresetDataByName(SceneGraph.Resources.Weather.Preset);
				}
			}
		}

		if (SceneGraph.Resources.ViewportConfig.ActiveViewport.Len() > 0)
		{
			AAerosimGameMode* AerosimGameMode = Cast<AAerosimGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
			if (IsValid(AerosimGameMode))
			{
				APawn* SpectatorPawn = AerosimGameMode->GetSpectatorPawn();
				if (!IsValid(SpectatorPawn))
				{
					UE_LOG(LogCore, Warning, TEXT("[UpdateResourcesFromSceneGraph] Invalid SpectatorPawn."));
					return;
				}
				FString Entity = SceneGraph.Resources.ViewportConfig.ActiveViewport;
				if (ActorNameIdMap.Contains(Entity))
				{
					AAerosimActor* Actor = Cast<AAerosimActor>(Registry->GetActor(ActorNameIdMap[Entity]));
					if (!IsValid(Actor))
					{
						UE_LOG(LogCore, Warning, TEXT("[UpdateResourcesFromSceneGraph] AerosimActor '%s' not found in registered actors."), *Entity);
						return;
					}

					if (SpectatorPawn->GetParentActor() != Actor)
					{
						SpectatorPawn->AttachToActor(Actor, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
						SpectatorPawn->SetActorRelativeLocation(FVector(0, 0, 0));
						SpectatorPawn->SetActorRelativeRotation(FRotator(0, 0, 0));
					}
				}
			}
		}
	}
}

void UCommandConsumer::UpdateActorTransformsFromSceneGraph(const FSceneGraph& SceneGraph)
{
	for (auto& States : SceneGraph.Components.ActorStates)
	{
		if (ActorNameIdMap.Contains(States.Key))
		{
			FActorState ActorState = States.Value;
			FTransformSceneGraph Transform = ActorState.Pose;

			// Convert pose into Unreal+Cesium ESU coordinates
			// TODO Add "frame_id" to Pose to determine if it's a global world frame pose or not,
			//   instead of checking the entity's parent tag.
			// TODO This assumes unparented entities have parent="" and not "world" or some other root parent tag
			if (SceneGraph.Components.ActorProperties[States.Key].Parent.Len() > 0)
			{
				// Convert relative position from FRD m to FRU cm
				// TODO Make a new conversion function in world-link lib?
				Transform.Position.X *= 100.0;
				Transform.Position.Y *= 100.0;
				Transform.Position.Z *= -100.0; // Down to Up

				// Convert relative rotation from FRD RPY rad to Unreal RPY deg
				// TODO Make a new conversion function in world-link lib?
				Transform.Rotation.Roll = FMath::RadiansToDegrees(Transform.Rotation.Roll);
				Transform.Rotation.Pitch = FMath::RadiansToDegrees(Transform.Rotation.Pitch);
				Transform.Rotation.Yaw = FMath::RadiansToDegrees(Transform.Rotation.Yaw);
			}
			else
			{
				// Convert global pose from NED to Unreal ESU
				ned_to_unreal_esu(&Transform.Position.X, &Transform.Position.Y, &Transform.Position.Z);
				rpy_ned_to_unreal_esu(&Transform.Rotation.Roll, &Transform.Rotation.Pitch, &Transform.Rotation.Yaw);
			}

			Registry->GetActor(ActorNameIdMap[States.Key])->SetActorRelativeTransform(FTransform(Transform.Rotation, Transform.Position, Transform.Scale));
			AActor* Actor = Registry->GetActor(ActorNameIdMap[States.Key]);
			if (IsValid(Actor))
			{
				Actor->SetActorRelativeTransform(FTransform(Transform.Rotation, Transform.Position, Transform.Scale));
				AAerosimActor* AerosimActor = Cast<AAerosimActor>(Actor);
				if (IsValid(AerosimActor))
				{
					// TODO: handle skipping updates in the actor's trajectory visualizer and expose this as an update interval parameter in the trajectory settings instead of throwing away command messages
					static constexpr unsigned int SKIP_MESSAGES = 10;
					static unsigned int counter = SKIP_MESSAGES;
					if (counter == SKIP_MESSAGES)
					{
						AerosimActor->UpdateTrajectoryVisualizer(Transform.Position);
						counter = 0;
					}
					else
					{
						counter++;
					}
				}
			}
		}
	}
}

void UCommandConsumer::UpdateEffectorsFromSceneGraph(FSceneGraph& SceneGraph)
{
	for (auto& Effectors : SceneGraph.Components.Effectors)
	{
		const FString& Entity = Effectors.Key;
		if (ActorNameIdMap.Contains(Entity))
		{
			AAerosimActor* Actor = Cast<AAerosimActor>(Registry->GetActor(ActorNameIdMap[Entity]));
			if (!IsValid(Actor))
			{
				UE_LOG(LogCore, Warning, TEXT("[UpdateEffectorsFromSceneGraph] AerosimActor '%s' not found in registered actors."), *Entity);
				continue;
			}

			if (!EffectorInitTransformMap.Contains(Entity))
			{
				EffectorInitTransformMap.Add(Entity, FEffectorInitialTransforms());
			}
			FEffectorInitialTransforms& EffectorInitTransforms = EffectorInitTransformMap[Entity];

			for (auto& Effector : Effectors.Value.Effectors)
			{
				TArray<FString> EffectorPathArray;
				Effector.USDPath.ParseIntoArray(EffectorPathArray, TEXT("/"), true);
				const FString& EffectorComponentName = EffectorPathArray.Last();
				UStaticMeshComponent* SMComponent = nullptr;
				for (UActorComponent* Component : Actor->GetComponents())
				{
					if (Component && Component->GetFName() == EffectorComponentName)
					{
						SMComponent = Cast<UStaticMeshComponent>(Component);
						break;
					}
				}

				if (IsValid(SMComponent))
				{
					FTransformSceneGraph InitialTransform;
					if (!EffectorInitTransforms.EffectorInitialTransformMap.Contains(EffectorComponentName))
					{
						InitialTransform.Position = SMComponent->GetRelativeLocation();
						InitialTransform.Rotation = SMComponent->GetRelativeRotation();
						InitialTransform.Scale = SMComponent->GetRelativeScale3D();
						EffectorInitTransforms.EffectorInitialTransformMap.Add(EffectorComponentName, InitialTransform);
					}
					else
					{
						InitialTransform = EffectorInitTransforms.EffectorInitialTransformMap[EffectorComponentName];
					}

					// Convert relative position from FRD m to FRU cm
					// TODO Make a new conversion function in world-link lib?
					Effector.Transform.Position.X *= 100.0;
					Effector.Transform.Position.Y *= 100.0;
					Effector.Transform.Position.Z *= -100.0; // Down to Up

					// Convert relative rotation from FRD RPY rad to Unreal RPY deg
					// TODO Make a new conversion function in world-link lib?
					Effector.Transform.Rotation.Roll = FMath::RadiansToDegrees(Effector.Transform.Rotation.Roll);
					Effector.Transform.Rotation.Pitch = FMath::RadiansToDegrees(Effector.Transform.Rotation.Pitch);
					Effector.Transform.Rotation.Yaw = FMath::RadiansToDegrees(Effector.Transform.Rotation.Yaw);

					FTransform NewRelTransform = FTransform(InitialTransform.Rotation + Effector.Transform.Rotation, InitialTransform.Position + Effector.Transform.Position, InitialTransform.Scale * Effector.Transform.Scale);
					FRotator NewRelRot = NewRelTransform.GetRotation().Rotator();

					SMComponent->SetRelativeTransform(NewRelTransform);
				}
			}
		}
	}
}

void UCommandConsumer::UpdatePFDsFromSceneGraph(const FSceneGraph& SceneGraph)
{
	for (auto& PFDComponents : SceneGraph.Components.PrimaryFlightDisplays)
	{
		const FString& Entity = PFDComponents.Key;
		const FPrimaryFlightDisplayData& PFDStateData = PFDComponents.Value;
		if (ActorNameIdMap.Contains(Entity))
		{
			AAerosimActor* AerosimActor = Cast<AAerosimActor>(Registry->GetActor(ActorNameIdMap[Entity]));
			if (!IsValid(AerosimActor))
			{
				UE_LOG(LogCore, Warning, TEXT("[UpdatePFDsFromSceneGraph] AerosimActor '%s' not found in registered actors."), *Entity);
				continue;
			}

			int PFDId = AerosimActor->ActorInstanceId;
			UAerosimDataTracker* DataTracker = GameMode->GetAerosimDataTracker();
			if (DataTracker)
			{
				// Update (or add) PFD data in the data tracker
				DataTracker->Airspeeds.Add(PFDId, PFDStateData.AirspeedKts);
				DataTracker->TrueAirspeeds.Add(PFDId, PFDStateData.TrueAirspeedKts);
				DataTracker->Altitudes.Add(PFDId, PFDStateData.AltitudeFt);
				DataTracker->TargetAltitudes.Add(PFDId, PFDStateData.TargetAltitudeFt);
				DataTracker->Pressures.Add(PFDId, PFDStateData.AltimeterPressureSettingInHg);
				DataTracker->VerticalSpeeds.Add(PFDId, PFDStateData.VerticalSpeedFpm);
				DataTracker->Pitchs.Add(PFDId, PFDStateData.PitchDeg);
				DataTracker->BankAngles.Add(PFDId, PFDStateData.RollDeg);
				DataTracker->Slips.Add(PFDId, PFDStateData.SideSlipFps2);
				DataTracker->Headings.Add(PFDId, PFDStateData.HeadingDeg);
				DataTracker->NeedleHeadings.Add(PFDId, PFDStateData.HsiCourseSelectHeadingDeg);
				DataTracker->Deviations.Add(PFDId, PFDStateData.HsiCourseDeviationDeg);
				DataTracker->NeedleModes.Add(PFDId, PFDStateData.HsiMode);
			}
		}
	}
}

void UCommandConsumer::UpdateTrajectoryVisualizationSettingsFromSceneGraph(const FSceneGraph& SceneGraph)
{
	for (auto TrajectorySettings : SceneGraph.Components.TrajectoryVisualizationSettings)
	{
		if (ActorNameIdMap.Contains(TrajectorySettings.Key))
		{
			AAerosimActor* Actor = Cast<AAerosimActor>(Registry->GetActor(ActorNameIdMap[TrajectorySettings.Key]));
			if (IsValid(Actor))
			{
				Actor->UpdateTrajectoryVisualizerSettings(TrajectorySettings.Value.DisplayFutureTrajectory, TrajectorySettings.Value.DisplayPastTrajectory, TrajectorySettings.Value.HighlightUserDefinedWaypoints, TrajectorySettings.Value.HighlightUserDefinedWaypoints);
			}
			else
			{
				UE_LOG(LogCore, Error, TEXT("Actor not valid for trajectory visualizer settings"));
			}
		}
		else
		{
			UE_LOG(LogCore, Error, TEXT("Actor not found for trajectory visualizer user defined waypoints"));
		}
	}
}

void UCommandConsumer::UpdateTrajectoryVisualizationUserDefinedWaypointsFromSceneGraph(const FSceneGraph& SceneGraph)
{
	for (auto TrajectoryUserDefinedWaypoints : SceneGraph.Components.TrajectoryVisualizationUserDefinedWaypoints)
	{
		if (ActorNameIdMap.Contains(TrajectoryUserDefinedWaypoints.Key))
		{
			AAerosimActor* Actor = Cast<AAerosimActor>(Registry->GetActor(ActorNameIdMap[TrajectoryUserDefinedWaypoints.Key]));
			if (IsValid(Actor))
			{
				Actor->UpdateTrajectoryVisualizerUserDefinedWaypoints(TrajectoryUserDefinedWaypoints.Value.Waypoints);
			}
			else
			{
				UE_LOG(LogCore, Error, TEXT("Actor not valid for trajectory user defined waypoints"));
			}
		}
		else
		{
			UE_LOG(LogCore, Error, TEXT("Actor not found for trajectory user defined waypoints"));
		}
	}
}

void UCommandConsumer::UpdateTrajectoryVisualizationFutureTrajectoryWaypointsFromSceneGraph(const FSceneGraph& SceneGraph)
{
	for (auto TrajectoryFutureWaypoints : SceneGraph.Components.TrajectoryVisualizationFutureTrajectoryWaypoints)
	{
		if (ActorNameIdMap.Contains(TrajectoryFutureWaypoints.Key))
		{
			AAerosimActor* Actor = Cast<AAerosimActor>(Registry->GetActor(ActorNameIdMap[TrajectoryFutureWaypoints.Key]));
			if (IsValid(Actor))
			{
				Actor->UpdateTrajectoryVisualizerFutureTrajectory(TrajectoryFutureWaypoints.Value.Waypoints);
			}
			else
			{
				UE_LOG(LogCore, Error, TEXT("Actor not valid for trajectory future trajectory waypoints"));
			}
		}
		else
		{
			UE_LOG(LogCore, Error, TEXT("Actor not found for trajectory future trajectory waypoints"));
		}
	}
}

void UCommandConsumer::ProcessCommandsFromQueue(float DeltaSeconds)
{
	if (!IsValid(Registry))
	{
		UE_LOG(LogCore, Warning, TEXT("No Registry Found"));
		return;
	}

	const uint32_t QueueSize = get_consumer_payload_queue_size();
	const uint32_t NumMaxCmdsToProcess = std::min(QueueSize, MAX_MESSAGES_PER_TICK);
	double OldestTimestamp = 0.0;
	double NewestTimestamp = 0.0;
	double ConsumeEndTime = 0.0;

	if (bRealTimePacingMode)
	{
		// Process timing for smoothly pacing out commands over real-time frame rate steps
		const FDateTime Now = FDateTime::Now();
		const FTimespan ElapsedTime = Now - LastTickTime;
		const double ElapsedTimeSec = ElapsedTime.GetTotalSeconds();
		LastTickTime = Now;
		OldestTimestamp = get_consumer_payload_queue_oldest_timestamp();
		NewestTimestamp = get_consumer_payload_queue_newest_timestamp();
		const double StartTimestamp = LastConsumedTimestamp >= 0.0 ? LastConsumedTimestamp : OldestTimestamp;
		ConsumeEndTime = StartTimestamp + ElapsedTimeSec;

		// UE_LOG(LogCore, Warning, TEXT("Start ElapsedTimeSec=%f, processing up to %d cmds out of queue size: %d, OldestTimestamp=%f, NewestTimestamp=%f, ConsumeEndTime=%f"),
		// 	ElapsedTimeSec, NumMaxCmdsToProcess, QueueSize, OldestTimestamp, NewestTimestamp, ConsumeEndTime);
	}

	// Start processing render commands for this tick step
	for (uint32_t Idx = 0; Idx < NumMaxCmdsToProcess; Idx++)
	{
		double PayloadTimestamp = get_consumer_payload_queue_oldest_timestamp();
		char* Message = get_consumer_payload_from_queue();
		if (Message == nullptr)
		{
			// UE_LOG(LogCore, VeryVerbose, TEXT("no commands in queue"));
			break;
		}

		// Process this payload's render command
		FString Payload(UTF8_TO_TCHAR(Message));
		LastConsumedTimestamp = PayloadTimestamp;

		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Payload);

		if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
		{
			UE_LOG(LogCore, Warning, TEXT("Failed to parse JSON"));
			return;
		}

		FString JsonString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
		// UE_LOG(LogCore, Log, TEXT("Parsed JSON: %s"), *JsonString);

		const TArray<TSharedPtr<FJsonValue>>* CommandVec;
		if (JsonObject->TryGetArrayField(TEXT("commands"), CommandVec))
		{
			for (const TSharedPtr<FJsonValue>& CommandValue : *CommandVec)
			{
				const TSharedPtr<FJsonObject> CurJsonObject = CommandValue->AsObject();
				const FString CommandType = CurJsonObject->GetStringField(TEXT("command_type"));

				if (CommandType == TEXT("configure_scene"))
				{
					ConfigureSceneCommand(CurJsonObject);
				}
				else if (CommandType == TEXT("spawn_actor"))
				{
					SpawnActorCommand(CurJsonObject);
				}
				else if (CommandType == TEXT("spawn_actor_by_name"))
				{
					SpawnActorByNameCommand(CurJsonObject);
				}
				else if (CommandType == TEXT("transform_actor"))
				{
					SetActorTransformCommand(CurJsonObject);
				}
				else if (CommandType == TEXT("delete_actor"))
				{
					DeleteActorCommand(CurJsonObject);
				}
				else if (CommandType == TEXT("spawn_sensor"))
				{
					SpawnSensorCommand(CurJsonObject);
				}
				else if (CommandType == TEXT("transform_sensor"))
				{
					TransformSensorCommand(CurJsonObject);
				}
				else if (CommandType == TEXT("delete_sensor"))
				{
					DeleteSensorCommand(CurJsonObject);
				}
				else if (CommandType == TEXT("attach_sensor"))
				{
					AttachSensorCommand(CurJsonObject);
				}
				else if (CommandType == TEXT("possess_sensor"))
				{
					PossessSensorCommand(CurJsonObject);
				}
				else if (CommandType == TEXT("load_coordinates"))
				{
					LoadCoordinatesCommand(CurJsonObject);
				}
				else if (CommandType == TEXT("select_socket"))
				{
					AttachSpectatorToSocketCommand(CurJsonObject);
				}
				else if (CommandType == TEXT("load_usd"))
				{
					LoadUSDInActorCommand(CurJsonObject);
				}
				else if (CommandType == TEXT("attach_actor_to_actor_with_socket"))
				{
					AttachActorToActorWithSocketCommand(CurJsonObject);
				}
				else if (CommandType == TEXT("measure_altitude_offset"))
				{
					MeasureAltitudeOffsetCommand(CurJsonObject);
				}
				else if (CommandType == TEXT("visualize_trajectory"))
				{
					VisualizeTrajectoryCommand(CurJsonObject);
				}
				else if (CommandType == TEXT("visualize_trajectory_settings"))
				{
					VisualizeTrajectorySettingsCommand(CurJsonObject);
				}
				else if (CommandType == TEXT("visualize_trajectory_set_user_defined_waypoints"))
				{
					SetUserDefinedWaypointsCommand(CurJsonObject);
				}
				else if (CommandType == TEXT("visualize_trajectory_add_ahead_waypoints"))
				{
					VisualizeTrajectoryAddAheadTrajectory(CurJsonObject);
				}
				else if (CommandType == TEXT("load_weather_preset"))
				{
					LoadWeatherPreset(CurJsonObject);
				}
				else if (CommandType == TEXT("set_cesium_enabled"))
				{
					SetCesiumEnabled(CurJsonObject);
				}
				else if (CommandType == TEXT("set_cesium_weather_enabled"))
				{
					SetCesiumWeatherEnabled(CurJsonObject);
				}
				else
				{
					UE_LOG(LogCore, Warning, TEXT("Unknown command_type: %s"), *CommandType);
				}
			}
		}

		UpdateSceneFromSceneGraph(Payload);

		// TODO Should this message pointer be passed back to the rust module for freeing?
		std::free(Message);

		if (bRealTimePacingMode)
		{
			// Check updated queue stats after processing each command
			NewestTimestamp = get_consumer_payload_queue_newest_timestamp();
			OldestTimestamp = get_consumer_payload_queue_oldest_timestamp();
			const double QueueDuration = NewestTimestamp - OldestTimestamp;
			// UE_LOG(LogCore, Warning, TEXT("Processed command, updated OldestTimestamp=%f, NewestTimestamp=%f, QueueDuration=%f"), OldestTimestamp, NewestTimestamp, QueueDuration);

			if (OldestTimestamp > ConsumeEndTime && QueueDuration < COMMAND_BUFFER_MAX_SEC)
			{
				// UE_LOG(LogCore, Warning, TEXT("Stop processing commands to leave buffer in queue"));
				break;
			}
		}
	}
}

void UCommandConsumer::ConfigureSceneCommand(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));
	bRealTimePacingMode = ParametersObject->GetBoolField(TEXT("enable_realtime_pacing"));
	// TODO Should this take an entire sim config JSON to set up everything for the scene
	// like enabling/disabling Cesium stuff for GIS vs synthetic scenes, spawning objects, etc?
}

void UCommandConsumer::SpawnActorCommand(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));

	uint32 InstanceId = ParametersObject->GetIntegerField(TEXT("instance_id"));
	uint32 ActorTypeId = ParametersObject->GetIntegerField(TEXT("actor_type_id"));

	float PosX = ParametersObject->GetNumberField(TEXT("pos_x"));
	float PosY = ParametersObject->GetNumberField(TEXT("pos_y"));
	float PosZ = ParametersObject->GetNumberField(TEXT("pos_z"));
	FVector Position(PosX, PosY, PosZ);

	float Roll = ParametersObject->GetNumberField(TEXT("roll"));
	float Pitch = ParametersObject->GetNumberField(TEXT("pitch"));
	float Yaw = ParametersObject->GetNumberField(TEXT("yaw"));
	FRotator Rotation(Pitch, Yaw, Roll);

	UAerosimDataTracker* DataTracker = GameMode->GetAerosimDataTracker();
	if (!IsValid(DataTracker))
	{
		UE_LOG(LogCore, Error, TEXT("DataTracker is not valid"))
	}
	else
	{
		DataTracker->Airspeeds.Add(InstanceId, PosZ); // Here until airspeed is available as testing
		DataTracker->TrueAirspeeds.Add(InstanceId, 0);
		DataTracker->Altitudes.Add(InstanceId, PosZ);
		DataTracker->TargetAltitudes.Add(InstanceId, 0);
		DataTracker->Pressures.Add(InstanceId, 0);
		DataTracker->VerticalSpeeds.Add(InstanceId, PosZ);
		DataTracker->Pitchs.Add(InstanceId, Pitch);
		DataTracker->BankAngles.Add(InstanceId, Roll);
		DataTracker->Slips.Add(InstanceId, 0);
		DataTracker->Headings.Add(InstanceId, 0);
		DataTracker->NeedleHeadings.Add(InstanceId, 0);
		DataTracker->Deviations.Add(InstanceId, 0);
		DataTracker->NeedleModes.Add(InstanceId, 0);
	}

	// Directly passing parameters from JSON to the function
	bool bActorRegistered = Registry->RegisterActorById(InstanceId, ActorTypeId, Position, Rotation);
	if (!bActorRegistered)
	{
		UE_LOG(LogCore, Warning, TEXT("Couldn't spawn actor type ID %d because registering actor in instance ID %d was unsuccessful."), ActorTypeId, InstanceId);
		return;
	}

	FString USDPath = ParametersObject->GetStringField(TEXT("usd_path"));
	AAerosimActor* AerosimActor = Cast<AAerosimActor>(Registry->GetActor(InstanceId));
	if (!IsValid(AerosimActor))
		return;

	AerosimActor->ActorInstanceId = InstanceId;
	AerosimActor->SetWidgetID(InstanceId);

	AerosimActor->SetRootLayer(USDPath);
	UE_LOG(LogCore, Warning, TEXT("Spawn actor command processed: ActorID: %d, TypeID: %d"), InstanceId, ActorTypeId);
	UE_LOG(LogCore, Warning, TEXT("Load USD command processed: ActorID: %d USD Path %s"), InstanceId, *USDPath);
}

void UCommandConsumer::SpawnActorByNameCommand(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));

	uint32 InstanceId = ParametersObject->GetIntegerField(TEXT("instance_id"));
	FString ActorTypeName = ParametersObject->GetStringField(TEXT("actor_type_name"));

	float PosX = ParametersObject->GetNumberField(TEXT("pos_x"));
	float PosY = ParametersObject->GetNumberField(TEXT("pos_y"));
	float PosZ = ParametersObject->GetNumberField(TEXT("pos_z"));
	FVector Position(PosX, PosY, PosZ);

	float Roll = ParametersObject->GetNumberField(TEXT("roll"));
	float Pitch = ParametersObject->GetNumberField(TEXT("pitch"));
	float Yaw = ParametersObject->GetNumberField(TEXT("yaw"));
	FRotator Rotation(Pitch, Yaw, Roll);

	UAerosimDataTracker* DataTracker = GameMode->GetAerosimDataTracker();
	if (!IsValid(DataTracker))
	{
		UE_LOG(LogCore, Error, TEXT("DataTracker is not valid"))
	}
	else
	{
		DataTracker->Airspeeds.Add(InstanceId, PosZ); // Here until airspeed is available as testing
		DataTracker->TrueAirspeeds.Add(InstanceId, 0);
		DataTracker->Altitudes.Add(InstanceId, PosZ);
		DataTracker->TargetAltitudes.Add(InstanceId, 0);
		DataTracker->Pressures.Add(InstanceId, 0);
		DataTracker->VerticalSpeeds.Add(InstanceId, PosZ);
		DataTracker->Pitchs.Add(InstanceId, Pitch);
		DataTracker->BankAngles.Add(InstanceId, Roll);
		DataTracker->Slips.Add(InstanceId, 0);
		DataTracker->Headings.Add(InstanceId, 0);
		DataTracker->NeedleHeadings.Add(InstanceId, 0);
		DataTracker->Deviations.Add(InstanceId, 0);
		DataTracker->NeedleModes.Add(InstanceId, 0);
	}

	// Directly passing parameters from JSON to the function
	bool bActorRegistered = Registry->RegisterActorByName(InstanceId, ActorTypeName, Position, Rotation);
	if (!bActorRegistered)
	{
		UE_LOG(LogCore, Warning, TEXT("Couldn't spawn actor '%s' because registering actor in instance ID %d was unsuccessful."), *ActorTypeName, InstanceId);
		return;
	}

	FString USDPath = ParametersObject->GetStringField(TEXT("usd_path"));
	AAerosimActor* AerosimActor = Cast<AAerosimActor>(Registry->GetActor(InstanceId));
	if (!IsValid(AerosimActor))
		return;

	AerosimActor->ActorInstanceId = InstanceId;
	AerosimActor->SetWidgetID(InstanceId);

	AerosimActor->SetRootLayer(USDPath);
	UE_LOG(LogCore, Log, TEXT("Spawn actor by name command processed: ActorID: %d, TypeName: %s"), InstanceId, *ActorTypeName);
}

void UCommandConsumer::SetActorTransformCommand(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));

	uint32 InstanceId = ParametersObject->GetIntegerField(TEXT("instance_id"));

	float PosX = ParametersObject->GetNumberField(TEXT("pos_x"));
	float PosY = ParametersObject->GetNumberField(TEXT("pos_y"));
	float PosZ = ParametersObject->GetNumberField(TEXT("pos_z"));
	FVector Position(PosX, PosY, PosZ);

	float Roll = ParametersObject->GetNumberField(TEXT("roll"));
	float Pitch = ParametersObject->GetNumberField(TEXT("pitch"));
	float Yaw = ParametersObject->GetNumberField(TEXT("yaw"));
	FRotator Rotation(Pitch, Yaw, Roll);

	SetActorTransformCommand(InstanceId, Position, Rotation);
}

void UCommandConsumer::SetActorTransformCommand(uint32 InstanceId, FVector Translation, FRotator Rotation)
{
	AActor* Actor = Registry->GetActor(InstanceId);
	if (IsValid(Actor))
	{
		Actor->SetActorLocation(Translation);
		Actor->SetActorRotation(Rotation);
	}
	else
	{
		UE_LOG(LogCore, Error, TEXT("Actor not found for transform request"));
	}

	TVariant<FVector, int, float> VariantValue;
	VariantValue.Emplace<float>(Translation.Z);

	// Pass the TVariant to UpdateDataTracker
	UAerosimDataTracker* DataTracker = GameMode->GetAerosimDataTracker();
	if (!IsValid(DataTracker))
	{
		UE_LOG(LogCore, Error, TEXT("DataTracker is not valid"))
	}
	else
	{
		DataTracker->UpdateDataTracker(EDataTrackerType::Airspeed, InstanceId, VariantValue);
		DataTracker->Airspeeds[InstanceId] = Translation.Z; // Here until airspeed is available as testing
		DataTracker->Altitudes[InstanceId] = Translation.Z;
		DataTracker->Airspeeds[InstanceId] = Translation.Z; // Here until airspeed is available as testing
		DataTracker->TrueAirspeeds[InstanceId] = 0;

		VariantValue.Emplace<float>(Translation.Z);
		DataTracker->UpdateDataTracker(EDataTrackerType::Altitude, InstanceId, VariantValue);
		DataTracker->Altitudes[InstanceId] = Translation.Z;
		DataTracker->TargetAltitudes[InstanceId] = 0;
		DataTracker->Pressures[InstanceId] = 0;

		DataTracker->VerticalSpeeds[InstanceId] = Translation.Z; // Here until vertical speed is available as testing

		VariantValue.Emplace<float>(Rotation.Pitch);
		DataTracker->Pitchs[InstanceId] = Rotation.Pitch;
		DataTracker->UpdateDataTracker(EDataTrackerType::Pitch, InstanceId, VariantValue);

		VariantValue.Emplace<float>(Rotation.Roll);
		DataTracker->BankAngles[InstanceId] = Rotation.Roll;
		DataTracker->UpdateDataTracker(EDataTrackerType::BankAngle, InstanceId, VariantValue);

		DataTracker->Slips[InstanceId] = 0;
		DataTracker->Headings[InstanceId] = 0;
		DataTracker->NeedleHeadings[InstanceId] = 0;
		DataTracker->Deviations[InstanceId] = 0;

		// VariantValue.Emplace<int>(0);
		// DataTracker->UpdateDataTracker(EDataTrackerType::Altitude, InstanceId, VariantValue);
		// DataTracker->NeedleModes[InstanceId] = 0;
	}
}

void UCommandConsumer::DeleteActorCommand(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));

	uint32 InstanceId = ParametersObject->GetIntegerField(TEXT("instance_id"));
	// Directly passing parameters from JSON to the function
	Registry->RemoveActor(InstanceId);
	UE_LOG(LogCore, Log, TEXT("Delete actor command processed: ActorID: %d"), InstanceId);
}

void UCommandConsumer::SpawnSensorCommand(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));
	uint32 InstanceId = ParametersObject->GetIntegerField(TEXT("instance_id"));
	FString SensorName = ParametersObject->GetStringField(TEXT("sensor_name"));
	FString SensorType = ParametersObject->GetStringField(TEXT("sensor_type"));
	float PosX = ParametersObject->GetNumberField(TEXT("pos_x"));
	float PosY = ParametersObject->GetNumberField(TEXT("pos_y"));
	float PosZ = ParametersObject->GetNumberField(TEXT("pos_z"));
	FVector Position(PosX, PosY, PosZ);
	float Roll = ParametersObject->GetNumberField(TEXT("roll"));
	float Pitch = ParametersObject->GetNumberField(TEXT("pitch"));
	float Yaw = ParametersObject->GetNumberField(TEXT("yaw"));
	FRotator Rotation(Pitch, Yaw, Roll);

	// Spawn the sensor actor
	Registry->RegisterActorByName(InstanceId, SensorType, Position, Rotation);
	UE_LOG(LogCore, Log, TEXT("Spawn sensor command processed: InstanceID: %d, SensorName: %s"), InstanceId, *SensorName);

	if (SensorType == "sensors/cameras/rgb_camera")
	{
		ACameraSensor* CameraActor = Cast<ACameraSensor>(Registry->GetActor(InstanceId));
		if (IsValid(CameraActor))
		{
			bool bCaptureEnabled = ParametersObject->GetBoolField(TEXT("capture_enabled"));
			CameraActor->SetCaptureEnabled(bCaptureEnabled);
		}
	}

	// Set Spectator to the sensor's location
	AAerosimGameMode* AerosimGameMode = Cast<AAerosimGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!IsValid(AerosimGameMode))
	{
		UE_LOG(LogCore, Error, TEXT("No AerosimGameMode Found"));
		return;
	}

	UE_LOG(LogCore, Log, TEXT("Spectator attached to sensor: ActorID: %d"), InstanceId);
}

void UCommandConsumer::TransformSensorCommand(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));
	uint32 InstanceId = ParametersObject->GetIntegerField(TEXT("instance_id"));
	FString SensorName = ParametersObject->GetStringField(TEXT("sensor_name"));
	float PosX = ParametersObject->GetNumberField(TEXT("pos_x"));
	float PosY = ParametersObject->GetNumberField(TEXT("pos_y"));
	float PosZ = ParametersObject->GetNumberField(TEXT("pos_z"));
	FVector Position(PosX, PosY, PosZ);
	float Roll = ParametersObject->GetNumberField(TEXT("roll"));
	float Pitch = ParametersObject->GetNumberField(TEXT("pitch"));
	float Yaw = ParametersObject->GetNumberField(TEXT("yaw"));
	FRotator Rotation(Pitch, Yaw, Roll);
	AActor* Actor = Registry->GetActor(InstanceId);
	if (IsValid(Actor))
	{
		Actor->SetActorLocation(Position);
		Actor->SetActorRotation(Rotation);
		// UE_LOG(LogCore, Warning, TEXT("Transform actor command processed: ActorID: %d"), InstanceId);
	}
	else
	{
		UE_LOG(LogCore, Error, TEXT("Actor not found for transform request"));
	}
}

void UCommandConsumer::DeleteSensorCommand(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));
	uint32 InstanceId = ParametersObject->GetIntegerField(TEXT("instance_id"));
	FString SensorName = ParametersObject->GetStringField(TEXT("sensor_name"));
	// Directly passing parameters from JSON to the function
	Registry->RemoveActor(InstanceId);
	UE_LOG(LogCore, Log, TEXT("Delete sensor command processed: ActorID: %d"), InstanceId);
}

void UCommandConsumer::AttachSensorCommand(TSharedPtr<FJsonObject> JsonObject)
{
	// TODO Implement this
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));
	uint32 SensorInstanceId = ParametersObject->GetIntegerField(TEXT("sensor_instance_id"));
	FString SensorName = ParametersObject->GetStringField(TEXT("sensor_name"));
	uint32 ActorInstanceId = ParametersObject->GetIntegerField(TEXT("actor_instance_id"));
	FString SocketName = ParametersObject->GetStringField(TEXT("socket_name"));
	AActor* Actor = Registry->GetActor(ActorInstanceId);
	if (!IsValid(Actor))
		return;
	AAerosimActor* AerosimActor = Cast<AAerosimActor>(Actor);
	if (!IsValid(AerosimActor))
		return;
	UStaticMeshComponent* StaticMeshComp = AerosimActor->MeshComponent;
	if (!IsValid(StaticMeshComp))
		return;
	if (StaticMeshComp->DoesSocketExist(*SocketName))
	{
		AAerosimGameMode* AerosimGameMode = Cast<AAerosimGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		if (!IsValid(AerosimGameMode))
		{
			UE_LOG(LogCore, Error, TEXT("No AerosimGameMode Found"));
			return;
		}
		UE_LOG(LogCore, Log, TEXT("Select socket command processed: SensorName: %s, SocketName: %s"), *SensorName, *SocketName);
	}
}

void UCommandConsumer::PossessSensorCommand(TSharedPtr<FJsonObject> JsonObject)
{
	// TODO Implement this
}

void UCommandConsumer::LoadCoordinatesCommand(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));

	float Lat = ParametersObject->GetNumberField(TEXT("lat"));
	float Lon = ParametersObject->GetNumberField(TEXT("lon"));
	float Alt = ParametersObject->GetNumberField(TEXT("alt"));

	// Directly passing parameters from JSON to the function
	CesiumTileManager->LoadTileSet(Lat, Lon, Alt);
	UE_LOG(LogCore, Warning, TEXT("Load coordinates command processed: Lat: %f, Lon: %f, Alt: %f"), Lat, Lon, Alt);
}

void UCommandConsumer::AttachSpectatorToSocketCommand(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));

	uint32 InstanceId = ParametersObject->GetIntegerField(TEXT("instance_id"));
	FString SocketName = ParametersObject->GetStringField(TEXT("socket_name"));

	AActor* Actor = Registry->GetActor(InstanceId);
	if (!IsValid(Actor))
		return;

	AAerosimActor* AerosimActor = Cast<AAerosimActor>(Actor);
	if (!IsValid(AerosimActor))
		return;

	UStaticMeshComponent* StaticMeshComp = AerosimActor->MeshComponent;
	if (!IsValid(StaticMeshComp))
		return;

	if (StaticMeshComp->DoesSocketExist(*SocketName))
	{
		AAerosimGameMode* AerosimGameMode = Cast<AAerosimGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		if (!IsValid(AerosimGameMode))
		{
			UE_LOG(LogCore, Error, TEXT("No AerosimGameMode Found"));
			return;
		}

		UE_LOG(LogCore, Log, TEXT("Select socket command processed: ActorID: %d, SocketName: %s"), InstanceId, *SocketName);
	}
}

void UCommandConsumer::LoadUSDInActorCommand(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));

	uint32 InstanceId = ParametersObject->GetIntegerField(TEXT("instance_id"));
	FString USDPath = ParametersObject->GetStringField(TEXT("usd_path"));

	// Directly passing parameters from JSON to the function
	AAerosimActor* AerosimActor = Cast<AAerosimActor>(Registry->GetActor(InstanceId));
	if (!IsValid(AerosimActor))
		return;

	AerosimActor->SetRootLayer(USDPath);

	UE_LOG(LogCore, Warning, TEXT("Load USD command processed: ActorID: %d USD Path %s"), InstanceId, *USDPath);
}

void UCommandConsumer::AttachActorToActorWithSocketCommand(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));

	uint32 ParentActorId = ParametersObject->GetIntegerField(TEXT("parent_actor_id"));
	uint32 ChildActorId = ParametersObject->GetIntegerField(TEXT("child_actor_id"));
	FString SocketName = ParametersObject->GetStringField(TEXT("socket_name"));

	AActor* ParentActor = Registry->GetActor(ParentActorId);
	if (!IsValid(ParentActor))
		return;

	AActor* ChildActor = Registry->GetActor(ChildActorId);
	if (!IsValid(ChildActor))
		return;

	UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(ParentActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));
	if (IsValid(StaticMeshComp))
	{
		if (StaticMeshComp->DoesSocketExist(*SocketName))
		{
			USceneComponent* RootComponent = ChildActor->GetRootComponent();
			if (!IsValid(RootComponent))
				return;

			RootComponent->AttachToComponent(StaticMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, *SocketName);
			UE_LOG(LogCore, Log, TEXT("Attach actor to actor with socket command processed: ParentActorID: %d, ChildActorID: %d, SocketName: %s"), ParentActorId, ChildActorId, *SocketName);
		}
	}
	else
	{
		bool Result = ChildActor->AttachToActor(ParentActor, FAttachmentTransformRules::SnapToTargetNotIncludingScale, *SocketName);
		UE_LOG(LogCore, Log, TEXT("Attach actor to actor with socket command processed %s: ParentActorID: %d, ChildActorID: %d, SocketName: %s"), Result ? "Successful" : "Failed", ParentActorId, ChildActorId, *SocketName);
	}
}

void UCommandConsumer::MeasureAltitudeOffsetCommand(TSharedPtr<FJsonObject> JsonObject)
{
	FString UUID = JsonObject->GetStringField(TEXT("uuid"));
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));

	double Lat = ParametersObject->GetNumberField(TEXT("lat"));
	double Lon = ParametersObject->GetNumberField(TEXT("lon"));
	double ExpectedAltitude = ParametersObject->GetNumberField(TEXT("expected_hae_in_meters"));

	UE_LOG(LogCore, Log, TEXT("Measure Altitude Offset Command Received"));

	if (!CesiumTileManager)
	{
		UE_LOG(LogCore, Error, TEXT("TileManager is not valid"));
		return;
	}

	// Load Cesium Tileset at the given coordinates
	CurrentDepthSensorOffset = InitialDepthSensorOffset;
	CesiumTileManager->LoadTileSet(Lat, Lon, ExpectedAltitude + CurrentDepthSensorOffset); // Add a known altitude offset of 500m to prevent measuring negative altitudes if the terrain is lower than the expected altitude

	AAerosimGameMode* AerosimGameMode = Cast<AAerosimGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!IsValid(AerosimGameMode))
	{
		UE_LOG(LogCore, Error, TEXT("No AerosimGameMode Found"));
		return;
	}

	APawn* SpectatorPawn = AerosimGameMode->GetSpectatorPawn();
	if (!IsValid(SpectatorPawn))
	{
		UE_LOG(LogCore, Error, TEXT("No SpectatorPawn Found"));
		return;
	}

	USceneComponent* RootComponent = SpectatorPawn->GetRootComponent();
	if (!IsValid(RootComponent))
	{
		UE_LOG(LogCore, Error, TEXT("No RootComponent Found"));
		return;
	}

	// Cache the params for later use
	CachedMeasureAltitudeOffsetCommandParams.UUID = UUID;
	CachedMeasureAltitudeOffsetCommandParams.Lat = Lat;
	CachedMeasureAltitudeOffsetCommandParams.Lon = Lon;
	CachedMeasureAltitudeOffsetCommandParams.ExpectedAltitude = ExpectedAltitude;

	// Detach the spectator pawn from any parent
	SpectatorPawn->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SpectatorPawn->SetActorLocation(FVector(0, 0, 0));
	SpectatorPawn->SetActorRotation(FVector(0, 0, -90).Rotation());
	CesiumTileManager->SpawnAltitudeOffsetSensor();

	// Bind a function to the OnTilesetLoaded delegate
	if (CesiumTileManager->GetCesiumTileset())
	{
		CesiumTileManager->GetCesiumTileset()->OnTilesetLoaded.AddDynamic(this, &UCommandConsumer::OnTilesetLoaded);
		UE_LOG(LogCore, Log, TEXT("OnTilesetLoaded delegate bound"));
	}
}

void UCommandConsumer::OnTilesetLoaded()
{
	UE_LOG(LogCore, Log, TEXT("Tileset loaded"));
	if (!CesiumTileManager)
	{
		UE_LOG(LogCore, Error, TEXT("CesiumTileManager is not valid"));
		return;
	}

	// Unbind the delegate
	if (CesiumTileManager->GetCesiumTileset())
	{
		CesiumTileManager->GetCesiumTileset()->OnTilesetLoaded.RemoveDynamic(this, &UCommandConsumer::OnTilesetLoaded);
		UE_LOG(LogCore, Log, TEXT("OnTilesetLoaded delegate unbound"));
	}

	if (!GetWorld())
	{
		UE_LOG(LogCore, Error, TEXT("World context is invalid. Timer cannot be set."));
		return;
	}

	UE_LOG(LogCore, Log, TEXT("Tileset loaded. Waiting for 5 seconds before adjusting offset"));
	// Delay for 5 seconds before measuring altitude to give the tileset time to load properly
	GetWorld()->GetTimerManager().SetTimer(DepthSensorDelayTimerHandle, this, &UCommandConsumer::AdjustDepthSensorOffset, 5.0f, false);
}

void UCommandConsumer::AdjustDepthSensorOffset()
{
	// Clear the timer handle
	if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(DepthSensorDelayTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(DepthSensorDelayTimerHandle);
		DepthSensorDelayTimerHandle.Invalidate(); // Reset the handle to ensure it's reusable
		UE_LOG(LogCore, Log, TEXT("Timer handle cleared and invalidated"));
	}

	// Query the values from the DepthCaptureActor
	double AltitudeOffset = CesiumTileManager->MeasureAltitudeOffset() / 100.0; // Convert from cm to m
	AltitudeOffset -= CurrentDepthSensorOffset;									// Remove added known altitude offset of 500m to prevent measuring negative altitudes if the terrain is lower than the expected altitude

	// The offset of 20m is an arbitrary value that allows the terrain to switch to the closest LOD to get a more accurate altitude measurement while being far enough to smooth
	// out irregularities in the terrain when average sampling
	if (AltitudeOffset < 0.0f)
	{
		CurrentDepthSensorOffset = abs(AltitudeOffset) + 20.0f;
		CesiumTileManager->SetOriginHeight(CachedMeasureAltitudeOffsetCommandParams.ExpectedAltitude + CurrentDepthSensorOffset);
	}
	else if (AltitudeOffset < 20.0f)
	{
		CurrentDepthSensorOffset = 20.0f;
		CesiumTileManager->SetOriginHeight(CachedMeasureAltitudeOffsetCommandParams.ExpectedAltitude + CurrentDepthSensorOffset);
	}
	else
	{
		CurrentDepthSensorOffset = 0.0f;
		CesiumTileManager->SetOriginHeight(CachedMeasureAltitudeOffsetCommandParams.ExpectedAltitude);
	}

	UE_LOG(LogCore, Log, TEXT("Tileset loaded. Waiting for 5 seconds before taking final offset"));
	// Delay for 5 seconds before measuring altitude to give the tileset time to switch to the LOD level we need.
	GetWorld()->GetTimerManager().SetTimer(DepthSensorDelayTimerHandle, this, &UCommandConsumer::MeasureAltitudeOffsetResponse, 5.0f, false);
}

void UCommandConsumer::MeasureAltitudeOffsetResponse()
{
	// Query the values from the DepthCaptureActor
	double AltitudeOffset = CesiumTileManager->MeasureAltitudeOffset() / 100.0; // Convert from cm to m
	AltitudeOffset -= CurrentDepthSensorOffset;
	CesiumTileManager->DeleteAltitudeOffsetSensor();

	// Create the response payload
	TSharedPtr<FJsonObject> ResponsePayload = MakeShareable(new FJsonObject);
	ResponsePayload->SetStringField(TEXT("uuid"), CachedMeasureAltitudeOffsetCommandParams.UUID);

	TSharedPtr<FJsonObject> ResponseType = MakeShareable(new FJsonObject);
	ResponseType->SetStringField(TEXT("response_type"), TEXT("measure_altitude_offset_response"));

	TSharedPtr<FJsonObject> ResponseParameters = MakeShareable(new FJsonObject);
	ResponseParameters->SetNumberField(TEXT("lat"), CachedMeasureAltitudeOffsetCommandParams.Lat);
	ResponseParameters->SetNumberField(TEXT("lon"), CachedMeasureAltitudeOffsetCommandParams.Lon);
	ResponseParameters->SetNumberField(TEXT("altitude_offset"), AltitudeOffset);

	ResponsePayload->SetObjectField(TEXT("parameters"), ResponseParameters);

	// Serialize the response payload to a string
	FString ResponseString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResponseString);
	FJsonSerializer::Serialize(ResponsePayload.ToSharedRef(), Writer);

	publish_to_topic("aerosim.renderer.responses", TCHAR_TO_UTF8(*ResponseString));
}

void UCommandConsumer::VisualizeTrajectoryCommand(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));
	uint32 InstanceId = ParametersObject->GetIntegerField(TEXT("instance_id"));

	float PosX = ParametersObject->GetNumberField(TEXT("pos_x"));
	float PosY = ParametersObject->GetNumberField(TEXT("pos_y"));
	float PosZ = ParametersObject->GetNumberField(TEXT("pos_z"));

	// Convert from NED to UE5 coordinate system
	FVector Position(PosY * 100.0f, -PosX * 100.0f, -PosZ * 100.0f);

	AAerosimActor* AerosimActor = Cast<AAerosimActor>(Registry->GetActor(InstanceId));
	if (IsValid(AerosimActor))
	{
		// TODO: handle skipping updates in the actor's trajectory visualizer and expose this as an update interval parameter in the trajectory settings instead of throwing away command messages
		static constexpr unsigned int SKIP_MESSAGES = 10;
		static unsigned int counter = SKIP_MESSAGES;
		if (counter == SKIP_MESSAGES)
		{
			AerosimActor->UpdateTrajectoryVisualizer(Position);
			counter = 0;
		}
		else
		{
			counter++;
		}
	}
	else
	{
		UE_LOG(LogCore, Error, TEXT("Actor not found for visualize trajectory request"));
	}
}

void UCommandConsumer::VisualizeTrajectorySettingsCommand(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));
	uint32 InstanceId = ParametersObject->GetIntegerField(TEXT("instance_id"));
	bool bDisplayFutureTrajectory = ParametersObject->GetBoolField(TEXT("show_ahead_waypoints"));
	bool bDisplayPastWaypoints = ParametersObject->GetBoolField(TEXT("show_behind_waypoints"));
	bool bHighlightUserDefinedWaypoints = ParametersObject->GetBoolField(TEXT("show_user_defined_waypoints"));
	uint32 NumberOfFutureWaypoints = ParametersObject->GetIntegerField(TEXT("number_of_waypoints_ahead"));

	AAerosimActor* AerosimActor = Cast<AAerosimActor>(Registry->GetActor(InstanceId));
	if (IsValid(AerosimActor))
	{
		AerosimActor->UpdateTrajectoryVisualizerSettings(bDisplayFutureTrajectory, bDisplayPastWaypoints, bHighlightUserDefinedWaypoints, NumberOfFutureWaypoints);
	}
	else
	{
		UE_LOG(LogCore, Error, TEXT("Actor not found for visualize trajectory request"));
	}
}

void UCommandConsumer::VisualizeTrajectoryAddAheadTrajectory(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));
	uint32 InstanceId = ParametersObject->GetIntegerField(TEXT("instance_id"));

	AAerosimActor* AerosimActor = Cast<AAerosimActor>(Registry->GetActor(InstanceId));
	if (IsValid(AerosimActor))
	{
		const TArray<TSharedPtr<FJsonValue>> Trajectory = ParametersObject->GetArrayField(TEXT("waypoints"));
		TArray<FVector> ParsedTrajectory;
		for (const TSharedPtr<FJsonValue>& Value : Trajectory)
		{
			TArray<TSharedPtr<FJsonValue>> PositionArray = Value->AsArray();
			if (PositionArray.Num() == 3)
			{
				float X = PositionArray[0]->AsNumber();
				float Y = PositionArray[1]->AsNumber();
				float Z = PositionArray[2]->AsNumber();
				// Convert from NED to UE5 coordinate system
				ParsedTrajectory.Add(FVector(Y * 100.0f, -X * 100.0f, -Z * 100.0f));
			}
			else
			{
				UE_LOG(LogCore, Warning, TEXT("The positions must have exactly 3 elements."));
			}
		}
		AerosimActor->UpdateTrajectoryVisualizerFutureTrajectory(ParsedTrajectory);
	}
	else
	{
		UE_LOG(LogCore, Error, TEXT("Actor not found for visualize trajectory request"));
	}
}

void UCommandConsumer::SetUserDefinedWaypointsCommand(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));
	uint32 InstanceId = ParametersObject->GetIntegerField(TEXT("instance_id"));

	AAerosimActor* AerosimActor = Cast<AAerosimActor>(Registry->GetActor(InstanceId));
	if (IsValid(AerosimActor))
	{
		const TArray<TSharedPtr<FJsonValue>> Waypoints = ParametersObject->GetArrayField(TEXT("waypoints"));
		TArray<FVector> ParsedWaypoints;
		for (const TSharedPtr<FJsonValue>& Value : Waypoints)
		{
			TArray<TSharedPtr<FJsonValue>> PositionArray = Value->AsArray();
			if (PositionArray.Num() == 3)
			{
				float X = PositionArray[0]->AsNumber();
				float Y = PositionArray[1]->AsNumber();
				float Z = PositionArray[2]->AsNumber();
				// Convert from NED to UE5 coordinate system
				ParsedWaypoints.Add(FVector(Y * 100.0f, -X * 100.0f, -Z * 100.0f));
			}
			else
			{
				UE_LOG(LogCore, Warning, TEXT("Waypoint array does not have exactly 3 elements."));
			}
		}
		AerosimActor->UpdateTrajectoryVisualizerUserDefinedWaypoints(MoveTemp(ParsedWaypoints));
	}
	else
	{
		UE_LOG(LogCore, Error, TEXT("Actor not found for visualize trajectory request"));
	}
}

void UCommandConsumer::LoadWeatherPreset(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));
	FString PresetID = ParametersObject->GetStringField(TEXT("weather_preset"));
	if (IsValid(GameMode))
	{
		AAerosimWeather* AerosimWeather = GameMode->GetAerosimWeather();
		if (IsValid(AerosimWeather))
		{
			AerosimWeather->LoadWeatherPresetDataByName(PresetID);
		}
	}
}

void UCommandConsumer::SetCesiumEnabled(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));
	bool IsEnabled = ParametersObject->GetBoolField(TEXT("enabled"));
	if (IsValid(GameMode))
	{
		CesiumTileManager->SetCesiumEnabled(IsEnabled);
	}
}

void UCommandConsumer::SetCesiumWeatherEnabled(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> ParametersObject = JsonObject->GetObjectField(TEXT("parameters"));
	bool IsEnabled = ParametersObject->GetBoolField(TEXT("enabled"));
	if (IsValid(GameMode))
	{
		CesiumTileManager->SetCesiumEnabled(IsEnabled);
	}
}
