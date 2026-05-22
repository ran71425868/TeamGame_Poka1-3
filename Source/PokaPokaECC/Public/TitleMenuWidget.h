#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleMenuWidget.generated.h"

UCLASS()
class POKAPOKAECC_API UTitleMenuWidget : public UUserWidget
{
	GENERATED_BODY()

	// 【変更】UIの変数化、クリック処理、フォーカス設定をすべてブループリント側で行うため、
	// C++側のコードはすべてコメントアウトして無効化します。


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
	class UButton* StartButton;


	UPROPERTY(meta = (BindWidget))
	class UButton* ExitButton;

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