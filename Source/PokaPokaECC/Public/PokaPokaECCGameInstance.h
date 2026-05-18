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

    // ショップに並べる8個のスキルを取得する
    UFUNCTION(BlueprintCallable, Category = "Shop System")
    TArray<UUSkillDataAsset*> GetShopSkills();

    // スキルの購入処理（成功したらtrueを返す）
    UFUNCTION(BlueprintCallable, Category = "Shop System")
    bool BuySkill(UUSkillDataAsset* SkillToBuy);
};