#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "AerosimWidget.generated.h"

class UUserWidget;

UCLASS()
class AEROSIMCONNECTOR_API UAerosimWidget : public UUserWidget
{
	GENERATED_BODY()
private:
	int AssociatedAerosimActorID;

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Update")
	void UpdateWidget();

	UFUNCTION(BlueprintCallable, Category = "Identification")
	void SetId(int InId) { AssociatedAerosimActorID = InId; }

	UFUNCTION(BlueprintCallable, Category = "Identification")
	int GetId() { return AssociatedAerosimActorID; }
};
