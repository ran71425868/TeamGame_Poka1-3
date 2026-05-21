#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "USkillDataAsset.h"
#include "Components/Button.h"
#include "ChefShopWidget.generated.h"

class UUniformGridPanel;
class UChefShopItemWidget;

UCLASS()
class POKAPOKAECC_API UChefShopWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Shop UI")
    void InitShopItems();

protected:
    virtual void NativeConstruct() override;

    // ★エディタ側のボタン名（例えば NextButton など）と変数名を完全に一致させる
    UPROPERTY(meta = (BindWidget))
    UButton* LeaveButton;

    UFUNCTION()
    void OnLeaveButtonClicked();


    // UMGで "ItemGrid" と名付けたUniformGridPanelと紐づく
    UPROPERTY(meta = (BindWidget))
    UUniformGridPanel* ItemGrid;

    // 生成する緑の枠のクラス（エディタで設定）
    UPROPERTY(EditAnywhere, Category = "Shop UI")
    TSubclassOf<UChefShopItemWidget> ItemWidgetClass;
};