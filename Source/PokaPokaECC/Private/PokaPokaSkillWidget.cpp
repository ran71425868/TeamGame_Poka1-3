#include "PokaPokaSkillWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/PanelWidget.h" 
#include "Components/Image.h"
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

	UpdateSkillDisplay();
}

void UPokaPokaSkillWidget::UpdateSkillDisplay()
{
	UUSkillDataAsset* SelectedSkill = nullptr;
	if (AvailableSkills.IsValidIndex(CurrentIndex))
	{
		SelectedSkill = AvailableSkills[CurrentIndex];
	}

	// 診断用：実行されているか画面に出す
	if (GEngine)
	{
		FString DebugText = SelectedSkill ? SelectedSkill->SkillName : TEXT("No Data");
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("UI Update: %s"), *DebugText));
	}

	if (SelectedSkill)
	{
		// もしここで名前が更新されなければ、UIパーツと名前が一致していません
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
		}
	}

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

void UPokaPokaSkillWidget::UpdateResultFocusVisuals()
{
	if (TitleBtn)
	{
		TitleBtn->SetBackgroundColor(ResultSelectedIndex == 0 ? FLinearColor::Yellow : FLinearColor::White);
	}
	if (NextBtn)
	{
		NextBtn->SetBackgroundColor(ResultSelectedIndex == 1 ? FLinearColor::Yellow : FLinearColor::White);
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
				UpdateSkillDisplay();
			}
		}
		else if (CurrentMenuState == ESkillMenuState::ResultSelect)
		{
			if (ResultSelectedIndex > 0)
			{
				ResultSelectedIndex--;
				OnResultSelectionChanged(ResultSelectedIndex);
				UpdateResultFocusVisuals();
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
				UpdateSkillDisplay();
			}
		}
		else if (CurrentMenuState == ESkillMenuState::ResultSelect)
		{
			if (ResultSelectedIndex < 1)
			{
				ResultSelectedIndex++;
				OnResultSelectionChanged(ResultSelectedIndex);
				UpdateResultFocusVisuals();
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
			CurrentSelectedSkill = AvailableSkills[CurrentIndex];
			ShowResultScreen();
		}
	}
	else if (CurrentMenuState == ESkillMenuState::ResultSelect)
	{
		if (ResultSelectedIndex == 0)
		{
			OnTitleClicked();
		}
		else
		{
			OnNextClicked();
		}
	}
}

void UPokaPokaSkillWidget::ShowResultScreen()
{
	if (EndGameBox)
	{
		EndGameBox->SetVisibility(ESlateVisibility::Visible);
	}

	if (SkillContainer) SkillContainer->SetVisibility(ESlateVisibility::Collapsed);
	if (Text_SelectedSkillName) Text_SelectedSkillName->SetVisibility(ESlateVisibility::Collapsed);
	if (Text_SelectedSkillDescription) Text_SelectedSkillDescription->SetVisibility(ESlateVisibility::Collapsed);
	if (Image_SelectedSkillIcon) Image_SelectedSkillIcon->SetVisibility(ESlateVisibility::Collapsed);

	CurrentMenuState = ESkillMenuState::ResultSelect;
	ResultSelectedIndex = 1;

	OnResultSelectionChanged(ResultSelectedIndex);
	UpdateResultFocusVisuals();
}

void UPokaPokaSkillWidget::OnNextClicked()
{
	APokaPokaECCPlayerController* PC = Cast<APokaPokaECCPlayerController>(GetOwningPlayer());
	if (PC)
	{
		PC->SelectSkill(CurrentSelectedSkill);

		// キャラクターを再び動かせるようにする処理
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}

	// ポーズ画面（一時停止）の解除
	UGameplayStatics::SetGamePaused(this, false);

	// Day1〜Day7、そしてリザルト画面への遷移ロジック
	FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);

	if (CurrentLevelName.Equals("Day1", ESearchCase::IgnoreCase))
	{
		UGameplayStatics::OpenLevel(this, FName("Day2"));
	}
	else if (CurrentLevelName.Equals("Day2", ESearchCase::IgnoreCase))
	{
		UGameplayStatics::OpenLevel(this, FName("Day3"));
	}
	else if (CurrentLevelName.Equals("Day3", ESearchCase::IgnoreCase))
	{
		UGameplayStatics::OpenLevel(this, FName("Day4"));
	}
	else if (CurrentLevelName.Equals("Day4", ESearchCase::IgnoreCase))
	{
		UGameplayStatics::OpenLevel(this, FName("Day5"));
	}
	else if (CurrentLevelName.Equals("Day5", ESearchCase::IgnoreCase))
	{
		UGameplayStatics::OpenLevel(this, FName("Day6"));
	}
	else if (CurrentLevelName.Equals("Day6", ESearchCase::IgnoreCase))
	{
		UGameplayStatics::OpenLevel(this, FName("Day7"));
	}
	else if (CurrentLevelName.Equals("Day7", ESearchCase::IgnoreCase))
	{
		// ※ リザルト画面のレベル名が「ResultMap」ではない場合は、以下を変更してください
		UGameplayStatics::OpenLevel(this, FName("Result"));
	}
	else
	{
		// それ以外の場合はUIを閉じてそのまま再開
		RemoveFromParent();
	}
}

void UPokaPokaSkillWidget::OnTitleClicked()
{
	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, FName("MainMenu"));
}