#pragma once

#include "HUD/PFDWidget.h"
#include "Game/AerosimGameMode.h"
#include "Game/Subsystems/AerosimDataTracker.h"
#include "Kismet/GameplayStatics.h"

void UPFDWidget::GetData(float& Airspeed, float& TrueAirspeed, float& Altitude, float& TargetAltitude, float& Pressure,
	float& VerticalSpeed, float& Pitch, float& BankAngle, float& SlipValue, float& Heading, float& NeedleHeading,
	float& Deviation, int& NeedleMode)
{
	AAerosimGameMode* GameMode = Cast<AAerosimGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!IsValid(GameMode))
	{
		UE_LOG(LogCore, Error, TEXT("GameMode Not Found: %s"));
		return;
	}

	UAerosimDataTracker* DataTracker = GameMode->GetAerosimDataTracker();
	if (!IsValid(DataTracker))
	{
		UE_LOG(LogCore, Error, TEXT("DataTracker is not valid"))
	}
	else
	{
		int WidgetId = GetId();
		if (DataTracker->Airspeeds.Contains(WidgetId))
		{
			Airspeed = DataTracker->Airspeeds[WidgetId];
		}
		if (DataTracker->TrueAirspeeds.Contains(WidgetId))
		{
			TrueAirspeed = DataTracker->TrueAirspeeds[WidgetId];
		}
		if (DataTracker->Altitudes.Contains(WidgetId))
		{
			Altitude = DataTracker->Altitudes[WidgetId];
		}
		if (DataTracker->TargetAltitudes.Contains(WidgetId))
		{
			TargetAltitude = DataTracker->TargetAltitudes[WidgetId];
		}
		if (DataTracker->Pressures.Contains(WidgetId))
		{
			Pressure = DataTracker->Pressures[WidgetId];
		}
		if (DataTracker->VerticalSpeeds.Contains(WidgetId))
		{
			VerticalSpeed = DataTracker->VerticalSpeeds[WidgetId];
		}
		if (DataTracker->Pitchs.Contains(WidgetId))
		{
			Pitch = DataTracker->Pitchs[WidgetId];
		}
		if (DataTracker->BankAngles.Contains(WidgetId))
		{
			BankAngle = DataTracker->BankAngles[WidgetId];
		}
		if (DataTracker->Slips.Contains(WidgetId))
		{
			SlipValue = DataTracker->Slips[WidgetId];
		}
		if (DataTracker->Headings.Contains(WidgetId))
		{
			Heading = DataTracker->Headings[WidgetId];
		}
		if (DataTracker->NeedleHeadings.Contains(WidgetId))
		{
			NeedleHeading = DataTracker->NeedleHeadings[WidgetId];
		}
		if (DataTracker->Deviations.Contains(WidgetId))
		{
			Deviation = DataTracker->Deviations[WidgetId];
		}
		if (DataTracker->NeedleModes.Contains(WidgetId))
		{
			NeedleMode = DataTracker->NeedleModes[WidgetId];
		}
	}
}
