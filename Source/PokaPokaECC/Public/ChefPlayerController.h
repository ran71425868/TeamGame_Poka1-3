// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "USkillDataAsset.h" 
#include "ChefPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class POKAPOKAECC_API AChefPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AChefPlayerController();

    // エディタ上で全スキルを登録するためのリスト（スキルプール）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill System")
    TArray<UUSkillDataAsset*> MasterSkillList;

    // スキル選択画面を開く処理（BPのTabキー入力などから呼び出す）
    UFUNCTION(BlueprintCallable, Category = "Skill System")
    void OpenSkillMenu();

    // UIでスキルが選択された時に呼ばれる処理
    UFUNCTION(BlueprintCallable, Category = "Skill System")
    void SelectSkill(UUSkillDataAsset* SelectedSkill);

protected:
    // UI表示用のイベント（BP側で実装）
    UFUNCTION(BlueprintImplementableEvent, Category = "Skill System")
    void ReceiveShowSkillSelection(const TArray<UUSkillDataAsset*>& AvailableSkills);

    // UI非表示用のイベント（BP側で実装）
    UFUNCTION(BlueprintImplementableEvent, Category = "Skill System")
    void ReceiveHideSkillSelection();

private:
    bool bIsSkillSelectionOpen;

    // スキルプールからランダムに4つ抽出する関数
    TArray<UUSkillDataAsset*> GetRandomSkills();
};
