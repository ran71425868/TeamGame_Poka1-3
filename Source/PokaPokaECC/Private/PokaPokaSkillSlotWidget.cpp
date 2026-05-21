#include "PokaPokaSkillSlotWidget.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/TextBlock.h" 
#include "Components/Image.h"     
#include "PokaPokaSkillWidget.h"
#include "PokaPokaECCPlayerController.h"
#include "Components/VerticalBoxSlot.h" // ★追加：C++側からレイアウトスロットを制御するために必要

void UPokaPokaSkillSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CardBtn)
	{
		CardBtn->IsFocusable = false;
		CardBtn->OnClicked.AddDynamic(this, &UPokaPokaSkillSlotWidget::OnCardClicked);
	}
}

void UPokaPokaSkillSlotWidget::InitializeSlot(UUSkillDataAsset* InSkillData, int32 InIndex)
{
	SkillData = InSkillData;
	SlotIndex = InIndex;

	if (SkillData)
	{
		// --- スキル名の流し込みと配置の固定 ---
		if (Text_SkillName)
		{
			Text_SkillName->SetText(FText::FromString(SkillData->SkillName));

			// VerticalBox内にあるスロットをC++で取得し、中央寄せと余白を設定
			if (UVerticalBoxSlot* NameSlot = Cast<UVerticalBoxSlot>(Text_SkillName->Slot))
			{
				NameSlot->SetPadding(FMargin(10.0f, 12.0f, 10.0f, 4.0f));
				NameSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center); // 横方向中央揃え
			}
		}

		// --- スキル説明文の流し込みと配置の固定 ---
		if (Text_Description)
		{
			Text_Description->SetText(FText::FromString(SkillData->Description));

			if (UVerticalBoxSlot* DescSlot = Cast<UVerticalBoxSlot>(Text_Description->Slot))
			{
				DescSlot->SetPadding(FMargin(12.0f, 4.0f, 12.0f, 12.0f));
				DescSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center); // 横方向中央揃え
				DescSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
			}
		}

		// --- アイコン画像の流し込みと配置の固定 ---
		if (Image_SkillIcon)
		{
			if (SkillData->SkillIcon)
			{
				Image_SkillIcon->SetBrushFromTexture(SkillData->SkillIcon);
				Image_SkillIcon->SetVisibility(ESlateVisibility::Visible);

				if (UVerticalBoxSlot* IconSlot = Cast<UVerticalBoxSlot>(Image_SkillIcon->Slot))
				{
					IconSlot->SetPadding(FMargin(0.0f, 16.0f, 0.0f, 8.0f)); // 上部に少し大きな余白
					IconSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center); // アイコン中央揃え
				}
			}
			else
			{
				Image_SkillIcon->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}

	OnSlotInitialized();
}

void UPokaPokaSkillSlotWidget::SetCardFocused(bool bFocused)
{
	if (CardBtn)
	{
		if (bFocused)
		{
			CardBtn->SetBackgroundColor(FLinearColor::Yellow);
		}
		else
		{
			CardBtn->SetBackgroundColor(FLinearColor::White);
		}
	}
}

void UPokaPokaSkillSlotWidget::OnCardClicked()
{
	if (APokaPokaECCPlayerController* PC = Cast<APokaPokaECCPlayerController>(GetOwningPlayer()))
	{
		if (PC->SkillMenuInstance)
		{
			PC->SkillMenuInstance->CurrentIndex = SlotIndex;
			PC->SkillMenuInstance->UpdateCardFocusVisuals();
		}
	}
}