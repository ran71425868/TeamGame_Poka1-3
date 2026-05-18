#include "ChefShopWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "ChefShopItemWidget.h"
#include "PokaPokaECCGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UChefShopWidget::InitShopItems()
{
    if (!ItemGrid || !ItemWidgetClass) return;

    ItemGrid->ClearChildren();

    UPokaPokaECCGameInstance* GameInst = Cast<UPokaPokaECCGameInstance>(UGameplayStatics::GetGameInstance(this));
    if (!GameInst) return;

    // GameInstanceから8個のスキルを取ってくる
    TArray<UUSkillDataAsset*> ShopSkills = GameInst->GetShopSkills();

    // 8個のアイテムを横4×縦2で配置する
    for (int32 i = 0; i < ShopSkills.Num(); ++i)
    {
        UUSkillDataAsset* Skill = ShopSkills[i];
        if (!Skill) continue;

        UChefShopItemWidget* ItemWidget = CreateWidget<UChefShopItemWidget>(this, ItemWidgetClass);
        if (ItemWidget)
        {
            ItemWidget->InitItem(Skill);

            int32 Row = i / 4;
            int32 Col = i % 4;

            UUniformGridSlot* GridSlot = ItemGrid->AddChildToUniformGrid(ItemWidget, Row, Col);
            if (GridSlot)
            {
                GridSlot->SetHorizontalAlignment(HAlign_Fill);
                GridSlot->SetVerticalAlignment(VAlign_Fill);
            }
        }
    }
}