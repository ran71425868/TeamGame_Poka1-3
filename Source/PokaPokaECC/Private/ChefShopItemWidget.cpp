#include "ChefShopItemWidget.h"
#include "PokaPokaECCGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UChefShopItemWidget::InitItem(UUSkillDataAsset* InSkill)
{
    SkillData = InSkill;
    OnItemInitialized(); // BPのイベントを呼ぶ
}

void UChefShopItemWidget::OnBuyClicked()
{
    if (!SkillData) return;

    // GameInstanceを取得して購入処理を実行
    UPokaPokaECCGameInstance* GameInst = Cast<UPokaPokaECCGameInstance>(UGameplayStatics::GetGameInstance(this));
    if (GameInst)
    {
        if (GameInst->BuySkill(SkillData))
        {
            // 購入成功時の処理（例：このボタンを非表示にする等、必要に応じてBP側で実装）
        }
    }
}