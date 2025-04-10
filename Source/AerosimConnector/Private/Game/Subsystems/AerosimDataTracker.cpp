#include "Game/Subsystems/AerosimDataTracker.h"
#include "Misc/Variant.h"

void UAerosimDataTracker::UpdateDataTracker(EDataTrackerType DataTrackerType, int InstanceId, TVariant<FVector, int, float> Value)
{
	switch (DataTrackerType)
	{
		case EDataTrackerType::Airspeed:
		{
			UpdateAirSpeed(InstanceId, Value.Get<float>());
			break;
		}
		case EDataTrackerType::TrueAirspeed:
		{
			UpdateTrueAirSpeed(InstanceId, Value.Get<float>());
			break;
		}
		case EDataTrackerType::Altitude:
		{
			UpdateAltitude(InstanceId, Value.Get<float>());
			break;
		}
		case EDataTrackerType::TargetAltitude:
		{
			UpdateTargetAltitude(InstanceId, Value.Get<float>());
			break;
		}
		case EDataTrackerType::Pressure:
		{
			UpdatePressure(InstanceId, Value.Get<float>());
			break;
		}
		case EDataTrackerType::VerticalSpeed:
		{
			UpdateVerticalSpeed(InstanceId, Value.Get<float>());
			break;
		}
		case EDataTrackerType::Pitch:
		{
			UpdatePitch(InstanceId, Value.Get<float>());
			break;
		}
		case EDataTrackerType::BankAngle:
		{
			UpdateBankAngle(InstanceId, Value.Get<float>());
			break;
		}
		case EDataTrackerType::Slip:
		{
			UpdateSlip(InstanceId, Value.Get<float>());
			break;
		}
		case EDataTrackerType::Heading:
		{
			UpdateHeading(InstanceId, Value.Get<float>());
			break;
		}
		case EDataTrackerType::NeedleHeading:
		{
			UpdateNeedleHeading(InstanceId, Value.Get<float>());
			break;
		}
		case EDataTrackerType::Deviation:
		{
			UpdateDeviation(InstanceId, Value.Get<float>());
			break;
		}
		case EDataTrackerType::NeedleMode:
		{
			UpdateNeedleMode(InstanceId, Value.Get<int>());
			break;
		}
		default:
		{
			UE_LOG(LogAerosimConnector, Error, TEXT("You are trying to update an invalid data tracker type"))
			break;
		}
	}
}

void UAerosimDataTracker::UpdateAirSpeed(int InstanceId, float Value)
{
	if (!Airspeeds.Contains(InstanceId))
	{
		Airspeeds.Add(InstanceId, Value);
	}
	else
	{
		Airspeeds[InstanceId] = Value;
	}
}

void UAerosimDataTracker::UpdateTrueAirSpeed(int InstanceId, float Value)
{
	if (!TrueAirspeeds.Contains(InstanceId))
	{
		TrueAirspeeds.Add(InstanceId, Value);
	}
	else
	{
		TrueAirspeeds[InstanceId] = Value;
	}
}

void UAerosimDataTracker::UpdateAltitude(int InstanceId, float Value)
{
	if (!Altitudes.Contains(InstanceId))
	{
		Altitudes.Add(InstanceId, Value);
	}
	else
	{
		Altitudes[InstanceId] = Value;
	}
}

void UAerosimDataTracker::UpdateTargetAltitude(int InstanceId, float Value)
{
	if (!TargetAltitudes.Contains(InstanceId))
	{
		TargetAltitudes.Add(InstanceId, Value);
	}
	else
	{
		TargetAltitudes[InstanceId] = Value;
	}
}

void UAerosimDataTracker::UpdatePressure(int InstanceId, float Value)
{
	if (!Pressures.Contains(InstanceId))
	{
		Pressures.Add(InstanceId, Value);
	}
	else
	{
		Pressures[InstanceId] = Value;
	}
}

void UAerosimDataTracker::UpdateVerticalSpeed(int InstanceId, float Value)
{
	if (!VerticalSpeeds.Contains(InstanceId))
	{
		VerticalSpeeds.Add(InstanceId, Value);
	}
	else
	{
		VerticalSpeeds[InstanceId] = Value;
	}
}

void UAerosimDataTracker::UpdatePitch(int InstanceId, float Value)
{
	if (!Pitchs.Contains(InstanceId))
	{
		Pitchs.Add(InstanceId, Value);
	}
	else
	{
		Pitchs[InstanceId] = Value;
	}
}

void UAerosimDataTracker::UpdateBankAngle(int InstanceId, float Value)
{
	if (!BankAngles.Contains(InstanceId))
	{
		BankAngles.Add(InstanceId, Value);
	}
	else
	{
		BankAngles[InstanceId] = Value;
	}
}

void UAerosimDataTracker::UpdateSlip(int InstanceId, float Value)
{
	if (!Slips.Contains(InstanceId))
	{
		Slips.Add(InstanceId, Value);
	}
	else
	{
		Slips[InstanceId] = Value;
	}
}

void UAerosimDataTracker::UpdateHeading(int InstanceId, float Value)
{
	if (!Headings.Contains(InstanceId))
	{
		Headings.Add(InstanceId, Value);
	}
	else
	{
		Headings[InstanceId] = Value;
	}
}

void UAerosimDataTracker::UpdateNeedleHeading(int InstanceId, float Value)
{
	if (!NeedleHeadings.Contains(InstanceId))
	{
		NeedleHeadings.Add(InstanceId, Value);
	}
	else
	{
		NeedleHeadings[InstanceId] = Value;
	}
}

void UAerosimDataTracker::UpdateDeviation(int InstanceId, float Value)
{
	if (!Deviations.Contains(InstanceId))
	{
		Deviations.Add(InstanceId, Value);
	}
	else
	{
		Deviations[InstanceId] = Value;
	}
}

void UAerosimDataTracker::UpdateNeedleMode(int InstanceId, int Value)
{
	if (!NeedleModes.Contains(InstanceId))
	{
		NeedleModes.Add(InstanceId, Value);
	}
	else
	{
		NeedleModes[InstanceId] = Value;
	}
}
