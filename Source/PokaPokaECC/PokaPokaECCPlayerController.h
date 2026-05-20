// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "USkillDataAsset.h"
#include "PokaPokaSkillWidget.h"
#include "PokaPokaECCPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UInputAction;

/**
 * Basic PlayerController class for a third person game
 * Manages input mappings
 */
UCLASS(abstract)
class APokaPokaECCPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	APokaPokaECCPlayerController();

	// エディタ上で全スキルを登録するためのリスト
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill System")
	TArray<UUSkillDataAsset*> MasterSkillList;

	UPROPERTY(BlueprintReadWrite, Category = "Skill System")
	UPokaPokaSkillWidget* SkillMenuInstance;

	// どのUIを作るか指定するための枠（エディタで設定します）
	UPROPERTY(EditAnywhere, Category = "Skill System")
	TSubclassOf<UPokaPokaSkillWidget> SkillMenuClass;

	// スキル選択画面を開く処理
	UFUNCTION(BlueprintCallable, Category = "Skill System")
	void OpenSkillMenu();

	// UIでスキルが選択され、かつ「次へ」が押された時に呼ばれる最終処理
	UFUNCTION(BlueprintCallable, Category = "Skill System")
	void SelectSkill(UUSkillDataAsset* SelectedSkill);

private:
	bool bIsSkillSelectionOpen;

	// スキルプールからランダムに4つ抽出する関数
	TArray<UUSkillDataAsset*> GetRandomSkills();

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category = "Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/// Input Action for pausing the game
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	UInputAction* PauseAction;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> PauseMenuWidgetInstance;

	// エンターキーを検知するためのアクションを追加
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	UInputAction* EnterAction;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

	/// Toggles the pause menu on or off
	void TogglePauseMenu();

	// エンターキーが押された時に走るC++関数
	void HandleEnterAction();
};