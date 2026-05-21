#include "BentoBoxItem.h"

ABentoBoxItem::ABentoBoxItem()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool ABentoBoxItem::AddIngredientWithData(FName IngredientTag, float PriceMult, int32 Score)
{
	// すでにお弁当が完成している場合は入れられない
	if (ProvidedFoodTag != FName("EmptyBento"))
	{
		return false;
	}

	// 具材は最大2つまで
	if (ContainedIngredients.Num() >= 2)
	{
		return false;
	}

	// すでに1つ具材が入っている場合の相性チェック
	if (ContainedIngredients.Num() == 1)
	{
		FName ExistingItem = ContainedIngredients[0];

		// ① すでに「米」が入っているのに、また「米」を入れようとしたら弾く
		if (ExistingItem == FName("Rice") && IngredientTag == FName("Rice"))
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("ご飯はすでに入っています！主菜を入れてください。"));
			return false;
		}

		// ② すでに「主菜（米以外）」が入っているのに、また「主菜（米以外）」を入れようとしたら弾く
		if (ExistingItem != FName("Rice") && IngredientTag != FName("Rice"))
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("主菜はすでに入っています！ご飯を入れてください。"));
			return false;
		}
	}

	// 条件をクリアしたので、具材リストに追加して計算
	ContainedIngredients.Add(IngredientTag);
	PriceMultiplier += PriceMult;
	ScorePoint += Score;

	// レシピの組み合わせが揃ったかチェックする
	UpdateBentoRecipe();

	return true;
}

void ABentoBoxItem::UpdateBentoRecipe()
{
	// お米が入っているかチェック
	bool bHasRice = ContainedIngredients.Contains(FName("Rice"));

	// 今回作る3つのお弁当はすべて「お米」が必須なので、お米がなければここで終了
	if (!bHasRice)
	{
		return;
	}

	// ① 焼肉弁当の判定
	if (ContainedIngredients.Contains(FName("Yakiniku")))
	{
		ProvidedFoodTag = FName("YakinikuBento");
		OnBentoCompleted(ProvidedFoodTag);
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("★ 焼肉弁当が完成しました！"));
	}
	// ② 唐揚げ弁当の判定
	else if (ContainedIngredients.Contains(FName("Karaage")))
	{
		ProvidedFoodTag = FName("KaraageBento");
		OnBentoCompleted(ProvidedFoodTag);
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("★ 唐揚げ弁当が完成しました！"));
	}
	// ③ のり弁の判定
	else if (ContainedIngredients.Contains(FName("Nori")))
	{
		ProvidedFoodTag = FName("NoriBento");
		OnBentoCompleted(ProvidedFoodTag);
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("★ のり弁が完成しました！"));
	}
	else if (ContainedIngredients.Contains(FName("Tempura")))
	{
		ProvidedFoodTag = FName("Tendon");
		OnBentoCompleted(ProvidedFoodTag);
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("★ 天丼が完成しました！"));
	}
}

// インターフェースを通じて自分のデータ（タグ、合計金額、合計スコア）を渡す処理
void ABentoBoxItem::GetFoodData_Implementation(FName& OutFoodTag, float& OutPriceMultiplier, int32& OutScorePoint)
{
	OutFoodTag = ProvidedFoodTag;
	OutPriceMultiplier = PriceMultiplier;
	OutScorePoint = ScorePoint;
}