// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Sensor.h"
#include "IntVectorTypes.h"

#include "CameraSensor.generated.h"

class ASceneCapture2D;

UCLASS(Blueprintable)
class AEROSIMCONNECTOR_API ACameraSensor : public ASensor
{
	GENERATED_BODY()

public:
	ACameraSensor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }

	UFUNCTION(BlueprintCallable)
	ASceneCapture2D* GetSceneCaptureActor() const { return SceneCaptureActor; }

	void SetCaptureEnabled(bool bCaptureEnabledIn);

protected:
	void GetCurrentFrame();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	ASceneCapture2D* SceneCaptureActor;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UTextureRenderTarget2D* RenderTarget;

	bool bCaptureEnabled = true;

	FVector2D Resolution = { 1920, 1080 };
};
