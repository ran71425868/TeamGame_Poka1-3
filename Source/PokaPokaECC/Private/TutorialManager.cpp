#include "TutorialManager.h"
#include "Engine/Engine.h"

ATutorialManager::ATutorialManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // 最初のステップを設定
    CurrentStep = ETutorialStep::Welcome;
}

void ATutorialManager::BeginPlay()
{
    Super::BeginPlay();

    // ゲーム開始時に最初のステップ通知を送る
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            OnTutorialStepChanged.Broadcast(CurrentStep);
        }, 0.2f, false);
}

void ATutorialManager::AdvanceStep()
{
    uint8 CurrentRawStep = static_cast<uint8>(CurrentStep);
    uint8 NextRawStep = CurrentRawStep + 1;

    // Complete（最後）を超えないように制限して更新
    if (NextRawStep <= static_cast<uint8>(ETutorialStep::Complete))
    {
        SetTutorialStep(static_cast<ETutorialStep>(NextRawStep));
    }
}

void ATutorialManager::SetTutorialStep(ETutorialStep NewStep)
{
    CurrentStep = NewStep;

    // ブループリント（UIや演出）に変ったことを通知
    if (OnTutorialStepChanged.IsBound())
    {
        OnTutorialStepChanged.Broadcast(CurrentStep);
    }

    // デバッグ用のログ画面表示
    FString StepName = TEXT("");
    switch (CurrentStep)
    {
    case ETutorialStep::Welcome:    StepName = TEXT("Welcome"); break;
    case ETutorialStep::PickTomato: StepName = TEXT("PickTomato"); break;
    case ETutorialStep::CookTomato: StepName = TEXT("CookTomato"); break;
    case ETutorialStep::ServeBob:   StepName = TEXT("ServeBob"); break;
    case ETutorialStep::Complete:   StepName = TEXT("Complete"); break;
    }
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("Tutorial Progressed to: %s"), *StepName));
}