#include "PokaPokaECCGameMode.h"
#include "ChefPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

APokaPokaECCGameMode::APokaPokaECCGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	DayDuration = 60.0f; // 初日は1分(60秒)
	TimeRemaining = DayDuration;
	CurrentState = EStoreState::Preparation;
	CurrentDay = 1;
	TitleMapName = TEXT("TitleMap"); // 実際のタイトルマップ名に合わせてください
	GameMapName = TEXT("MainGameMap"); // 実際のメインゲームマップ名に合わせてください
}

void APokaPokaECCGameMode::BeginPlay()
{
	Super::BeginPlay();
	CurrentState = EStoreState::Preparation;
	TimeRemaining = DayDuration;

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

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("Business start"));
	}
}

void APokaPokaECCGameMode::EndDay()
{
	CurrentState = EStoreState::StoreClosed;

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Business finish"));

	// 1. プレイヤーの操作を禁止にする
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		if (APawn* PlayerPawn = PC->GetPawn())
		{
			PlayerPawn->DisableInput(PC); // 移動やインタラクトを物理的にストップ
		}

		// 2. スキル選択画面を表示する
		if (AChefPlayerController* ChefPC = Cast<AChefPlayerController>(PC))
		{
			ChefPC->OpenSkillMenu();
		}
	}
}

void APokaPokaECCGameMode::TransitionToNextDay()
{
	// 日にちをプラスして、再度ゲームマップを読み直す（セーブデータやGameInstanceに日数を引き継ぐ設計が一般的です）
	CurrentDay++;

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Day %d へ遷移します..."), CurrentDay));

	// 同じマップをリロード、あるいは次のステージのマップを開く
	UGameplayStatics::OpenLevel(this, FName(*GameMapName));
}

void APokaPokaECCGameMode::TransitionToTitle()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, TEXT("タイトル画面へ戻ります..."));

	// タイトル画面用のマップを開く
	UGameplayStatics::OpenLevel(this, FName(*TitleMapName));
}
