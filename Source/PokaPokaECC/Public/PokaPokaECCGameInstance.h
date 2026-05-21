#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "USkillDataAsset.h"
#include "PokaPokaECCGameInstance.generated.h"

UCLASS()
class POKAPOKAECC_API UPokaPokaECCGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    // ゲーム全体のスキルリスト（エディタで登録）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game Data")
    TArray<UUSkillDataAsset*> MasterSkillList;

    // プレイヤーが現在取得しているスキル
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Data")
    TArray<UUSkillDataAsset*> AcquiredSkills;

    // 所持金と現在の日数
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Data")
    int32 CurrentMoney = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Data")
    int32 CurrentDay = 1;

    // --- ショップ機能 ---
    UFUNCTION(BlueprintCallable, Category = "Shop System")
    TArray<UUSkillDataAsset*> GetShopSkills();

    UFUNCTION(BlueprintCallable, Category = "Shop System")
    bool BuySkill(UUSkillDataAsset* SkillToBuy);


    // ====================================================
    // --- スキル効果による永続バフパラメータ (12スキル分) ---
    // ====================================================

    // 客関連
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Data|Buffs")
    float CustomerPatienceBonus = 0.0f;          // ArtisanPatience

    // プレイヤー関連
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Data|Buffs")
    float MovementSpeedBonus = 0.0f;             // SpeedstersHaste / LightFootwork
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Data|Buffs")
    float InteractSpeedMultiplier = 1.0f;        // UltraEfficiency

    // 調理器具関連 (速度倍率・デフォルトは1.0)
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Data|Buffs")
    float FryerSpeedMultiplier = 1.0f;           // FastFryer
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Data|Buffs")
    float BurnerSpeedMultiplier = 1.0f;          // HighHeatBurner

    // スコア・システム関連
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Data|Buffs")
    float TipProbabilityBonus = 0.0f;            // CozyAtmosphere
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Data|Buffs")
    float TipAmountMultiplier = 1.0f;            // GratuityTipping
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Data|Buffs")
    float PlatingValueBonus = 0.0f;              // PlatingMastery
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Data|Buffs")
    float RecipeUpgradeProbability = 0.0f;       // RecipeInspiration
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Data|Buffs")
    float ReputationGainMultiplier = 1.0f;       // StarService

    // 特殊フラグ
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Data|Buffs")
    bool bIsForbiddenMenuUnlocked = false;       // ForbiddenFullCourse
};