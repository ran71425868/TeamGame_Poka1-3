#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "USkillDataAsset.generated.h"

class UTexture2D; // 画像を扱うための前方宣言を追加

UENUM(BlueprintType)
enum class ESkillCategory : uint8
{
    PlayerBuff,
    CookingBoost,
    CustomerService,
    ScoreMultiplier
};

/**
 * 個別のスキルデータを保持するアセット
 */
UCLASS(BlueprintType)
class POKAPOKAECC_API UUSkillDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    // エディタのコンテンツブラウザで見分けやすくするためのID（任意）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    FName SkillID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    FString SkillName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill", meta = (MultiLine = true))
    FString Description;

    // スキルのアイコン画像（後からエディタ上で写真をアサインするだけで動くようになります）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    UTexture2D* SkillIcon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    ESkillCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    float ModifierValue;

    // --- ショップ用の設定 ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Shop")
    int32 Price = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Shop")
    bool bIsShopExclusive = false;

    // DataAssetをAssetManagerで管理する場合に推奨されるオーバーライド
    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId("SkillData", GetFName());
    }
	
};
