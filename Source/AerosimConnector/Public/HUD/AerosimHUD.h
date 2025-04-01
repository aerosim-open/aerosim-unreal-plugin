#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AerosimHUD.generated.h"

UCLASS()
class AEROSIMCONNECTOR_API AAerosimHUD : public AHUD
{
	GENERATED_BODY()

public:
	AAerosimHUD();
	virtual void BeginPlay() override;
};