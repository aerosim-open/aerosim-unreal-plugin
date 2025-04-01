#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Game/Subsystems/SceneGraph.h"
#include "PayloadProcessor.generated.h"

UCLASS()
class AEROSIMCONNECTOR_API UPayloadProcessor : public UObject
{
	GENERATED_BODY()

public:
	UPayloadProcessor() {};
	~UPayloadProcessor() {};

	UFUNCTION(BlueprintCallable, Category = "Payload Processor")
	static bool ParseJson(const FString& JsonString, FSceneGraph& OutSceneGraph);
};
