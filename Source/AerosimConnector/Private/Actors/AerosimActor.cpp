#include "Actors/AerosimActor.h"
#include "AerosimConnector.h"
#include "Components/TrajectoryVisualizerComponent.h"
#include "Components/WidgetComponent.h"
#include "HUD/PFDWidget.h"

AAerosimActor::AAerosimActor()
{
	// If sync mode is enable disable it
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRootComponent"));
	RootComponent = SceneRootComponent;

	CGOffsetComponent = CreateDefaultSubobject<USceneComponent>(TEXT("CGOffsetComponent"));
	CGOffsetComponent->SetupAttachment(SceneRootComponent);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CGOffsetComponent);

	SceneComponent->SetupAttachment(CGOffsetComponent); // USDStageActor's root SceneComponent (inherited)

	TrajectoryVisualizerComponent = CreateDefaultSubobject<UTrajectoryVisualizerComponent>(TEXT("TrajectoryVisualizerComponent"));
	TrajectoryVisualizerComponent->SetupAttachment(SceneRootComponent);
	if (!IsValid(TrajectoryVisualizerComponent))
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("TrajectoryVisualizerComponent failed to create"));
	}
}

// Called when the game starts or when spawned
void AAerosimActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AAerosimActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

USceneComponent* AAerosimActor::GetChildrenActorByChildName(const FString& ChildNameToSearch)
{
	USceneComponent* Result = GetGeneratedComponent(ChildNameToSearch);
	if (IsValid(Result))
		return Result;

	UE_LOG(LogAerosimConnector, Error, TEXT("Requested USD part is not valid"));
	return nullptr;
}

void AAerosimActor::AddRelativeTransformToChild(const FString& ChildNameToSearch, const FTransform& DeltaTransform)
{
	USceneComponent* USDNode = GetChildrenActorByChildName(ChildNameToSearch);
	if (IsValid(USDNode))
		USDNode->AddLocalTransform(DeltaTransform);
}

void AAerosimActor::AddWorldTransformToChild(const FString& ChildNameToSearch, const FTransform& DeltaTransform)
{
	USceneComponent* ChildNode = GetChildrenActorByChildName(ChildNameToSearch);
	if (IsValid(ChildNode))
		ChildNode->AddWorldTransform(DeltaTransform);
}

void AAerosimActor::AddRelativeLocationToChild(const FString& ChildNameToSearch, const FVector& DeltaLocation)
{
	USceneComponent* ChildNode = GetChildrenActorByChildName(ChildNameToSearch);
	if (IsValid(ChildNode))
		ChildNode->AddRelativeLocation(DeltaLocation);
}

void AAerosimActor::AddWorldLocationToChild(const FString& ChildNameToSearch, const FVector& DeltaLocation)
{
	USceneComponent* ChildNode = GetChildrenActorByChildName(ChildNameToSearch);
	if (IsValid(ChildNode))
		ChildNode->AddWorldOffset(DeltaLocation);
}

void AAerosimActor::AddRelativeRotationToChild(const FString& ChildNameToSearch, const FRotator& DeltaRotation)
{
	USceneComponent* ChildNode = GetChildrenActorByChildName(ChildNameToSearch);
	if (IsValid(ChildNode))
		ChildNode->AddRelativeRotation(DeltaRotation);
}

void AAerosimActor::AddWorldRotationToChild(const FString& ChildNameToSearch, const FRotator& DeltaRotation)
{
	USceneComponent* ChildNode = GetChildrenActorByChildName(ChildNameToSearch);
	if (IsValid(ChildNode))
		ChildNode->AddWorldRotation(DeltaRotation);
}

void AAerosimActor::SetWorldTransformToChild(const FString& ChildNameToSearch, const FTransform& Transform)
{
	USceneComponent* ChildNode = GetChildrenActorByChildName(ChildNameToSearch);
	if (IsValid(ChildNode))
		ChildNode->SetWorldTransform(Transform);
}

void AAerosimActor::SetRelativeTransformToChild(const FString& ChildNameToSearch, const FTransform& Transform)
{
	USceneComponent* ChildNode = GetChildrenActorByChildName(ChildNameToSearch);
	if (IsValid(ChildNode))
		ChildNode->SetRelativeTransform(Transform);
}

void AAerosimActor::UpdateTrajectoryVisualizer(FVector Position)
{
	if (!IsValid(TrajectoryVisualizerComponent))
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("TrajectoryVisualizerComponent is not valid"));
		return;
	}
	TrajectoryVisualizerComponent->Update(Position);
}

void AAerosimActor::UpdateTrajectoryVisualizerUserDefinedWaypoints(const TArray<FVector>& Waypoints)
{
	if (!IsValid(TrajectoryVisualizerComponent))
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("TrajectoryVisualizerComponent is not valid"));
		return;
	}
	TrajectoryVisualizerComponent->UpdateUserDefinedWaypoints(Waypoints);
}

void AAerosimActor::UpdateTrajectoryVisualizerFutureTrajectory(const TArray<FVector>& Waypoints)
{
	if (!IsValid(TrajectoryVisualizerComponent))
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("UpdateTrajectoryVisualizerAddAheadTrajectory is not valid"));
		return;
	}
	TrajectoryVisualizerComponent->UpdateFutureTrajectory(Waypoints);
}

void AAerosimActor::UpdateTrajectoryVisualizerSettings(bool bDisplayFutureTrajectory, bool bDisplayPastWaypoints, bool bHighlightUserDefinedWaypoints, uint32 NumberOfFutureWaypoints)
{
	if (!IsValid(TrajectoryVisualizerComponent))
	{
		UE_LOG(LogAerosimConnector, Error, TEXT("TrajectoryVisualizerComponent is not valid"));
		return;
	}
	TrajectoryVisualizerComponent->UpdateSettings(bDisplayFutureTrajectory, bDisplayPastWaypoints, bHighlightUserDefinedWaypoints, NumberOfFutureWaypoints);
}

void AAerosimActor::SetWidgetID(int Id)
{
	UWidgetComponent* WidgetComponent = FindComponentByClass<UWidgetComponent>();
	if (!IsValid(WidgetComponent))
	{
		UE_LOG(LogAerosimConnector, VeryVerbose, TEXT("No widget component found in actor %s"), *GetName());
	}
	else
	{
		UUserWidget* UserWidget = WidgetComponent->GetWidget();
		if (!IsValid(UserWidget))
		{
			UE_LOG(LogAerosimConnector, Error, TEXT("No widget object retrieved"));
		}
		Widget = Cast<UPFDWidget>(UserWidget);
		if (!IsValid(Widget))
		{
			UE_LOG(LogAerosimConnector, Error, TEXT("airspeed widget not valid"));
		}
		else
		{
			Widget->SetId(Id);
		}
	}
}
