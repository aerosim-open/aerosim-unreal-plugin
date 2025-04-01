#include "Game/Subsystems/ActorRegistry.h"
#include "CoreMinimal.h"
#include "Actors/AerosimActor.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"

static uint32 GCurrentId = 0;

void UActorRegistry::RegisterInitialActors(uint32 InitialIDForAlreadySpawnedActors)
{
	TArray<AActor*> FoundActors;

	// Replace AAerosimActor with the class of the actor we are searching for
	UGameplayStatics::GetAllActorsOfClass(WorldReference, AAerosimActor::StaticClass(), FoundActors);

	int32 InstanceId = InitialIDForAlreadySpawnedActors;
	for (AActor* Actor : FoundActors)
	{
		ActiveActors.Emplace((uint32)InstanceId, Actor);
		InstanceId++;
	}
}

bool UActorRegistry::RegisterActorById(int InstanceId, int ActorTypeId, FVector Position, FRotator Rotation, bool bDestroyExisting)
{
	if (ActiveActors.Contains((uint32)InstanceId))
	{
		if (!bDestroyExisting)
		{
			UE_LOG(LogCore, Warning, TEXT("Instance at ID %d already in registry, could not register."), InstanceId);
			return false;
		}
		else
		{
			UE_LOG(LogCore, Warning, TEXT("Instance at ID %d already in registry, destroying the existing actor to spawn new one."), InstanceId);
			RemoveActor(InstanceId);
		}
	}

	AActor* NewActor = SpawnActorById((uint32)ActorTypeId, Position, Rotation);
	if (!IsValid(NewActor))
	{
		UE_LOG(LogCore, Error, TEXT("New actor reference not found"));
		return false;
	}

	ActiveActors.Emplace((uint32)InstanceId, NewActor);
	UE_LOG(LogCore, Verbose, TEXT("Actor added to registry"));
	return true;
}

bool UActorRegistry::RegisterActorByName(int InstanceId, const FString& ActorTypeName, FVector Position, FRotator Rotation, bool bDestroyExisting)
{
	if (ActiveActors.Contains((uint32)InstanceId))
	{
		if (!bDestroyExisting)
		{
			UE_LOG(LogCore, Warning, TEXT("Instance at ID %d already in registry, could not register."), InstanceId);
			return false;
		}
		else
		{
			UE_LOG(LogCore, Warning, TEXT("Instance at ID %d already in registry, destroying the existing actor to spawn new one."), InstanceId);
			RemoveActor(InstanceId);
		}
	}

	AActor* NewActor = SpawnActorByName(ActorTypeName, Position, Rotation);
	if (!IsValid(NewActor))
	{
		UE_LOG(LogCore, Error, TEXT("New actor reference not found"));
		return false;
	}

	ActiveActors.Emplace((uint32)InstanceId, NewActor);
	UE_LOG(LogCore, Verbose, TEXT("Actor added to registry"));
	return true;
}

uint32 UActorRegistry::RegisterActorById(int ActorTypeId, FVector Position, FRotator Rotation)
{
	uint32 NewId = GCurrentId;
	bool bIdAlreadyRegistered = ActiveActors.Contains(NewId);
	while (bIdAlreadyRegistered)
	{
		++GCurrentId;
		NewId = GCurrentId;
		bIdAlreadyRegistered = ActiveActors.Contains(NewId);
	}
	RegisterActorById(NewId, ActorTypeId, Position, Rotation);
	return NewId;
}

uint32 UActorRegistry::RegisterActorByName(const FString& ActorTypeName, FVector Position, FRotator Rotation)
{
	uint32 NewId = GCurrentId;
	bool bIdAlreadyRegistered = ActiveActors.Contains(NewId);
	while (bIdAlreadyRegistered)
	{
		++GCurrentId;
		NewId = GCurrentId;
		bIdAlreadyRegistered = ActiveActors.Contains(NewId);
	}
	RegisterActorByName(NewId, ActorTypeName, Position, Rotation);
	return NewId;
}

AActor* UActorRegistry::GetActor(int InstanceId)
{
	AActor** Actor = ActiveActors.Find((uint32)InstanceId);
	if (Actor != nullptr)
	{
		return *Actor;
	}
	else
	{
		return nullptr;
	}
}

bool UActorRegistry::RemoveActor(int InstanceId)
{
	AActor* Actor = GetActor((uint32)InstanceId);
	if (!IsValid(Actor))
	{
		UE_LOG(LogCore, Warning, TEXT("actor not found in registry"));
		return false;
	}
	Actor->Destroy();
	ActiveActors.Remove((uint32)InstanceId);
	UE_LOG(LogCore, Verbose, TEXT("Actor successfully removed"));
	return true;
}

AActor* UActorRegistry::SpawnActorById(uint32 ActorTypeId, FVector Location, FRotator Rotation)
{
	if (!IsValid(WorldReference))
	{
		UE_LOG(LogCore, Error, TEXT("World reference not set"));
		return nullptr;
	}

	UClass* ActorClass = FindActorClassById(ActorTypeId).LoadSynchronous();
	if (!IsValid(ActorClass))
	{
		UE_LOG(LogCore, Error, TEXT("Actor type not found"));
		return nullptr;
	}

	AActor* SpawnedActor = WorldReference->SpawnActor<AActor>(ActorClass, Location, Rotation);
	if (!IsValid(SpawnedActor))
	{
		UE_LOG(LogCore, Error, TEXT("Actor failed to spawn"));
	}
	else
	{
		UE_LOG(LogCore, Verbose, TEXT("Actor successfully spawned"));
	}

	return SpawnedActor;
}

AActor* UActorRegistry::SpawnActorByName(FString ActorTypeName, FVector Location, FRotator Rotation)
{
	if (!IsValid(WorldReference))
	{
		UE_LOG(LogCore, Error, TEXT("World reference not set"));
		return nullptr;
	}

	UClass* ActorClass = FindActorClassByName(ActorTypeName).LoadSynchronous();
	if (!IsValid(ActorClass))
	{
		UE_LOG(LogCore, Error, TEXT("Actor type not found"));
		return nullptr;
	}

	AActor* SpawnedActor = WorldReference->SpawnActor<AActor>(ActorClass, Location, Rotation);
	if (!IsValid(SpawnedActor))
	{
		UE_LOG(LogCore, Error, TEXT("Actor failed to spawn"));
	}
	else
	{
		UE_LOG(LogCore, Verbose, TEXT("Actor successfully spawned"));
	}

	return SpawnedActor;
}

TSoftClassPtr<AActor> UActorRegistry::FindActorClassById(uint32 ActorTypeId)
{
	for (const FSpawnableActorInfo& ActorInfo : SpawnableActorTypes)
	{
		if (ActorInfo.ActorTypeId == ActorTypeId)
		{
			UE_LOG(LogCore, Verbose, TEXT("Actor type successfully retrieved"));
			return ActorInfo.ActorClass;
		}
	}
	return nullptr;
}

TSoftClassPtr<AActor> UActorRegistry::FindActorClassByName(FString ActorTypeName)
{
	for (const FSpawnableActorInfo& ActorInfo : SpawnableActorTypes)
	{
		if (ActorInfo.ActorTypeName == ActorTypeName)
		{
			UE_LOG(LogCore, Verbose, TEXT("Actor type successfully retrieved"));
			return ActorInfo.ActorClass;
		}
	}
	return nullptr;
}
