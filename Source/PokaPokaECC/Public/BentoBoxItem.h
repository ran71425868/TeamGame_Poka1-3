#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FoodItemInterface.h" // 【追加】インターフェースをインクルード
#include "BentoBoxItem.generated.h"

UCLASS()
class POKAPOKAECC_API ABentoBoxItem : public AActor, public IFoodItemInterface // 【変更】IFoodItemInterfaceを継承
{
	GENERATED_BODY()

public:
	ABentoBoxItem();

	// 1. お客さん(Bob)の受け取り判定にそのまま使い回せる変数群
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bento")
	FName ProvidedFoodTag = FName("EmptyBento"); // 最初は空弁当

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bento")
	float PriceMultiplier = 0.0f; // 具材のお金をここに足していく

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bento")
	int32 ScorePoint = 0; // 具材の評価をここに足していく

	// 2. 内部で持っている具材のリスト
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bento")
	TArray<FName> ContainedIngredients;

	// 具材をデータ（金額やスコア）ごと追加する関数
	bool AddIngredientWithData(FName IngredientTag, float PriceMult, int32 Score);

	// 組み合わせ（レシピ）をチェックする関数
	void UpdateBentoRecipe();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bento|Events")
	void OnBentoCompleted(FName BentoName);

	// 【追加】インターフェース関数のオーバーライド宣言
	// これにより、ItemHoldComponentなどがこの弁当箱からデータを取り出せるようになります
	virtual void GetFoodData_Implementation(FName& OutFoodTag, float& OutPriceMultiplier, int32& OutScorePoint) override;
};