#pragma once

#include "CoreMinimal.h"
#include "HUD/AerosimWidget.h"

#include "PFDWidget.generated.h"

class UUserWidget;

UCLASS()
class AEROSIMCONNECTOR_API UPFDWidget : public UAerosimWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Data")
	void GetData(float& Airspeed, float& TrueAirspeed, float& Altitude, float& TargetAltitude, float& Pressure,
		float& VerticalSpeed, float& Pitch, float& BankAngle, float& SlipValue, float& Heading, float& NeedleHeading,
		float& Deviation, int& NeedleMode);
};
