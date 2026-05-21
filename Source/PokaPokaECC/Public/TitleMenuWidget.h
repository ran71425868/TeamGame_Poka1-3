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

/*
public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetFirstFocus();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	class UButton* StartButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ExitButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Transition")
	FName NextLevelName = FName("testmap");

	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void OnExitClicked();
*/
};