#include "TitleMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UTitleMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UTitleMenuWidget::OnStartClicked);
	}
	// 【追加】Day1ボタンのクリックイベントを登録（Nullチェック付きで安全です）
	if (Day1Button)
	{
		Day1Button->OnClicked.AddDynamic(this, &UTitleMenuWidget::OnDay1Clicked);
	}
	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UTitleMenuWidget::OnExitClicked);
	}
}

void UTitleMenuWidget::SetFirstFocus()
{
	if (StartButton)
	{
		StartButton->SetFocus();
	}
}

void UTitleMenuWidget::OnStartClicked()
{
	UGameplayStatics::OpenLevel(this, FName(TEXT("Tutorial")));
}
// 【追加】Day1ボタンが押されたときの処理
void UTitleMenuWidget::OnDay1Clicked()
{
	UGameplayStatics::OpenLevel(this, Day1LevelName);
}
void UTitleMenuWidget::OnExitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, true);
}