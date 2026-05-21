#include "PokaPokaSkillSlotWidget.h"
#include "Components/Button.h"
#include "PokaPokaSkillWidget.h"
#include "PokaPokaECCPlayerController.h"

void UPokaPokaSkillSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// ★修正：C++側でボタンのキーボードフォーカスを奪わないように設定
	if (CardBtn)
	{
		CardBtn->IsFocusable = false; // エディタの「Is Focusable」チェックを外すのと同等の処理
		CardBtn->OnClicked.AddDynamic(this, &UPokaPokaSkillSlotWidget::OnCardClicked);
	}
}

void UPokaPokaSkillSlotWidget::InitializeSlot(UUSkillDataAsset* InSkillData, int32 InIndex)
{
	SkillData = InSkillData;
	SlotIndex = InIndex;

	// C++側のデータ登録が終わったら、BP側の「見た目構築イベント」を呼び出す
	OnSlotInitialized();
}

void UPokaPokaSkillSlotWidget::SetCardFocused(bool bFocused)
{
	// ★追加：フォーカス状態の変更をBPイベント（OnFocusChanged）へそのまま伝える
	OnFocusChanged(bFocused);
}

void UPokaPokaSkillSlotWidget::OnCardClicked()
{
	// マウスでカードが直接クリックされた時、PlayerControllerを経由してポップアップを出す
	if (APokaPokaECCPlayerController* PC = Cast<APokaPokaECCPlayerController>(GetOwningPlayer()))
	{
		if (PC->SkillMenuInstance)
		{
			// メインUIの選択位置を選ばれたカードの番号に同期する
			PC->SkillMenuInstance->CurrentIndex = SlotIndex;

			// ★追加：マウスクリックされた時も、即座にC++側でカード全体のフォーカスビジュアルを同期させる
			PC->SkillMenuInstance->UpdateCardFocusVisuals();

			// ポップアップを表示
			PC->SkillMenuInstance->ShowSkillPopup(SkillData);
		}
	}
}