#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "GameFramework/Actor.h"

#include "ActorRegistry.generated.h"

USTRUCT(BlueprintType)
struct FSpawnableActorInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawnable Actor")
	int32 ActorTypeId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawnable Actor")
	FString ActorTypeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawnable Actor")
	TSoftClassPtr<AActor> ActorClass;
};

UCLASS(Blueprintable)
class AEROSIMCONNECTOR_API UActorRegistry : public UObject
{
	GENERATED_BODY()

public:
	UActorRegistry() {};
	~UActorRegistry() {};

	void SetWorldReference(UWorld* NewWorldReference)
	{
		WorldReference = NewWorldReference;
	};

	void RegisterInitialActors(uint32 InitialIDForAlreadySpawnedActors);
	bool RegisterActorById(int InstanceId, int ActorTypeId, FVector Position, FRotator Rotation, bool bDestroyExisting = true);
	bool RegisterActorByName(int InstanceId, const FString& ActorTypeName, FVector Position, FRotator Rotation, bool bDestroyExisting = true);

	uint32 RegisterActorById(int ActorTypeId, FVector Position, FRotator Rotation);
	uint32 RegisterActorByName(const FString& ActorTypeName, FVector Position, FRotator Rotation);

	AActor* GetActor(int InstanceId);
	bool RemoveActor(int InstanceId);

private:
	TSoftClassPtr<AActor> FindActorClassById(uint32 ActorTypeID);
	TSoftClassPtr<AActor> FindActorClassByName(FString ActorTypeName);

	AActor* SpawnActorById(uint32 ActorTypeId, FVector Location, FRotator Rotation);
	AActor* SpawnActorByName(FString ActorTypeName, FVector Location, FRotator Rotation);

private:
	UPROPERTY(EditAnywhere, Category = "Actor Definitions", meta = (AllowPrivateAccess = "true"))
	TArray<FSpawnableActorInfo> SpawnableActorTypes;

	UPROPERTY(VisibleAnywhere, Category = "Actor Registry")
	TMap<uint32, AActor*> ActiveActors{};

	UPROPERTY()
	UWorld* WorldReference;
};
