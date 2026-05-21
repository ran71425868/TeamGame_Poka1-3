#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TitleChefAnimInstance.generated.h"

UCLASS()
class POKAPOKAECC_API UTitleChefAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// 初期化時に1度だけ呼ばれる関数（Blueprintの Event Blueprint Initialize Animation に相当）
	virtual void NativeInitializeAnimation() override;

	// 毎フレーム呼ばれる関数（Blueprintの Event Blueprint Update Animation に相当）
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// ブループリント（AnimGraph）側で読み取るための速度変数
	// BlueprintReadOnly をつけることで、ノードとして取得可能になります
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Speed;

	// 落下状態の判定用変数
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsFalling;
};