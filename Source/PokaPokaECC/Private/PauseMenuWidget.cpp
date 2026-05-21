#include "PauseMenuWidget.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

// コンストラクタの実装
UPauseMenuWidget::UPauseMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// キーボード/ゲームパッドの入力をウィジェットで受け取るための必須設定
	bIsFocusable = true;
}

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 各ボタンにクリック時のイベントを紐づけ
	if (ResumeButton)
	{
		ResumeButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);
	}
	if (RetryButton)
	{
		RetryButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnRetryClicked);
	}
	if (TitleButton)
	{
		TitleButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnTitleClicked);
	}
}

// 最初にフォーカスを当てるボタンを設定する関数
void UPauseMenuWidget::SetFirstFocus()
{
	if (ResumeButton)
	{
		// SetKeyboardFocus() ではなく SetFocus() を使用します。
		// これにより、キーボードだけでなくUMGのUIナビゲーションシステム全体が
		// フォーカスを正しく認識し、矢印キーでの移動が安定します。
		ResumeButton->SetFocus();
	}
}

void UPauseMenuWidget::OnResumeClicked()
{
	// ポーズを解除してUIを閉じる
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->SetPause(false);

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);

		// 閉じる時も念のためマウスカーソルをオフにする
		PC->bShowMouseCursor = false;
	}
	RemoveFromParent();
}

void UPauseMenuWidget::OnRetryClicked()
{
	// 現在のレベル（マップ）を再読み込みしてリトライ
	UWorld* World = GetWorld();
	if (World)
	{
		FString CurrentLevelName = World->GetMapName();
		CurrentLevelName.RemoveFromStart(World->StreamingLevelsPrefix);
		UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
	}
}

void UPauseMenuWidget::OnTitleClicked()
{
	// タイトル画面へ遷移
	// ※ "TitleLevel" の部分は実際のタイトルマップ名に合わせて変更してください
	UGameplayStatics::OpenLevel(this, FName("MainMenu"));
}