#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "USkillDataAsset.h" 
#include "ChefSkillSelectionWidget.generated.h"

UCLASS()
class POKAPOKAECC_API UChefSkillSelectionWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // コンストラクタを追加
    UChefSkillSelectionWidget(const FObjectInitializer& ObjectInitializer);

    // BPからUI作成時に呼び出し、抽選されたスキルデータを渡す
    UFUNCTION(BlueprintCallable, Category = "Skill UI")
    void InitSkills(const TArray<UUSkillDataAsset*>& InSkills);

protected:
    // キーボード入力を受け取るためのC++オーバーライド関数
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    // 選択中のインデックスが変わった時にBPへ通知（見た目の更新用）
    UFUNCTION(BlueprintImplementableEvent, Category = "Skill UI")
    void OnSelectionChanged(int32 NewIndex);

    // 現在選択されているスキルのインデックス(0〜3)
    UPROPERTY(BlueprintReadOnly, Category = "Skill UI")
    int32 CurrentSelectedIndex = 0;

    // UIに表示しているスキルのリスト
    UPROPERTY(BlueprintReadOnly, Category = "Skill UI")
    TArray<UUSkillDataAsset*> AvailableSkills;
};