#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeatherMapDataAsset.generated.h"

UCLASS(BlueprintType)
class AEROSIMCONNECTOR_API UWeatherMapDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WeatherMap")
    TMap<FString, FString> WeatherMap;
};
