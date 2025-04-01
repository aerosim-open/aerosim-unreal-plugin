#include "Actors/DepthCaptureSensor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/World.h"

// Constructor
ADepthCaptureSensor::ADepthCaptureSensor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRootComponent"));
	RootComponent = SceneRootComponent;

	// Create the Scene Capture Component and configure it
	SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent"));
	SceneCaptureComponent->SetupAttachment(SceneRootComponent);
	SceneCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic; // Set to orthographic
}

// Called when the game starts or when spawned
void ADepthCaptureSensor::BeginPlay()
{
	Super::BeginPlay();

	DepthRenderTarget = NewObject<UTextureRenderTarget2D>();

	// TODO: Parametrize Pixelformat and LinearGamma
	DepthRenderTarget->InitCustomFormat(Resolution.X, Resolution.Y, EPixelFormat::PF_FloatRGBA, true);
	DepthRenderTarget->UpdateResourceImmediate();

	// TODO: Decide when we want to get the frame
	SceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_SceneDepth;
	SceneCaptureComponent->TextureTarget = DepthRenderTarget;
	SceneCaptureComponent->bCaptureEveryFrame = true;
}

void ADepthCaptureSensor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float ADepthCaptureSensor::SampleHeight(int NumSamples)
{
	float TotalDistance = 0.0f;
	int ValidSamples = 0;

	// Calculate the center of the image
	int CenterX = Resolution.X / 2;
	int CenterY = Resolution.Y / 2;

	for (int i = 0; i < NumSamples; i++)
	{
		// Generate random pixels weighted towards the center
		int RandomX = CenterX + FMath::RandRange(-CenterX / 2, CenterX / 2);
		int RandomY = CenterY + FMath::RandRange(-CenterY / 2, CenterY / 2);

		// Clamp the values to ensure they stay within the resolution bounds
		RandomX = FMath::Clamp(RandomX, 0, Resolution.X - 1);
		RandomY = FMath::Clamp(RandomY, 0, Resolution.Y - 1);

		float DistanceValue = GetDistanceAtPixel(RandomX, RandomY);

		if (DistanceValue >= 0.0f && DistanceValue <= 500000) // Check if the distance value is valid (The depth sensor cannot measure distances greater than 65536 so 50000 is a safe round value	that ensures the value is valid)
		{
			TotalDistance += DistanceValue;
			ValidSamples++;
		}

		// Optionally print the value of each sampled pixel
		// UE_LOG(LogTemp, Log, TEXT("Distance at weighted pixel (%d, %d): %f"), RandomX, RandomY, DistanceValue);
	}

	if (ValidSamples > NumSamples / 2) // If more than half of the samples are valid, return the average
	{
		UE_LOG(LogTemp, Log, TEXT("Distance: %f"), TotalDistance / ValidSamples);
		return TotalDistance / ValidSamples;
	}
	else
	{
		return -1.0f; // Indicate that not enough valid samples were found
	}
}

float ADepthCaptureSensor::GetDistanceAtPixel(int X, int Y)
{
	if (!DepthRenderTarget)
		return -1.0f;

	FTextureRenderTargetResource* RenderTargetResource = DepthRenderTarget->GameThread_GetRenderTargetResource();
	TArray<FLinearColor> PixelData;
	RenderTargetResource->ReadLinearColorPixels(PixelData);

	int32 Index = Y * DepthRenderTarget->SizeX + X;
	if (PixelData.IsValidIndex(Index))
	{
		return PixelData[Index].R; // Normalized depth value
	}

	return -1.0f;
}
