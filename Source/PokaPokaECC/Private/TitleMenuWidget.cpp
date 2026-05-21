#include "TitleMenuWidget.h"
// C++側でボタンや遷移の処理を行わないため、以下のインクルードも不要になります
// #include "Components/Button.h"
// #include "Kismet/GameplayStatics.h"
// #include "Kismet/KismetSystemLibrary.h"

// 【変更】処理の中身もすべてコメントアウトします
/*
void UTitleMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UTitleMenuWidget::OnStartClicked);
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
	UGameplayStatics::OpenLevel(this, NextLevelName);
}

void UTitleMenuWidget::OnExitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, true);
}
*/