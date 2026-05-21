#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleMenuWidget.generated.h"

class UButton;

UCLASS()
class POKAPOKAECC_API UTitleMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetFirstFocus();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UButton* StartButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Transition")
	FName NextLevelName = FName("testmap");

	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void OnExitClicked();
};