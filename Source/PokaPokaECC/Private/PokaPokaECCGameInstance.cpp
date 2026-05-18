#include "PokaPokaECCGameInstance.h"

TArray<UUSkillDataAsset*> UPokaPokaECCGameInstance::GetShopSkills()
{
    TArray<UUSkillDataAsset*> Result;
    TArray<UUSkillDataAsset*> ShopPool;

    // 今回は例として「ショップ専用スキル」だけをプールに入れる
    for (UUSkillDataAsset* Skill : MasterSkillList)
    {
        if (Skill && Skill->bIsShopExclusive)
        {
            ShopPool.Add(Skill);
        }
    }

    // シャッフル
    for (int32 i = 0; i < ShopPool.Num(); ++i)
    {
        int32 Index = FMath::RandRange(i, ShopPool.Num() - 1);
        ShopPool.Swap(i, Index);
    }

    // 最大8個を抽出
    for (int32 i = 0; i < FMath::Min(8, ShopPool.Num()); ++i)
    {
        Result.Add(ShopPool[i]);
    }

    return Result;
}

bool UPokaPokaECCGameInstance::BuySkill(UUSkillDataAsset* SkillToBuy)
{
    if (!SkillToBuy) return false;

    if (CurrentMoney >= SkillToBuy->Price)
    {
        CurrentMoney -= SkillToBuy->Price;
        AcquiredSkills.Add(SkillToBuy);

        UE_LOG(LogTemp, Warning, TEXT("Purchase Successful: %s"), *SkillToBuy->SkillName);
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("Not enough money!"));
    return false;
}