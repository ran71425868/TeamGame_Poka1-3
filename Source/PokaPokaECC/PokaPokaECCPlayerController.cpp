// Copyright Epic Games, Inc. All Rights Reserved.

#include "PokaPokaECCPlayerController.h"
#include "PokaPokaECCGameMode.h"
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
#include "ChefShopWidget.h"
#include "PokaPokaECCGameMode.h"
#include "SkillEffectManagerComponent.h"

APokaPokaECCPlayerController::APokaPokaECCPlayerController()
{
	bIsSkillSelectionOpen = false;

	//コントローラー生成時にスキルマネージャーコンポーネントも一緒に生成する
	SkillEffectManager = CreateDefaultSubobject<USkillEffectManagerComponent>(TEXT("SkillEffectManager"));
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

	if (SkillEffectManager)
	{
		SkillEffectManager->ExecuteSkillEffect(SelectedSkill);
	}

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

	// --- 変更箇所：次のステップ（ショップ判定）をGameModeに依頼 ---
	if (APokaPokaECCGameMode* GameMode = Cast<APokaPokaECCGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->ProceedAfterSkill();
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

// ----------------------------------------------------
// ショップUIを生成して表示する処理
// ----------------------------------------------------
void APokaPokaECCPlayerController::ShowShopUI()
{
	if (!ShopWidgetClass) return;

	ShopWidgetInstance = CreateWidget<UChefShopWidget>(this, ShopWidgetClass);
	if (ShopWidgetInstance)
	{
		ShopWidgetInstance->InitShopItems(); // 8個のアイテムを並べる
		ShopWidgetInstance->AddToViewport();

		// UI操作のみにフォーカスをロック
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(ShopWidgetInstance->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}