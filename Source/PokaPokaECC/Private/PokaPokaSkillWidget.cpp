#include "PokaPokaSkillWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/PanelWidget.h" 
#include "PokaPokaSkillSlotWidget.h"
#include "PokaPokaECCPlayerController.h"
#include "Input/Reply.h"
#include "Kismet/GameplayStatics.h"

UPokaPokaSkillWidget::UPokaPokaSkillWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsFocusable = true;
}

void UPokaPokaSkillWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PopupBorder)
	{
		PopupBorder->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (EndGameBox)
	{
		EndGameBox->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (ConfirmBtn)
	{
		ConfirmBtn->IsFocusable = false;
		ConfirmBtn->OnClicked.AddDynamic(this, &UPokaPokaSkillWidget::OnConfirmClicked);
	}
	if (CancelBtn)
	{
		CancelBtn->IsFocusable = false;
		CancelBtn->OnClicked.AddDynamic(this, &UPokaPokaSkillWidget::OnCancelClicked);
	}
	if (NextBtn)
	{
		NextBtn->IsFocusable = false;
		NextBtn->OnClicked.AddDynamic(this, &UPokaPokaSkillWidget::OnNextClicked);
	}
	if (TitleBtn)
	{
		TitleBtn->IsFocusable = false;
		TitleBtn->OnClicked.AddDynamic(this, &UPokaPokaSkillWidget::OnTitleClicked);
	}

	CurrentMenuState = ESkillMenuState::SelectingSkill;
	OnSkillSelectionChanged(CurrentIndex);
	// 初期フォーカスのビジュアルを反映
	UpdateCardFocusVisuals();
}

void UPokaPokaSkillWidget::GenerateUI()
{
	if (!SkillContainer || !SkillSlotClass || AvailableSkills.Num() == 0) return;

	SkillContainer->ClearChildren();

	for (int32 i = 0; i < AvailableSkills.Num(); ++i)
	{
		if (UUSkillDataAsset* SkillData = AvailableSkills[i])
		{
			UPokaPokaSkillSlotWidget* NewSlot = CreateWidget<UPokaPokaSkillSlotWidget>(this, SkillSlotClass);
			if (NewSlot)
			{
				NewSlot->InitializeSlot(SkillData, i);
				SkillContainer->AddChild(NewSlot);
			}
		}
	}

	// ★追加：生成完了後に、一番最初のカード（CurrentIndex=0）をC++側で光らせる
	UpdateCardFocusVisuals();
}

void UPokaPokaSkillWidget::UpdateCardFocusVisuals()
{
	// ★追加：SkillContainer内の子ウィジェットをループしてフォーカスを直接切り替える
	if (!SkillContainer) return;

	int32 ChildCount = SkillContainer->GetChildrenCount();
	for (int32 i = 0; i < ChildCount; ++i)
	{
		if (UPokaPokaSkillSlotWidget* SlotWidget = Cast<UPokaPokaSkillSlotWidget>(SkillContainer->GetChildAt(i)))
		{
			// 現在のループ番号が、選択中のCurrentIndexと一致していれば True、それ以外は False を渡す
			SlotWidget->SetCardFocused(i == CurrentIndex);
		}
	}
}

FReply UPokaPokaSkillWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	FKey PressedKey = InKeyEvent.GetKey();

	if (PressedKey == EKeys::A || PressedKey == EKeys::Left)
	{
		if (CurrentMenuState == ESkillMenuState::SelectingSkill)
		{
			if (CurrentIndex > 0)
			{
				CurrentIndex--;
				OnSkillSelectionChanged(CurrentIndex);
				// ★修正：インデックス変更時にC++から直接カードの見た目を一括更新する
				UpdateCardFocusVisuals();
			}
		}
		else if (CurrentMenuState == ESkillMenuState::PopupConfirm)
		{
			if (PopupSelectedIndex > 0)
			{
				PopupSelectedIndex--;
				OnPopupSelectionChanged(PopupSelectedIndex);
			}
		}
		else if (CurrentMenuState == ESkillMenuState::ResultSelect)
		{
			if (ResultSelectedIndex > 0)
			{
				ResultSelectedIndex--;
				OnResultSelectionChanged(ResultSelectedIndex);
			}
		}
		return FReply::Handled();
	}
	else if (PressedKey == EKeys::D || PressedKey == EKeys::Right)
	{
		if (CurrentMenuState == ESkillMenuState::SelectingSkill)
		{
			if (CurrentIndex < AvailableSkills.Num() - 1)
			{
				CurrentIndex++;
				OnSkillSelectionChanged(CurrentIndex);
				// ★修正：インデックス変更時にC++から直接カードの見た目を一括更新する
				UpdateCardFocusVisuals();
			}
		}
		else if (CurrentMenuState == ESkillMenuState::PopupConfirm)
		{
			if (PopupSelectedIndex < 1)
			{
				PopupSelectedIndex++;
				OnPopupSelectionChanged(PopupSelectedIndex);
			}
		}
		else if (CurrentMenuState == ESkillMenuState::ResultSelect)
		{
			if (ResultSelectedIndex < 1)
			{
				ResultSelectedIndex++;
				OnResultSelectionChanged(ResultSelectedIndex);
			}
		}
		return FReply::Handled();
	}
	else if (PressedKey == EKeys::Enter)
	{
		OnEnterKeyPressed();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPokaPokaSkillWidget::OnEnterKeyPressed()
{
	if (CurrentMenuState == ESkillMenuState::SelectingSkill)
	{
		if (AvailableSkills.IsValidIndex(CurrentIndex))
		{
			ShowSkillPopup(AvailableSkills[CurrentIndex]);
		}
	}
	else if (CurrentMenuState == ESkillMenuState::PopupConfirm)
	{
		if (PopupSelectedIndex == 0)
		{
			OnConfirmClicked();
		}
		else
		{
			OnCancelClicked();
		}
	}
	else if (CurrentMenuState == ESkillMenuState::ResultSelect)
	{
		if (ResultSelectedIndex == 0)
		{
			OnNextClicked();
		}
		else
		{
			OnTitleClicked();
		}
	}
}

void UPokaPokaSkillWidget::ShowSkillPopup(UUSkillDataAsset* SelectedSkill)
{
	if (!SelectedSkill) return;

	CurrentSelectedSkill = SelectedSkill;

	if (PopupNameText)
	{
		PopupNameText->SetText(FText::FromString(SelectedSkill->SkillName));
	}
	if (PopupDescText)
	{
		PopupDescText->SetText(FText::FromString(SelectedSkill->Description));
	}

	if (PopupBorder)
	{
		PopupBorder->SetVisibility(ESlateVisibility::Visible);
	}

	CurrentMenuState = ESkillMenuState::PopupConfirm;
	PopupSelectedIndex = 0;
	OnPopupSelectionChanged(PopupSelectedIndex);
}

void UPokaPokaSkillWidget::OnConfirmClicked()
{
	if (PopupBorder)
	{
		PopupBorder->SetVisibility(ESlateVisibility::Collapsed);
	}
	ShowResultScreen();
}

void UPokaPokaSkillWidget::OnCancelClicked()
{
	if (PopupBorder)
	{
		PopupBorder->SetVisibility(ESlateVisibility::Collapsed);
	}

	CurrentMenuState = ESkillMenuState::SelectingSkill;
	OnSkillSelectionChanged(CurrentIndex);
	// ★修正：キャンセルで戻ってきた際にも見た目を再同期
	UpdateCardFocusVisuals();
}

void UPokaPokaSkillWidget::ShowResultScreen()
{
	if (EndGameBox)
	{
		EndGameBox->SetVisibility(ESlateVisibility::Visible);
	}

	CurrentMenuState = ESkillMenuState::ResultSelect;
	ResultSelectedIndex = 0;
	OnResultSelectionChanged(ResultSelectedIndex);
}

void UPokaPokaSkillWidget::OnNextClicked()
{
	if (APokaPokaECCPlayerController* PC = Cast<APokaPokaECCPlayerController>(GetOwningPlayer()))
	{
		PC->SelectSkill(CurrentSelectedSkill);
	}
}

void UPokaPokaSkillWidget::OnTitleClicked()
{
	UGameplayStatics::OpenLevel(this, FName("TitleMap"));
}