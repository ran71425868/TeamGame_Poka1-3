#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FoodItemInterface.generated.h"

// ---------------------------------------------------------
// この UFoodItemInterface はUEのシステム用なので、基本的には触りません
// Blueprintable をつけることで、BPからもこのインターフェースを実装できるようになります
// ---------------------------------------------------------
UINTERFACE(MinimalAPI, Blueprintable)
class UFoodItemInterface : public UInterface
{
	GENERATED_BODY()
};

// ---------------------------------------------------------
// 実際にゲーム内で使う関数はこちらの IFoodItemInterface に定義します
// ---------------------------------------------------------
class POKAPOKAECC_API IFoodItemInterface
{
	GENERATED_BODY()

public:
	// 食材のデータ（タグ、お金、スコア）をまとめて取得するための関数
	// 
	// 【BlueprintNativeEvent】: C++でデフォルトの処理を書きつつ、BPで上書き(オーバーライド)できる最強のハイブリッド設定。
	// 【BlueprintCallable】: BPのノードとして呼び出せるようにする設定。
	// 【& (参照渡し)】: 引数に & をつけることで、BPではこれらの引数が「出力ピン」として扱われます。
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Food")
	void GetFoodData(FName& OutFoodTag, float& OutPriceMultiplier, int32& OutScorePoint);
};