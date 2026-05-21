#include "BentoBoxItem.h"

ABentoBoxItem::ABentoBoxItem()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool ABentoBoxItem::AddIngredientWithData(FName IngredientTag, float PriceMult, int32 Score)
{
	// すでにお弁当が完成している、または具材が4つ以上なら入れられない
	if (ProvidedFoodTag != FName("EmptyBento") || ContainedIngredients.Num() >= 4)
	{
		return false;
	}

	// 具材リストに追加
	ContainedIngredients.Add(IngredientTag);

	// お金と評価を足し算する！
	PriceMultiplier += PriceMult;
	ScorePoint += Score;

	// 具材が入るたびに、レシピの組み合わせが揃ったかチェックする
	UpdateBentoRecipe();

	return true;
}

void ABentoBoxItem::UpdateBentoRecipe()
{
	// 配列の中に特定の具材が入っているか調べる
	bool bHasRice = ContainedIngredients.Contains(FName("Rice"));
	bool bHasYakiniku = ContainedIngredients.Contains(FName("Yakiniku")); // 実際の調理された肉のタグ名に合わせてください

	// 【仕様】焼肉とお米が揃ったら「焼肉弁当」に変身！
	if (bHasRice && bHasYakiniku)
	{
		ProvidedFoodTag = FName("YakinikuBento");

		// ブループリント側に「焼肉弁当が完成したよ！」と通知（ここでBP側でモデルをパッと変える）
		OnBentoCompleted(ProvidedFoodTag);
	}

	// 💡 今後別の弁当（例：ハンバーグ弁当）を増やしたい時は、ここにelse ifを足すだけで簡単に拡張できます！
}

// 【追加】インターフェースを通じて自分のデータ（タグ、合計金額、合計スコア）を渡す処理
void ABentoBoxItem::GetFoodData_Implementation(FName& OutFoodTag, float& OutPriceMultiplier, int32& OutScorePoint)
{
	OutFoodTag = ProvidedFoodTag;
	OutPriceMultiplier = PriceMultiplier;
	OutScorePoint = ScorePoint;
}