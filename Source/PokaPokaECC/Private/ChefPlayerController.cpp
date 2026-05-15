// Fill out your copyright notice in the Description page of Project Settings.


#include "ChefPlayerController.h"
#include "Kismet/KismetArrayLibrary.h" // シャッフル機能用
#include "Engine/Engine.h"

AChefPlayerController::AChefPlayerController()
{
    bIsSkillSelectionOpen = false;
}

void AChefPlayerController::OpenSkillMenu()
{
    // 既に開いている、またはスキルリストが空の場合は何もしない
    if (bIsSkillSelectionOpen || MasterSkillList.Num() == 0) return;

    bIsSkillSelectionOpen = true;

    // ゲームを一時停止
    SetPause(true);

    // マウスカーソルを表示し、入力モードをUI専用に変更
    bShowMouseCursor = true;
    FInputModeUIOnly InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);

    // スキルを抽選
    TArray<UUSkillDataAsset*> RandomSkills = GetRandomSkills();

    // BlueprintへUI表示を指示し、抽選したスキルを渡す
    ReceiveShowSkillSelection(RandomSkills);
}

void AChefPlayerController::SelectSkill(UUSkillDataAsset* SelectedSkill)
{
    if (!SelectedSkill) return;

    // デバッグログ出力
    FString LogMessage = FString::Printf(TEXT("【%s】を取得！ 効果量: %.2f"),
        *SelectedSkill->SkillName, SelectedSkill->ModifierValue);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, LogMessage);
    }
    UE_LOG(LogTemp, Warning, TEXT("%s"), *LogMessage);

    // ----------------------------------------------------
    // TODO: ここで実際にプレイヤーやゲームステートに値を反映させる
    // ----------------------------------------------------

    // UIを閉じる指示をBlueprintへ送る
    ReceiveHideSkillSelection();

    // ゲームのポーズを解除
    SetPause(false);

    // 入力モードをゲームプレイに戻す
    bShowMouseCursor = false;
    SetInputMode(FInputModeGameOnly());

    bIsSkillSelectionOpen = false;
}

TArray<UUSkillDataAsset*> AChefPlayerController::GetRandomSkills()
{
    TArray<UUSkillDataAsset*> Result;
    TArray<UUSkillDataAsset*> TempPool = MasterSkillList;

    // リストをシャッフル
    for (int32 i = 0; i < TempPool.Num(); ++i)
    {
        int32 Index = FMath::RandRange(i, TempPool.Num() - 1);
        TempPool.Swap(i, Index);
    }

    // 上位最大4つを抽出
    for (int32 i = 0; i < FMath::Min(4, TempPool.Num()); ++i)
    {
        Result.Add(TempPool[i]);
    }

    return Result;
}
