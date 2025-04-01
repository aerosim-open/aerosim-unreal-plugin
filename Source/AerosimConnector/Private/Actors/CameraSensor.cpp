#include "Actors/CameraSensor.h"
#include "Render/ImageUtil.h"
#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Serialization/BufferArchive.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Util/MessageHandler.h"

ACameraSensor::ACameraSensor()
{
}

// Called when the game starts or when spawned
void ACameraSensor::BeginPlay()
{
	Super::BeginPlay();

	SceneCaptureActor = GetWorld()->SpawnActor<ASceneCapture2D>(ASceneCapture2D::StaticClass());
	SceneCaptureActor->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	USceneCaptureComponent2D* SceneCaptureComponent = SceneCaptureActor->GetCaptureComponent2D();

	RenderTarget = NewObject<UTextureRenderTarget2D>();
	// TODO: Parametrize Pixelformat and LinearGamma
	RenderTarget->InitCustomFormat(Resolution.X, Resolution.Y, EPixelFormat::PF_B8G8R8A8, true); // true=force Linear Gamma
	RenderTarget->UpdateResourceImmediate();

	// TODO: Decide when we want to get the frame
	SceneCaptureComponent->CaptureSource = SCS_FinalColorLDR;
	SceneCaptureComponent->TextureTarget = RenderTarget;
	SceneCaptureComponent->bCaptureEveryFrame = false;
	SceneCaptureComponent->bCaptureOnMovement = false;
	SceneCaptureComponent->bAlwaysPersistRenderingState = true;
}

void ACameraSensor::SetCaptureEnabled(bool bCaptureEnabledIn)
{
	bCaptureEnabled = bCaptureEnabledIn;
}

// Called every frame
void ACameraSensor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCaptureEnabled)
	{
		SceneCaptureActor->GetCaptureComponent2D()->CaptureScene();
		GetCurrentFrame();
	}
}

void ACameraSensor::GetCurrentFrame()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ACameraSensor::GetCurrentFrame);

	// Possible optimizations;
	// 1. GetRawData instead of processed ones from ImageUtil and make clients to process it.
	//             Done, from 2fps avg to 25fps avg
	// 2. Use Multi-threading to get the image data
	// 3. Split the image in chunks
	ImageUtil::ReadSensorDataAsyncRaw(this, [this](const void* Mapping, FIntPoint Size) -> bool {
		TRACE_CPUPROFILER_EVENT_SCOPE(ACameraSensor::RetrieveDataAndPublish);
		publish_image_to_topic("aerosim.renderer.responses", Size.X, Size.Y, 1, Mapping, Size.X * Size.Y * 4);
		return true;
	});
}
