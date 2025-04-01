
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DepthCaptureSensor.generated.h"

UCLASS(Blueprintable)
class AEROSIMCONNECTOR_API ADepthCaptureSensor : public AActor
{
    GENERATED_BODY()

public:
    ADepthCaptureSensor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Depth Capture")
    float GetDistanceAtPixel(int X, int Y);

    UFUNCTION(BlueprintCallable, Category = "Depth Capture")
    float SampleHeight(int NumSamples);

protected:
    TArray<FColor> GetCurrentFrame();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* SceneRootComponent;
	
    UPROPERTY(VisibleAnywhere, Category="Depth Capture")
    USceneCaptureComponent2D* SceneCaptureComponent;

    UPROPERTY(VisibleAnywhere, Category="Depth Capture")
    UTextureRenderTarget2D* DepthRenderTarget;

	FVector2D Resolution = {480, 480};
};
