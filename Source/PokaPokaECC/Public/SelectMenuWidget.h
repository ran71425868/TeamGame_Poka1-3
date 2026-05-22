#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SelectMenuWidget.generated.h"

class UButton;

/**
 * セレクト画面用のウィジェットクラス
 * ウィジェット自身で入力モードとフォーカス処理を完結させます
 */
UCLASS()
class POKAPOKAECC_API USelectMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// ウィジェットが作成・画面に追加されたときに呼ばれる処理
	virtual void NativeConstruct() override;

	// 毎フレーム呼ばれる処理（フォーカスの見た目更新）
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ボタンの見た目を更新するヘルパー関数
	void UpdateButtonAppearance(UButton* Button);

	// --- エンターキーやクリックで決定（選択）された時に呼ばれる関数 ---
	UFUNCTION()
	void OnPlayClicked();

	UFUNCTION()
	void OnTutorialClicked();

	UFUNCTION()
	void OnMainClicked();

public:
	// BPW_Select内のボタンと紐付ける
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Play;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Tutorial;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Main;
};