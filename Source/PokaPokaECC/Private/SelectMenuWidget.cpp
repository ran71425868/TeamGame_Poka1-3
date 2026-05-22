#include "SelectMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void USelectMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 1. 各ボタンがフォーカスを受け取れるように変数(IsFocusable)を直接trueにし、クリック処理を紐付け
	// ※ UButtonクラスの変数は例外的に「b」がつかない「IsFocusable」になります。
	if (Button_Play)
	{
		Button_Play->IsFocusable = true;
		Button_Play->OnClicked.AddDynamic(this, &USelectMenuWidget::OnPlayClicked);
	}

	if (Button_Tutorial)
	{
		Button_Tutorial->IsFocusable = true;
		Button_Tutorial->OnClicked.AddDynamic(this, &USelectMenuWidget::OnTutorialClicked);
	}

	if (Button_Main)
	{
		Button_Main->IsFocusable = true;
		Button_Main->OnClicked.AddDynamic(this, &USelectMenuWidget::OnMainClicked);
	}

	// 2. ウィジェット自身でプレイヤーコントローラーを取得し、入力モードとフォーカス処理を完結させる
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		// UI専用の入力モードを作成
		FInputModeUIOnly InputMode;

		if (Button_Play)
		{
			// スレート（内部UIシステム）レベルでButton_Playに直接フォーカスを設定
			InputMode.SetWidgetToFocus(Button_Play->TakeWidget());

			// Unreal Engineレベルでのキーボードフォーカスも強制的にButton_Playへ当てる
			Button_Play->SetKeyboardFocus();
		}
		else
		{
			// ボタンが見つからなければウィジェット全体にフォーカスを当てる
			InputMode.SetWidgetToFocus(TakeWidget());
		}

		// プレイヤーコントローラーに入力モードを適用し、マウスカーソルも表示する
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}

	// ウィジェット自身（UUserWidget）の変数は通常通り「b」がつく「bIsFocusable」が正解です
	bIsFocusable = true;
}

void USelectMenuWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 毎フレームフォーカス状態を確認し、見た目を更新する
	UpdateButtonAppearance(Button_Play);
	UpdateButtonAppearance(Button_Tutorial);
	UpdateButtonAppearance(Button_Main);
}

void USelectMenuWidget::UpdateButtonAppearance(UButton* Button)
{
	if (!Button)
	{
		return;
	}

	// フォーカスが当たっている（キーボードの矢印キーで選ばれている）場合
	if (Button->HasAnyUserFocus())
	{
		Button->SetBackgroundColor(FLinearColor(1.0f, 1.0f, 0.0f, 1.0f)); // 黄色
		Button->SetRenderScale(FVector2D(1.1f, 1.1f));                    // 少し拡大
	}
	else
	{
		Button->SetBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)); // 通常の色
		Button->SetRenderScale(FVector2D(1.0f, 1.0f));                    // 通常のサイズ
	}
}

void USelectMenuWidget::OnPlayClicked()
{
	UGameplayStatics::OpenLevel(this, FName("Day1"));
}

void USelectMenuWidget::OnTutorialClicked()
{
	UGameplayStatics::OpenLevel(this, FName("Tutorial"));
}

void USelectMenuWidget::OnMainClicked()
{
	UGameplayStatics::OpenLevel(this, FName("MainMenu"));
}