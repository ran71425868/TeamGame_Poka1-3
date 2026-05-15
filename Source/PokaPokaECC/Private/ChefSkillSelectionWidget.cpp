#include "ChefSkillSelectionWidget.h"
#include "ChefPlayerController.h" // 決定時にSelectSkillを呼ぶために必要
#include "Input/Reply.h"

// コンストラクタの実装
UChefSkillSelectionWidget::UChefSkillSelectionWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // A/Dキーなどの入力をUIで受け取るための必須設定
    bIsFocusable = true;
}

void UChefSkillSelectionWidget::InitSkills(const TArray<UUSkillDataAsset*>& InSkills)
{
    AvailableSkills = InSkills;
    CurrentSelectedIndex = 0; // 初期選択を一番左(インデックス0)にする

    // 初期状態の見た目をBPに更新させる
    OnSelectionChanged(CurrentSelectedIndex);
}

FReply UChefSkillSelectionWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    FKey PressedKey = InKeyEvent.GetKey();

    // Aキー または 左矢印 で左へ移動
    if (PressedKey == EKeys::A || PressedKey == EKeys::Left)
    {
        if (CurrentSelectedIndex > 0)
        {
            CurrentSelectedIndex--;
            OnSelectionChanged(CurrentSelectedIndex); // 見た目を更新
        }
        return FReply::Handled(); // 入力をここで消費する
    }
    // Dキー または 右矢印 で右へ移動
    else if (PressedKey == EKeys::D || PressedKey == EKeys::Right)
    {
        if (CurrentSelectedIndex < AvailableSkills.Num() - 1)
        {
            CurrentSelectedIndex++;
            OnSelectionChanged(CurrentSelectedIndex); // 見た目を更新
        }
        return FReply::Handled();
    }
    // Enterキー で決定
    else if (PressedKey == EKeys::Enter)
    {
        // 現在のインデックスが有効かチェック
        if (AvailableSkills.IsValidIndex(CurrentSelectedIndex))
        {
            // PlayerControllerを取得してスキル決定処理を呼び出す
            if (AChefPlayerController* PC = Cast<AChefPlayerController>(GetOwningPlayer()))
            {
                PC->SelectSkill(AvailableSkills[CurrentSelectedIndex]);
            }
        }
        return FReply::Handled();
    }

    // 上記（A, D, Enter, 左右矢印）以外のキー入力は、通常の処理へ流す
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}