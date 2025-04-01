#include "Game/Subsystems/PayloadProcessor.h"
#include "JsonUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Util/MessageHandler.h"
#include "MathUtil.h"

/* json schema of scene graph for documentation purpouses

{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "properties": {
	"entities": {
	  "type": "object",
	  "patternProperties": {
		"^entity_\\d+$": {
		  "type": "array",
		  "items": { "type": "string" }
		}
	  }
	},
	"resources": {
	  "type": "object",
	  "properties": {
		"origin": {
		  "type": "object",
		  "properties": {
			"latitude": { "type": "number" },
			"longitude": { "type": "number" },
			"altitude": { "type": "number" }
		  },
		  "required": ["latitude", "longitude", "altitude"]
		},
		"sim_time": {
		  "type": "object",
		  "properties": {
			"sec": { "type": "integer" },
			"nsec": { "type": "integer" }
		  },
		  "required": ["sec", "nsec"]
		}
	  },
	  "required": ["origin", "sim_time"]
	},
	"components": {
	  "type": "object",
	  "properties": {
		"actor_properties": {
		  "type": "object",
		  "patternProperties": {
			"^entity_\\d+$": {
			  "type": "object",
			  "properties": {
				"actor_name": { "type": "string" },
				"actor_asset": { "type": "string" },
				"parent": { "type": "string" }
			  },
			  "required": ["actor_name", "actor_asset", "parent"]
			}
		  }
		},
		"actor_state": {
		  "type": "object",
		  "patternProperties": {
			"^entity_\\d+$": {
			  "type": "object",
			  "properties": {
				"pose": {
				  "type": "object",
				  "properties": {
					"transform": {
					  "type": "object",
					  "properties": {
						"position": {
						  "type": "object",
						  "properties": {
							"x": { "type": "number" },
							"y": { "type": "number" },
							"z": { "type": "number" }
						  },
						  "required": ["x", "y", "z"]
						},
						"orientation": {
						  "type": "object",
						  "properties": {
							"x": { "type": "number" },
							"y": { "type": "number" },
							"z": { "type": "number" },
							"w": { "type": "number" }
						  },
						  "required": ["x", "y", "z", "w"]
						},
						"scale": {
						  "type": "object",
						  "properties": {
							"x": { "type": "number" },
							"y": { "type": "number" },
							"z": { "type": "number" }
						  },
						  "required": ["x", "y", "z"]
						}
					  },
					  "required": ["position", "orientation", "scale"]
					}
				  },
				  "required": ["transform"]
				}
			  },
			  "required": ["pose"]
			}
		  }
		},
		"sensor": {
		  "type": "object",
		  "patternProperties": {
			"^entity_\\d+$": {
			  "type": "object",
			  "properties": {
				"sensor_name": { "type": "string" },
				"sensor_type": { "type": "string" },
				"sensor_parameters": {
				  "type": "object",
				  "properties": {
					"resolution": {
					  "type": "object",
					  "properties": {
						"x": { "type": "integer" },
						"y": { "type": "integer" }
					  },
					  "required": ["x", "y"]
					},
					"tick_rate": { "type": "number" },
					"fov": { "type": "number" },
					"near_clip": { "type": "number" },
					"far_clip": { "type": "number" }
				  },
				  "required": ["resolution", "tick_rate", "fov", "near_clip", "far_clip"]
				}
			  },
			  "required": ["sensor_name", "sensor_type", "sensor_parameters"]
			}
		  }
		}
	  }
	}
  }
}
*/

bool UPayloadProcessor::ParseJson(const FString& JsonString, FSceneGraph& OutSceneGraph)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{

		// === Parse Entities ===
		if (JsonObject->HasField("entities"))
		{
			TSharedPtr<FJsonObject> EntitiesObject = JsonObject->GetObjectField("entities");
			if (EntitiesObject)
			{
				for (auto& EntityPair : EntitiesObject->Values)
				{
					FEntityComponentList EntityComponents;
					TArray<TSharedPtr<FJsonValue>> ComponentsArray = EntityPair.Value->AsArray();

					for (TSharedPtr<FJsonValue> Component : ComponentsArray)
					{
						EntityComponents.Components.Add(Component->AsString());
					}

					OutSceneGraph.Entities.Add(EntityPair.Key, EntityComponents);
				}
			}
		}

		if (JsonObject->HasField("resources"))
		{
			// === Parse Resources ===
			TSharedPtr<FJsonObject> ResourcesObject = JsonObject->GetObjectField("resources");
			if (ResourcesObject)
			{
				FResources Resources;
				TSharedPtr<FJsonObject> Origin = ResourcesObject->GetObjectField("origin");
				Resources.Origin = FVector(
					Origin->GetNumberField("latitude"),
					Origin->GetNumberField("longitude"),
					Origin->GetNumberField("altitude"));

				TSharedPtr<FJsonObject> Weather = ResourcesObject->GetObjectField("weather");
				Resources.Weather.Preset = Weather->GetStringField("preset");

				TSharedPtr<FJsonObject> ViewportConfig = ResourcesObject->GetObjectField("viewport_config");
				if(ViewportConfig)
				{
					Resources.ViewportConfig.ActiveViewport = ViewportConfig->GetStringField("active_camera");
					Resources.ViewportConfig.RendererInstanceID = ViewportConfig->GetStringField("renderer_instance");
				}
				Resources.bResourcesSet = true;

				OutSceneGraph.Resources = Resources;
			}
		}

		if (JsonObject->HasField("components"))
		{
			// === Parse Components ===
			TSharedPtr<FJsonObject> ComponentsObject = JsonObject->GetObjectField("components");
			if (!ComponentsObject)
				return false;

			if (ComponentsObject->HasField("actor_properties"))
			{
				// Parse actor properties
				TSharedPtr<FJsonObject> ActorProperties = ComponentsObject->GetObjectField("actor_properties");
				for (auto& ActorPair : ActorProperties->Values)
				{
					FActorProperties ActorData;
					TSharedPtr<FJsonObject> ActorInfo = ActorPair.Value->AsObject();

					ActorData.ActorName = ActorInfo->GetStringField("actor_name");
					ActorData.ActorAsset = ActorInfo->GetStringField("actor_asset");
					ActorData.Parent = ActorInfo->GetStringField("parent");

					OutSceneGraph.Components.ActorProperties.Add(ActorPair.Key, ActorData);
				}
			}

			if (ComponentsObject->HasField("actor_state"))
			{
				// Parse transforms
				TSharedPtr<FJsonObject> ActorState = ComponentsObject->GetObjectField("actor_state");
				for (auto& ActorPair : ActorState->Values)
				{
					FTransformSceneGraph Transform;
					TSharedPtr<FJsonObject> TransformObject = ActorPair.Value->AsObject()->GetObjectField("pose")->GetObjectField("transform");

					Transform.Position = FVector(
						TransformObject->GetObjectField("position")->GetNumberField("x"),
						TransformObject->GetObjectField("position")->GetNumberField("y"),
						TransformObject->GetObjectField("position")->GetNumberField("z"));

					FQuat Aux;
					Aux.X = TransformObject->GetObjectField("orientation")->GetNumberField("x");
					Aux.Y = TransformObject->GetObjectField("orientation")->GetNumberField("y");
					Aux.Z = TransformObject->GetObjectField("orientation")->GetNumberField("z");
					Aux.W = TransformObject->GetObjectField("orientation")->GetNumberField("w");

					if (Aux.Size() < TMathUtilConstants<float>::Epsilon)
					{
						UE_LOG(LogCore, Error, TEXT("Invalid quaternion in actor state, setting to identity."));
						Aux = FQuat::Identity;
					}

					Transform.Scale = FVector(
						TransformObject->GetObjectField("scale")->GetNumberField("x"),
						TransformObject->GetObjectField("scale")->GetNumberField("y"),
						TransformObject->GetObjectField("scale")->GetNumberField("z"));

					aerosim_quat_wxyz_to_rpy(
						Aux.W,
						Aux.X,
						Aux.Y,
						Aux.Z,
						&Transform.Rotation.Roll,
						&Transform.Rotation.Pitch,
						&Transform.Rotation.Yaw);

					FActorState State;
					State.Pose = Transform;
					OutSceneGraph.Components.ActorStates.Add(ActorPair.Key, State);
				}
			}

			if (ComponentsObject->HasField("sensor"))
			{
				// Parse sensor data
				TSharedPtr<FJsonObject> SensorComponent = ComponentsObject->GetObjectField("sensor");
				for (auto& SensorPair : SensorComponent->Values)
				{
					FSensorData Sensor;
					TSharedPtr<FJsonObject> SensorInfo = SensorPair.Value->AsObject();
					Sensor.SensorName = SensorInfo->GetStringField("sensor_name");
					Sensor.SensorType = SensorInfo->GetStringField("sensor_type");

					TSharedPtr<FJsonObject> SensorParameters = SensorInfo->GetObjectField("sensor_parameters");
					TSharedPtr<FJsonObject> RGBAParams = SensorParameters->GetObjectField("RGBCamera");

					Sensor.TickRate = RGBAParams->GetNumberField("tick_rate");
					Sensor.FOV = RGBAParams->GetNumberField("fov");
					Sensor.NearClip = RGBAParams->GetNumberField("near_clip");
					Sensor.FarClip = RGBAParams->GetNumberField("far_clip");

					if (RGBAParams->HasField("projection_type"))
					{
						if (RGBAParams->GetStringField("projection_type").Equals("orthographic"))
						{
							Sensor.ProjectionMode = ECameraProjectionMode::Type::Orthographic;
						}
						else
						{
							Sensor.ProjectionMode = ECameraProjectionMode::Type::Perspective;
						}
					}
					if (RGBAParams->HasField("ortographic_width"))
					{
						Sensor.OrthoWidth = RGBAParams->GetNumberField("ortographic_width");
					}
					Sensor.bCaptureEnabled = RGBAParams->GetBoolField("capture_enabled");
					// Extract resolution array
					const TArray<TSharedPtr<FJsonValue>>* ResolutionArray;
					if (RGBAParams->TryGetArrayField(TEXT("resolution"), ResolutionArray) && ResolutionArray->Num() == 2)
					{
						Sensor.Resolution.X = (*ResolutionArray)[0]->AsNumber();
						Sensor.Resolution.Y = (*ResolutionArray)[1]->AsNumber();
					}

					OutSceneGraph.Components.Sensors.Add(SensorPair.Key, Sensor);
				}
			}

			if (ComponentsObject->HasField("effectors"))
			{
				// Parse effectors
				TSharedPtr<FJsonObject> EffectorsComponent = ComponentsObject->GetObjectField("effectors");
				for (auto& EffectorPair : EffectorsComponent->Values)
				{
					TArray<FEffectorData> EffectorList;
					for (TSharedPtr<FJsonValue> EffectorValue : EffectorPair.Value->AsArray())
					{
						FEffectorData Effector;
						TSharedPtr<FJsonObject> EffectorObject = EffectorValue->AsObject();

						Effector.EffectorID = EffectorObject->GetStringField("effector_id");
						Effector.USDPath = EffectorObject->GetStringField("relative_path");

						// Store transform
						TSharedPtr<FJsonObject> TransformObject = EffectorObject->GetObjectField("pose")->GetObjectField("transform");
						Effector.Transform.Position = FVector(
							TransformObject->GetObjectField("position")->GetNumberField("x"),
							TransformObject->GetObjectField("position")->GetNumberField("y"),
							TransformObject->GetObjectField("position")->GetNumberField("z"));

						FQuat Aux;
						Aux.X = TransformObject->GetObjectField("orientation")->GetNumberField("x");
						Aux.Y = TransformObject->GetObjectField("orientation")->GetNumberField("y");
						Aux.Z = TransformObject->GetObjectField("orientation")->GetNumberField("z");
						Aux.W = TransformObject->GetObjectField("orientation")->GetNumberField("w");

						if (Aux.Size() < TMathUtilConstants<float>::Epsilon)
						{
							UE_LOG(LogCore, Error, TEXT("Invalid quaternion in effector state, setting to identity."));
							Aux = FQuat::Identity;
						}

						Effector.Transform.Scale = FVector(
							TransformObject->GetObjectField("scale")->GetNumberField("x"),
							TransformObject->GetObjectField("scale")->GetNumberField("y"),
							TransformObject->GetObjectField("scale")->GetNumberField("z"));

						aerosim_quat_wxyz_to_rpy(
							Aux.W,
							Aux.X,
							Aux.Y,
							Aux.Z,
							&Effector.Transform.Rotation.Roll,
							&Effector.Transform.Rotation.Pitch,
							&Effector.Transform.Rotation.Yaw);

						EffectorList.Add(Effector);
					}

					if (!OutSceneGraph.Components.Effectors.Contains(EffectorPair.Key))
					{
						OutSceneGraph.Components.Effectors.Add(EffectorPair.Key, FEffectorList());
					}
					OutSceneGraph.Components.Effectors[EffectorPair.Key] = FEffectorList(EffectorList);
				}
			}

			if (ComponentsObject->HasField("primary_flight_display_state"))
			{
				// Parse PFD state data
				TSharedPtr<FJsonObject> PFDComponentJSON = ComponentsObject->GetObjectField("primary_flight_display_state");
				for (auto& [Entity, ComponentValue] : PFDComponentJSON->Values)
				{
					TSharedPtr<FJsonObject> ComponentObject = ComponentValue->AsObject();
					TSharedPtr<FJsonObject> PFDDataJSON = ComponentObject->GetObjectField("pfd_data");

					FPrimaryFlightDisplayData PFDState;
					PFDState.AirspeedKts = PFDDataJSON->GetNumberField("airspeed_kts");
					PFDState.TrueAirspeedKts = PFDDataJSON->GetNumberField("true_airspeed_kts");
					PFDState.AltitudeFt = PFDDataJSON->GetNumberField("altitude_ft");
					PFDState.TargetAltitudeFt = PFDDataJSON->GetNumberField("target_altitude_ft");
					PFDState.AltimeterPressureSettingInHg = PFDDataJSON->GetNumberField("altimeter_pressure_setting_inhg");
					PFDState.VerticalSpeedFpm = PFDDataJSON->GetNumberField("vertical_speed_fpm");
					PFDState.PitchDeg = PFDDataJSON->GetNumberField("pitch_deg");
					PFDState.RollDeg = PFDDataJSON->GetNumberField("roll_deg");
					PFDState.SideSlipFps2 = PFDDataJSON->GetNumberField("side_slip_fps2");
					PFDState.HeadingDeg = PFDDataJSON->GetNumberField("heading_deg");
					PFDState.HsiCourseSelectHeadingDeg = PFDDataJSON->GetNumberField("hsi_course_select_heading_deg");
					PFDState.HsiCourseDeviationDeg = PFDDataJSON->GetNumberField("hsi_course_deviation_deg");
					PFDState.HsiMode = PFDDataJSON->GetIntegerField("hsi_mode");
					OutSceneGraph.Components.PrimaryFlightDisplays.Add(Entity, PFDState);
				}
			}
			if (ComponentsObject->HasField("trajectory"))
			{
				TSharedPtr<FJsonObject> TrajectorySettingsProperties = ComponentsObject->GetObjectField("trajectory");
				for (auto& TrajectoryPair : TrajectorySettingsProperties->Values)
				{
					TSharedPtr<FJsonObject> Info = TrajectoryPair.Value->AsObject()->GetObjectField("parameters");
					{
						TSharedPtr<FJsonObject> SettingsInfo = Info->GetObjectField("settings");
						FTrajectoryVisualizationSettingsData Settings;
						Settings.DisplayFutureTrajectory = SettingsInfo->GetBoolField("display_future_trajectory");
						Settings.DisplayPastTrajectory = SettingsInfo->GetBoolField("display_past_trajectory");
						Settings.HighlightUserDefinedWaypoints = SettingsInfo->GetBoolField("highlight_user_defined_waypoints");
						Settings.NumberOfFutureWaypoints = SettingsInfo->GetIntegerField("number_of_future_waypoints");
						if (!OutSceneGraph.Components.TrajectoryVisualizationSettings.Contains(TrajectoryPair.Key))
						{
							OutSceneGraph.Components.TrajectoryVisualizationSettings.Add(TrajectoryPair.Key, FTrajectoryVisualizationSettingsData());
						}
						OutSceneGraph.Components.TrajectoryVisualizationSettings[TrajectoryPair.Key] = Settings;
					}
					{
						TSharedPtr<FJsonObject> UserDefinedWaypointsInfo = Info->GetObjectField("user_defined_waypoints");
						TArray<TSharedPtr<FJsonValue>> WaypointsJson = UserDefinedWaypointsInfo->GetArrayField("waypoints");
						FTrajectoryVisualizationWaypointsData Waypoints;
						for (auto& WaypointValue : WaypointsJson)
						{
							if (WaypointValue->Type == EJson::Array)
							{
								TArray<TSharedPtr<FJsonValue>> CoordArray = WaypointValue->AsArray();
								if (CoordArray.Num() == 3)
								{
									const float X = CoordArray[0]->AsNumber();
									const float Y = CoordArray[1]->AsNumber();
									const float Z = CoordArray[2]->AsNumber();
									// Convert from NED to UE5 coordinate system
									Waypoints.Waypoints.Add(FVector(Y * 100.0f, -X * 100.0f, -Z * 100.0f));
								}
								else
								{
									UE_LOG(LogCore, Error, TEXT("The waypoint element is not an array of 3 elements"));
								}
							}
							else
							{
								UE_LOG(LogCore, Error, TEXT("The waypoint element is not an array"));
							}
						}
						if (Waypoints.Waypoints.Num() > 0)
						{
							if (!OutSceneGraph.Components.TrajectoryVisualizationUserDefinedWaypoints.Contains(TrajectoryPair.Key))
							{
								OutSceneGraph.Components.TrajectoryVisualizationUserDefinedWaypoints.Add(TrajectoryPair.Key, FTrajectoryVisualizationWaypointsData(Waypoints));
							}
							else
							{
								OutSceneGraph.Components.TrajectoryVisualizationUserDefinedWaypoints[TrajectoryPair.Key] = FTrajectoryVisualizationWaypointsData(Waypoints);
							}
						}
					}
					{
						TSharedPtr<FJsonObject> FutureTrajectoryWaypointsInfo = Info->GetObjectField("future_trajectory");
						TArray<TSharedPtr<FJsonValue>> WaypointsJson = FutureTrajectoryWaypointsInfo->GetArrayField("waypoints");
						FTrajectoryVisualizationWaypointsData Waypoints;
						for (auto& WaypointValue : WaypointsJson)
						{
							if (WaypointValue->Type == EJson::Array)
							{
								TArray<TSharedPtr<FJsonValue>> CoordArray = WaypointValue->AsArray();
								if (CoordArray.Num() == 3)
								{
									const float X = CoordArray[0]->AsNumber();
									const float Y = CoordArray[1]->AsNumber();
									const float Z = CoordArray[2]->AsNumber();
									// Convert from NED to UE5 coordinate system
									Waypoints.Waypoints.Add(FVector(Y * 100.0f, -X * 100.0f, -Z * 100.0f));
								}
								else
								{
									UE_LOG(LogCore, Error, TEXT("The waypoint element is not an array of 3 elements"));
								}
							}
							else
							{
								UE_LOG(LogCore, Error, TEXT("The waypoint element is not an array"));
							}
						}
						if (Waypoints.Waypoints.Num() > 0)
						{
							if (!OutSceneGraph.Components.TrajectoryVisualizationFutureTrajectoryWaypoints.Contains(TrajectoryPair.Key))
							{
								OutSceneGraph.Components.TrajectoryVisualizationFutureTrajectoryWaypoints.Add(TrajectoryPair.Key, FTrajectoryVisualizationWaypointsData(Waypoints));
							}
							else
							{
								OutSceneGraph.Components.TrajectoryVisualizationFutureTrajectoryWaypoints[TrajectoryPair.Key] = FTrajectoryVisualizationWaypointsData(Waypoints);
							}
						}
					}
				}
			}
		}

		return true;
	}
	return false;
}
