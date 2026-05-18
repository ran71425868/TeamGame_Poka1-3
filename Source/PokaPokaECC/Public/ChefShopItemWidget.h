#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "USkillDataAsset.h"
#include "ChefShopItemWidget.generated.h"

UCLASS()
class POKAPOKAECC_API UChefShopItemWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Shop UI")
    void InitItem(UUSkillDataAsset* InSkill);

    UFUNCTION(BlueprintCallable, Category = "Shop UI")
    void OnBuyClicked();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Shop UI")
    UUSkillDataAsset* SkillData;

    // BPに「テキストやアイコンを更新してね」と伝えるイベント
    UFUNCTION(BlueprintImplementableEvent, Category = "Shop UI")
    void OnItemInitialized();
};