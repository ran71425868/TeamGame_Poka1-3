#include "TutorialManager.h"
#include "TimerManager.h"

ATutorialManager::ATutorialManager()
{
    PrimaryActorTick.bCanEverTick = false;
    CurrentStep = ETutorialStep::Start;
}

void ATutorialManager::BeginPlay()
{
    Super::BeginPlay();

    // ゲーム開始から1秒後に、最初の「お肉を取る」指示を自動で出す
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            SetTutorialStep(ETutorialStep::GetMeat);
        }, 1.0f, false);
}

void ATutorialManager::AdvanceTutorial()
{
    if (CurrentStep == ETutorialStep::Completed) return;

    uint8 NextStepNum = static_cast<uint8>(CurrentStep) + 1;
    SetTutorialStep(static_cast<ETutorialStep>(NextStepNum));
}

void ATutorialManager::SetTutorialStep(ETutorialStep NewStep)
{
    // すでにそのステップになっている場合は二重で呼ばないようにする
    if (CurrentStep == NewStep) return;

    CurrentStep = NewStep;

    // ブループリント側のUIに合図を送る
    UpdateTutorialUI(CurrentStep);
}