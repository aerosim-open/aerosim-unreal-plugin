// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/AerosimActor.h"

#include "Sensor.generated.h"

UCLASS(Blueprintable)
class AEROSIMCONNECTOR_API ASensor : public AAerosimActor
{
	GENERATED_BODY()

public:
	ASensor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

};
