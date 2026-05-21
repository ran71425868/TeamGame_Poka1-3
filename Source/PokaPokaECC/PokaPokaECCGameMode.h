#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PokaPokaECCGameMode.generated.h"

UENUM(BlueprintType)
enum class EStoreState : uint8
{
	Preparation UMETA(DisplayName = "開店準備"),
	InService   UMETA(DisplayName = "営業中"),
	StoreClosed UMETA(DisplayName = "営業終了")
};

UCLASS(minimalapi)
class APokaPokaECCGameMode : public AGameModeBase
{
	GENERATED_BODY() 

public:
	APokaPokaECCGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void StartDay();

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void EndDay();

	// EndGameBoxで「次へ（進む）」を押した時に呼ばれる（スキル選択を開く）
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void ProceedToSkillSelection();

	// スキル選択が完了した時にコントローラーから呼ばれる（ショップを開くか判定）
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void ProceedAfterSkill();

	// ショップ終了後、またはショップが無い日に呼ばれる（次のレベルへ遷移）
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void TransitionToNextDay();

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void TransitionToTitle();

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	bool IsShopDay() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Flow")
	EStoreState CurrentState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Flow")
	float DayDuration = 5.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Flow")
	float TimeRemaining;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Flow")
	int32 CurrentDay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Flow")
	FString TitleMapName = TEXT("Title");

	// 最終リザルトシーンのマップ名
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Flow")
	FString FinalResultMapName = TEXT("FinalResultScene");

protected:
	// --- BP側でUIを表示するためのイベント ---

	// 1日の終わりにEndGameBoxを表示するイベント
	UFUNCTION(BlueprintImplementableEvent, Category = "Game Flow|UI")
	void OnShowEndGameBox();

	// ショップ画面のUIを表示するイベント
	UFUNCTION(BlueprintImplementableEvent, Category = "Game Flow|UI")
	void OnShowShopUI();
};