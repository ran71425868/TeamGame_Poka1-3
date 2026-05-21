#include "PokaPokaSkillWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/PanelWidget.h" 
#include "Components/Image.h" // ★追加
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

	if (EndGameBox)
	{
		EndGameBox->SetVisibility(ESlateVisibility::Collapsed);
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

	// ★追加：背景を暗くする
	if (BackgroundDim)
	{
		BackgroundDim->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.65f));
	}

	CurrentMenuState = ESkillMenuState::SelectingSkill;
	OnSkillSelectionChanged(CurrentIndex);
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

	// ★追加：生成完了後に、テキストや画像、フォーカスを一括更新して初期表示を整える
	UpdateSkillDisplay();
}

// ★追加：選択中のスキル情報（テキスト、画像、フォーカス）をすべて更新する統合関数
void UPokaPokaSkillWidget::UpdateSkillDisplay()
{
	UUSkillDataAsset* SelectedSkill = nullptr;
	if (AvailableSkills.IsValidIndex(CurrentIndex))
	{
		SelectedSkill = AvailableSkills[CurrentIndex];
	}

	if (SelectedSkill)
	{
		if (Text_SelectedSkillName)
		{
			Text_SelectedSkillName->SetText(FText::FromString(SelectedSkill->SkillName));
			Text_SelectedSkillName->SetVisibility(ESlateVisibility::Visible);
		}
		if (Text_SelectedSkillDescription)
		{
			Text_SelectedSkillDescription->SetText(FText::FromString(SelectedSkill->Description));
			Text_SelectedSkillDescription->SetVisibility(ESlateVisibility::Visible);
		}
		if (Image_SelectedSkillIcon)
		{
			if (SelectedSkill->SkillIcon)
			{
				Image_SelectedSkillIcon->SetBrushFromTexture(SelectedSkill->SkillIcon);
				Image_SelectedSkillIcon->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				Image_SelectedSkillIcon->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}

	// 最後にカードの光る演出（フォーカス）を更新
	UpdateCardFocusVisuals();
}

void UPokaPokaSkillWidget::UpdateCardFocusVisuals()
{
	if (!SkillContainer) return;

	int32 ChildCount = SkillContainer->GetChildrenCount();
	for (int32 i = 0; i < ChildCount; ++i)
	{
		if (UPokaPokaSkillSlotWidget* SlotWidget = Cast<UPokaPokaSkillSlotWidget>(SkillContainer->GetChildAt(i)))
		{
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
				UpdateSkillDisplay(); // ★修正：移動した時に画像・テキストも更新する
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
				UpdateSkillDisplay(); // ★修正：移動した時に画像・テキストも更新する
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
			// ★修正：決定したスキルをここで確実に記憶しておく
			CurrentSelectedSkill = AvailableSkills[CurrentIndex];
			ShowResultScreen();
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