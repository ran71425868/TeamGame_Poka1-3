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
	
	// 【追加】新しく増やすDay1ボタン（安全のためOptionalにしています）
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Day1Button;

	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Transition")
	FName NextLevelName = FName("Tutorial");
	// 【追加】Day1用の遷移先レベル名
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Transition")
	FName Day1LevelName = FName("Day1");

	UFUNCTION()
	void OnStartClicked();

	// 【追加】Day1ボタンが押されたときの関数
	UFUNCTION()
	void OnDay1Clicked();

	UFUNCTION()
	void OnExitClicked();
};