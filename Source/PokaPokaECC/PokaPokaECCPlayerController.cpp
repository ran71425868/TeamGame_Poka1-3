// Copyright Epic Games, Inc. All Rights Reserved.


#include "PokaPokaECCPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "PauseMenuWidget.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "PokaPokaECC.h"
#include "Widgets/Input/SVirtualJoystick.h"

void APokaPokaECCPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogPokaPokaECC, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void APokaPokaECCPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}

	// Bind input actions
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (PauseAction)
		{
			// ESCキーなど（Started）でTogglePauseMenuを呼ぶ
			EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &APokaPokaECCPlayerController::TogglePauseMenu);
		}
	}
}

bool APokaPokaECCPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

// ゲームのポーズとポーズメニューの表示/非表示を切り替える関数
void APokaPokaECCPlayerController::TogglePauseMenu()
{
	if (!PauseMenuWidgetClass) return;

	if (IsPaused())
	{
		SetPause(false);

		// ゲームプレイのみの入力モードに戻す
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);

		bShowMouseCursor = false;

		if (PauseMenuWidgetInstance)
		{
			PauseMenuWidgetInstance->RemoveFromParent();
			PauseMenuWidgetInstance = nullptr;
		}
	}
	else
	{
		SetPause(true);

		PauseMenuWidgetInstance = CreateWidget<UUserWidget>(this, PauseMenuWidgetClass);
		if (PauseMenuWidgetInstance)
		{
			PauseMenuWidgetInstance->AddToViewport(100);

			// 【修正】UIOnlyではなくGameAndUIを使用する
			FInputModeGameAndUI InputMode;

			// 作成したウィジェット（スレート）を明示的にフォーカス対象として渡す
			InputMode.SetWidgetToFocus(PauseMenuWidgetInstance->TakeWidget());

			// マウスが画面外に出て裏のウィンドウをクリックしてしまうのを防ぐ
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);

			// キャプチャ中のカーソルを非表示にする
			InputMode.SetHideCursorDuringCapture(true);

			SetInputMode(InputMode);

			// 最後に確実にカーソルをオフにする
			bShowMouseCursor = false;

			// 専用クラスにキャストして、一番上のボタンにフォーカスを当てる
			if (UPauseMenuWidget* PauseMenu = Cast<UPauseMenuWidget>(PauseMenuWidgetInstance))
			{
				PauseMenu->SetFirstFocus();
			}
		}
	}
}