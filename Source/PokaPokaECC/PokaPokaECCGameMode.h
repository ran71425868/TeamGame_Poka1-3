// Copyright Epic Games, Inc. All Rights Reserved.

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

	// 営業を開始する
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void StartDay();

	// 営業を終了する（タイマー0で自動呼び出し）
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void EndDay();

	// スキル獲得後に「次へ」を押したときの処理
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void TransitionToNextDay();

	// 「タイトルへ」を押したときの処理
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void TransitionToTitle();

	// --- 進行管理変数 ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Flow")
	EStoreState CurrentState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Flow")
	float DayDuration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Flow")
	float TimeRemaining;

	// 現在の日にち（初日は1）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Flow")
	int32 CurrentDay;

	// タイトル画面のマップ名（エディタで設定可能に）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Flow")
	FString TitleMapName;

	// メインゲームのマップ名（ループ用）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Flow")
	FString GameMapName;
};



