#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialManager.generated.h"

// チュートリアルの各進捗ステージを定義する列挙型
UENUM(BlueprintType)
enum class ETutorialStep : uint8
{
    Welcome      UMETA(DisplayName = "Welcome"),
    PickTomato   UMETA(DisplayName = "Pick Tomato"),
    CookTomato   UMETA(DisplayName = "Cook Tomato"),
    ServeBob     UMETA(DisplayName = "Serve Bob"),
    Complete     UMETA(DisplayName = "Complete")
};

// ステップが切り替わったことをUIに伝えるためのイベント（デリゲート）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTutorialStepChangedSignature, ETutorialStep, NewStep);

UCLASS()
class POKAPOKAECC_API ATutorialManager : public AActor
{
    GENERATED_BODY()

public:
    ATutorialManager();

protected:
    virtual void BeginPlay() override;

public:
    // 現在のチュートリアルのステップ（BPからいつでも確認可能）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial")
    ETutorialStep CurrentStep;

    // ステップが変更された時に発行されるイベント（BP側でイベントをバインドしてUIアニメーションを流す）
    UPROPERTY(BlueprintAssignable, Category = "Tutorial|Events")
    FOnTutorialStepChangedSignature OnTutorialStepChanged;

    // 次のステップに進める関数（トマトを拾った時などにBPから呼び出す）
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void AdvanceStep();

    // 任意のステップを直接セットする関数
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void SetTutorialStep(ETutorialStep NewStep);
};