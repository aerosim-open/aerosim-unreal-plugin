// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Variant.h"

#include "AerosimDataTracker.generated.h"

UENUM(BlueprintType)
enum class EDataTrackerType : uint8
{
	Airspeed = 0 UMETA(DisplayName = "Airspeed"),
	TrueAirspeed = 1 UMETA(DisplayName = "True Airspeed"),
	Altitude = 2 UMETA(DisplayName = "Altitude"),
	TargetAltitude = 3 UMETA(DisplayName = "Target Altitude"),
	Pressure = 4 UMETA(DisplayName = "Pressure"),
	VerticalSpeed = 5 UMETA(DisplayName = "Vertical Speed"),
	Pitch = 6 UMETA(DisplayName = "Bank Angle"),
	BankAngle = 7 UMETA(DisplayName = "Bank Angle"),
	Slip = 8 UMETA(DisplayName = "Slip"),
	Heading = 9 UMETA(DisplayName = "Heading"),
	NeedleHeading = 10 UMETA(DisplayName = "Needle Heading"),
	Deviation = 11 UMETA(DisplayName = "Deviation"),
	NeedleMode = 12 UMETA(DisplayName = "NeedleMode")
};

UCLASS()
class AEROSIMCONNECTOR_API UAerosimDataTracker : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Airspeed Tape")
	TMap<int32, float> Airspeeds;
	UPROPERTY(BlueprintReadWrite, Category = "Airspeed Tape")
	TMap<int32, float> TrueAirspeeds;
	UPROPERTY(BlueprintReadWrite, Category = "Altitude Tape")
	TMap<int32, float> Altitudes;
	UPROPERTY(BlueprintReadWrite, Category = "Altitude Tape")
	TMap<int32, float> TargetAltitudes;
	UPROPERTY(BlueprintReadWrite, Category = "Altitude Tape")
	TMap<int32, float> Pressures;
	UPROPERTY(BlueprintReadWrite, Category = "Vertical Speed Indicator Tape")
	TMap<int32, float> VerticalSpeeds;
	UPROPERTY(BlueprintReadWrite, Category = "Bank Angle Indicator Tape")
	TMap<int32, float> Pitchs;
	UPROPERTY(BlueprintReadWrite, Category = "Bank Angle Indicator Tape")
	TMap<int32, float> BankAngles;
	UPROPERTY(BlueprintReadWrite, Category = "Bank Angle Indicator Tape")
	TMap<int32, float> Slips;
	UPROPERTY(BlueprintReadWrite, Category = "Horizontal Situation Indicator Tape")
	TMap<int32, float> Headings;
	UPROPERTY(BlueprintReadWrite, Category = "Horizontal Situation Indicator Tape")
	TMap<int32, float> NeedleHeadings;
	UPROPERTY(BlueprintReadWrite, Category = "Horizontal Situation Indicator Tape")
	TMap<int32, float> Deviations;
	UPROPERTY(BlueprintReadWrite, Category = "Horizontal Situation Indicator Tape")
	TMap<int32, int> NeedleModes;

	void UpdateDataTracker(EDataTrackerType DataTrackerType, int InstanceId, TVariant<FVector, int, float> Value);

private:
	void UpdateAirSpeed(int InstanceId, float Value);
	void UpdateTrueAirSpeed(int InstanceId, float Value);
	void UpdateAltitude(int InstanceId, float Value);
	void UpdateTargetAltitude(int InstanceId, float Value);
	void UpdatePressure(int InstanceId, float Value);
	void UpdateVerticalSpeed(int InstanceId, float Value);
	void UpdatePitch(int InstanceId, float Value);
	void UpdateBankAngle(int InstanceId, float Value);
	void UpdateSlip(int InstanceId, float Value);
	void UpdateHeading(int InstanceId, float Value);
	void UpdateNeedleHeading(int InstanceId, float Value);
	void UpdateDeviation(int InstanceId, float Value);
	void UpdateNeedleMode(int InstanceId, int Value);
};
