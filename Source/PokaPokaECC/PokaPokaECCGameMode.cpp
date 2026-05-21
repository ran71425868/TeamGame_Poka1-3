#include "PokaPokaECCGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "PokaPokaECCGameInstance.h"
#include "PokaPokaECCPlayerController.h"

APokaPokaECCGameMode::APokaPokaECCGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentState = EStoreState::Preparation;
	TimeRemaining = DayDuration;
	CurrentDay = 1;
}

void APokaPokaECCGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UPokaPokaECCGameInstance* GI = Cast<UPokaPokaECCGameInstance>(GetGameInstance()))
	{
		CurrentDay = GI->CurrentDay;
	}

	TimeRemaining = DayDuration;
	CurrentState = EStoreState::Preparation;

	StartDay();
}

void APokaPokaECCGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CurrentState == EStoreState::InService)
	{
		TimeRemaining -= DeltaSeconds;
		if (TimeRemaining <= 0.0f)
		{
			TimeRemaining = 0.0f;
			EndDay();
		}
	}
}

void APokaPokaECCGameMode::StartDay()
{
	if (CurrentState == EStoreState::Preparation)
	{
		CurrentState = EStoreState::InService;
		TimeRemaining = DayDuration;
	}
}

// 営業終了時（タイマー0）の処理
void APokaPokaECCGameMode::EndDay()
{
	if (CurrentState == EStoreState::InService)
	{
		CurrentState = EStoreState::StoreClosed;
		UE_LOG(LogTemp, Warning, TEXT("Day %d: 営業終了！スキル選択(兼EndGameBox)を開きます。"), CurrentDay);

		// PlayerControllerのOpenSkillMenuを直接呼ぶ
		if (APokaPokaECCPlayerController* PC = Cast<APokaPokaECCPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
		{
			PC->OpenSkillMenu();
		}
	}
}

void APokaPokaECCGameMode::ProceedToSkillSelection()
{
	// EndGameBoxで「次へ」が押されたら、スキル選択を開く
	if (APokaPokaECCPlayerController* PC = Cast<APokaPokaECCPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->OpenSkillMenu();
	}
}

bool APokaPokaECCGameMode::IsShopDay() const
{
	return (CurrentDay == 1 || CurrentDay == 3 || CurrentDay == 6);
}

// スキル選択・EndGameBoxが完了した後に呼ばれる処理
void APokaPokaECCGameMode::ProceedAfterSkill()
{
	if (IsShopDay())
	{
		UE_LOG(LogTemp, Warning, TEXT("Day %d はショップ日のため、ショップを開きます。"), CurrentDay);
		// コントローラーにショップを開くよう指示
		if (APokaPokaECCPlayerController* PC = Cast<APokaPokaECCPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
		{
			PC->ShowShopUI();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ショップ日ではないため、次のレベルへ進みます。"));
		TransitionToNextDay();
	}
}

void APokaPokaECCGameMode::TransitionToNextDay()
{
	if (UPokaPokaECCGameInstance* GI = Cast<UPokaPokaECCGameInstance>(GetGameInstance()))
	{
		GI->CurrentDay++;

		// 7日目を終えていた場合（現在の日数が8になった場合）、最終リザルトシーンへ遷移
		if (GI->CurrentDay > 7)
		{
			UE_LOG(LogTemp, Warning, TEXT("7日間の営業達成！最終リザルトシーンへ遷移します。"));
			// ※ここで CurrentDay=1 にリセットするかどうかは、リザルト画面の仕様に合わせて調整してください
			UGameplayStatics::OpenLevel(this, FName(*FinalResultMapName));
			return;
		}

		// 例：Day2, Day3 などのマップを読み込む
		FString NextLevelName = FString::Printf(TEXT("Day%d"), GI->CurrentDay);
		UE_LOG(LogTemp, Warning, TEXT("次のレベルを読み込みます: %s"), *NextLevelName);

		UGameplayStatics::OpenLevel(this, FName(*NextLevelName));
	}
}

void APokaPokaECCGameMode::TransitionToTitle()
{
	if (!TitleMapName.IsEmpty())
	{
		UGameplayStatics::OpenLevel(this, FName(*TitleMapName));
	}
}