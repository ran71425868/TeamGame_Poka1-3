#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "USkillDataAsset.h"
#include "PokaPokaSkillSlotWidget.generated.h"

class UButton;

UCLASS()
class POKAPOKAECC_API UPokaPokaSkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// メインUIからカードを生成した時に呼び出される初期化関数
	void InitializeSlot(UUSkillDataAsset* InSkillData, int32 InIndex);

	// ★追加：メインUIのC++側から、このカードが選択中かどうかを切り替える関数
	void SetCardFocused(bool bFocused);

	// BP側からいつでもデータを確認できるようにプロパティ化
	UPROPERTY(BlueprintReadOnly, Category = "Skill Slot")
	UUSkillDataAsset* SkillData;

	UPROPERTY(BlueprintReadOnly, Category = "Skill Slot")
	int32 SlotIndex;

protected:
	virtual void NativeConstruct() override;

	// カードが生成された後に、BP側で「スキル名」や「アイコン」のテキストを書き換えるための合図
	UFUNCTION(BlueprintImplementableEvent, Category = "Skill Slot")
	void OnSlotInitialized();

	// ★追加：フォーカス状態が変わった時に、このカード単体の見た目（色や枠）をBP側で変えるためのイベント
	UFUNCTION(BlueprintImplementableEvent, Category = "Skill Slot")
	void OnFocusChanged(bool bNewFocused);

	// --- ブループリントと紐づくUIパーツ ---
	UPROPERTY(meta = (BindWidget))
	UButton* CardBtn;

	// ボタンが直接クリックされた時のC++関数
	UFUNCTION()
	void OnCardClicked();
};