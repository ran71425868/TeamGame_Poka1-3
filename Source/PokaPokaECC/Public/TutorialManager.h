#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialManager.generated.h"

// ★変更：チュートリアルの手順（ステップ）を焼肉弁当用に一新！
UENUM(BlueprintType)
enum class ETutorialStep : uint8
{
    Start           UMETA(DisplayName = "Start"),
    GetMeat         UMETA(DisplayName = "Get Meat"),
    CookMeat        UMETA(DisplayName = "Cook Meat"),
    GetRice         UMETA(DisplayName = "Get Rice"),
    CookRice        UMETA(DisplayName = "Cook Rice"),
    TakeCookedRice  UMETA(DisplayName = "Take Cooked Rice"),
    AssembleBento   UMETA(DisplayName = "Assemble Bento"),
    ServeBento      UMETA(DisplayName = "Serve Bento"),
    Completed       UMETA(DisplayName = "Completed")
};

UCLASS()
class POKAPOKAECC_API ATutorialManager : public AActor
{
    GENERATED_BODY()

public:
    ATutorialManager();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tutorial")
    ETutorialStep CurrentStep;

    // 順番に次のステップへ進める関数
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void AdvanceTutorial();

    // 「特定のステップ」に直接進める関数（※安全確実な方法）
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void SetTutorialStep(ETutorialStep NewStep);

    // ブループリント（UI）に「文字を更新して！」と伝える合図
    UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial")
    void UpdateTutorialUI(ETutorialStep NewStep);
};