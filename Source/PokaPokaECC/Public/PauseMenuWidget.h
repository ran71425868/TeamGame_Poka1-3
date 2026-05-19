#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;
class UScrollBox;

UCLASS()
class POKAPOKAECC_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// コンストラクタを追加（キーボード入力を受け取るための設定に使用）
	UPauseMenuWidget(const FObjectInitializer& ObjectInitializer);

	// 最初にフォーカスを当てるボタンを設定する関数
	void SetFirstFocus();

protected:
	virtual void NativeConstruct() override;

	// meta = (BindWidget) をつけることで、UMG側で必ずこの名前の変数を配置するよう強制できます。
	// 共同制作時に名前の付け忘れなどを防げます。
	UPROPERTY(meta = (BindWidget))
	UButton* ResumeButton;

	UPROPERTY(meta = (BindWidget))
	UButton* RetryButton;

	UPROPERTY(meta = (BindWidget))
	UButton* TitleButton;

	// スキルリストを表示するためのスクロールボックス
	UPROPERTY(meta = (BindWidget))
	UScrollBox* SkillListContainer;

	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnRetryClicked();

	UFUNCTION()
	void OnTitleClicked();
};