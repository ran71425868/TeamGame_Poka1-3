// Copyright Epic Games, Inc. All Rights Reserved.

#include "PokaPokaECCPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "PauseMenuWidget.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "PokaPokaECC.h"
#include "Kismet/KismetArrayLibrary.h" 
#include "Engine/Engine.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "Kismet/GameplayStatics.h"
#include "PokaPokaECCGameInstance.h" // ローグライクの日数管理にキャストするためインクルード

APokaPokaECCPlayerController::APokaPokaECCPlayerController()
{
	bIsSkillSelectionOpen = false;
}

void APokaPokaECCPlayerController::OpenSkillMenu()
{
	if (bIsSkillSelectionOpen || MasterSkillList.Num() == 0) return;

	bIsSkillSelectionOpen = true;
	SetPause(true);

	TArray<UUSkillDataAsset*> RandomSkills = GetRandomSkills();

	if (SkillMenuClass)
	{
		SkillMenuInstance = CreateWidget<UPokaPokaSkillWidget>(this, SkillMenuClass);

		if (SkillMenuInstance)
		{
			SkillMenuInstance->AvailableSkills = RandomSkills;
			SkillMenuInstance->CurrentIndex = 0;

			SkillMenuInstance->GenerateUI();
			SkillMenuInstance->AddToViewport();

			// -----------------------------------------------------------
			// ★変更箇所：GameAndUI ではなく UIOnly に変更し、UIに完全に操作をロックする
			// -----------------------------------------------------------
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(SkillMenuInstance->TakeWidget());
			// UIOnlyの時はマウスロックを外さないとクリックした瞬間にフォーカスが飛ぶのを防ぎやすい
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);

			// ウィジェット本体に確実にフォーカスを当てる
			SkillMenuInstance->SetKeyboardFocus();

			bShowMouseCursor = true;
		}
	}
}

void APokaPokaECCPlayerController::SelectSkill(UUSkillDataAsset* SelectedSkill)
{
	if (!SelectedSkill) return;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("【%s】のスキルを永続確定！"), *SelectedSkill->SkillName));
	}

	// ------------------------------------------------------------------------
	// TODO: ここでプレイヤーのパラメータやGameInstanceに永続バフの効果量を反映させる
	// ------------------------------------------------------------------------

	// UIインスタンスを画面から破棄
	if (SkillMenuInstance)
	{
		SkillMenuInstance->RemoveFromParent();
		SkillMenuInstance = nullptr;
	}

	// ゲームの一時停止を解除
	SetPause(false);

	// 入力モードを通常ゲームプレイに戻す
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());
	bIsSkillSelectionOpen = false;

	// --- 7日間のローグライク進行と「次のレベルに移行」させる処理 ---
	if (UPokaPokaECCGameInstance* GI = Cast<UPokaPokaECCGameInstance>(GetGameInstance()))
	{
		// 例: GameInstance等で日数（Day）を管理している場合、インクリメントして対応するマップをロード
		// ※ GameInstance側に日数管理用の int32 CurrentDay などの変数があると非常に綺麗に管理できます

		// ここでは例として、次の日のレベル（例: "Stage_Day2" など）をロードする処理の枠組みを記述します
		// FString NextLevelName = FString::Printf(TEXT("Stage_Day%d"), GI->CurrentDay);
		// UGameplayStatics::OpenLevel(this, FName(*NextLevelName));

		// 一時的な確認用として、現在のステージをリスタート、あるいは次の固定の料理マップへ移行
		UGameplayStatics::OpenLevel(this, FName("CookingStage_Next"));
	}
}

TArray<UUSkillDataAsset*> APokaPokaECCPlayerController::GetRandomSkills()
{
	TArray<UUSkillDataAsset*> Result;
	TArray<UUSkillDataAsset*> TempPool = MasterSkillList;

	for (int32 i = 0; i < TempPool.Num(); ++i)
	{
		int32 Index = FMath::RandRange(i, TempPool.Num() - 1);
		TempPool.Swap(i, Index);
	}
	for (int32 i = 0; i < FMath::Min(4, TempPool.Num()); ++i)
	{
		Result.Add(TempPool[i]);
	}
	return Result;
}

void APokaPokaECCPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);
		if (MobileControlsWidget)
		{
			MobileControlsWidget->AddToPlayerScreen(0);
		}
		else {
			UE_LOG(LogPokaPokaECC, Error, TEXT("Could not spawn mobile controls widget."));
		}
	}
}

void APokaPokaECCPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (PauseAction)
		{
			EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &APokaPokaECCPlayerController::TogglePauseMenu);
		}
		if (EnterAction)
		{
			EnhancedInputComponent->BindAction(EnterAction, ETriggerEvent::Started, this, &APokaPokaECCPlayerController::HandleEnterAction);
		}
	}
}

bool APokaPokaECCPlayerController::ShouldUseTouchControls() const
{
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void APokaPokaECCPlayerController::TogglePauseMenu()
{
	if (!PauseMenuWidgetClass) return;

	if (IsPaused())
	{
		SetPause(false);
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
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(PauseMenuWidgetInstance->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
			InputMode.SetHideCursorDuringCapture(true);
			SetInputMode(InputMode);
			bShowMouseCursor = false;

			if (UPauseMenuWidget* PauseMenu = Cast<UPauseMenuWidget>(PauseMenuWidgetInstance))
			{
				PauseMenu->SetFirstFocus();
			}
		}
	}
}

void APokaPokaECCPlayerController::HandleEnterAction()
{
	if (bIsSkillSelectionOpen && SkillMenuInstance)
	{
		SkillMenuInstance->OnEnterKeyPressed();
	}
}