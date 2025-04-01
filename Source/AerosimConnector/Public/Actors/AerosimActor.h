// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "USDStageActor.h"
#include "ObjectLabel.h"

#include "AerosimActor.generated.h"

class UTrajectoryVisualizerComponent;
class UPFDWidget;

UCLASS(Blueprintable)
class AEROSIMCONNECTOR_API AAerosimActor : public AUsdStageActor
{
	GENERATED_BODY()

public:
	AAerosimActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Info")
	FString ActorName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Info")
	int ActorInstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UPFDWidget* Widget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* SceneRootComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* CGOffsetComponent; // offset to move actor origin from mesh origin to CG

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UTrajectoryVisualizerComponent* TrajectoryVisualizerComponent;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	inline const TArray<ObjectLabel>& GetSemanticTags() const { return SemanticTags; };
	inline TArray<ObjectLabel>& GetSemanticTags() { return SemanticTags; };
	USceneComponent* GetChildrenActorByChildName(const FString& UsdNameToSearch);

	void AddRelativeTransformToChild(const FString& UsdNameToSearch, const FTransform& DeltaTransform);
	void AddWorldTransformToChild(const FString& UsdNameToSearch, const FTransform& DeltaTransform);

	void AddRelativeLocationToChild(const FString& UsdNameToSearch, const FVector& DeltaLocation);
	void AddWorldLocationToChild(const FString& UsdNameToSearch, const FVector& DeltaLocation);

	void AddRelativeRotationToChild(const FString& UsdNameToSearch, const FRotator& DeltaRotation);
	void AddWorldRotationToChild(const FString& UsdNameToSearch, const FRotator& DeltaRotation);

	void SetWorldTransformToChild(const FString& UsdNameToSearch, const FTransform& Transform);
	void SetRelativeTransformToChild(const FString& UsdNameToSearch, const FTransform& Transform);

	void UpdateTrajectoryVisualizer(FVector Position);
	void UpdateTrajectoryVisualizerUserDefinedWaypoints(const TArray<FVector>& Waypoints);
	void UpdateTrajectoryVisualizerFutureTrajectory(const TArray<FVector>& Waypoints);
	void UpdateTrajectoryVisualizerSettings(bool bDisplayFutureTrajectory, bool bDisplayPastWaypoints, bool bHighlightUserDefinedWaypoints, uint32 NumberOfFutureWaypoints);

	UFUNCTION(BlueprintCallable, Category = "Identification")
	void SetWidgetID(int Id);

private:
	TArray<ObjectLabel> SemanticTags{};
};
