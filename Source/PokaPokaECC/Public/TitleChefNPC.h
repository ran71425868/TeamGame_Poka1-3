#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TitleChefNPC.generated.h"

UCLASS()
class POKAPOKAECC_API ATitleChefNPC : public ACharacter
{
	GENERATED_BODY()

public:
	ATitleChefNPC();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// --- 1. コンロ（Stove）サイクル用設定 ---
	UPROPERTY(EditAnywhere, Category = "Title AI|1. Stove Cycle")
	AActor* StoveFridgeMoveTarget;
	UPROPERTY(EditAnywhere, Category = "Title AI|1. Stove Cycle")
	AActor* StoveFridgeActor;
	UPROPERTY(EditAnywhere, Category = "Title AI|1. Stove Cycle")
	AActor* StoveMoveTarget;
	UPROPERTY(EditAnywhere, Category = "Title AI|1. Stove Cycle")
	AActor* StoveActor;

	// --- 2. フライヤー（Fryer）サイクル用設定 ---
	UPROPERTY(EditAnywhere, Category = "Title AI|2. Fryer Cycle")
	AActor* FryerFridgeMoveTarget;
	UPROPERTY(EditAnywhere, Category = "Title AI|2. Fryer Cycle")
	AActor* FryerFridgeActor;
	UPROPERTY(EditAnywhere, Category = "Title AI|2. Fryer Cycle")
	AActor* FryerMoveTarget;
	UPROPERTY(EditAnywhere, Category = "Title AI|2. Fryer Cycle")
	AActor* FryerActor;

	// --- 3. 炊飯器（Rice Cooker）サイクル用設定 ---
	UPROPERTY(EditAnywhere, Category = "Title AI|3. Rice Cooker Cycle")
	AActor* RiceFridgeMoveTarget;
	UPROPERTY(EditAnywhere, Category = "Title AI|3. Rice Cooker Cycle")
	AActor* RiceFridgeActor;
	UPROPERTY(EditAnywhere, Category = "Title AI|3. Rice Cooker Cycle")
	AActor* RiceMoveTarget;
	UPROPERTY(EditAnywhere, Category = "Title AI|3. Rice Cooker Cycle")
	AActor* RiceCookerActor;

	// --- 共通設定 ---
	UPROPERTY(EditAnywhere, Category = "Title AI|Common")
	AActor* CounterMoveTarget;

	UPROPERTY(EditAnywhere, Category = "Title AI|Common")
	TSubclassOf<AActor> TargetMoneyClass;

	UPROPERTY()
	AActor* HeldItem;

	FTimerHandle ActionTimer;

	int32 CurrentStep;  // 0=冷蔵庫へ, 1=調理器具へ, 2=カウンターへ
	int32 CurrentCycle; // 0=Stove, 1=Fryer, 2=RiceCooker

	bool bIsLerpingItem;
	bool bIsLerpingToStation;

	void MoveToFridge();
	void TakeIngredient();
	void GrabIngredient();

	void MoveToStation();
	void StartCooking();
	void PutFoodOnStation();
	void PickupFood();

	void MoveToCounter();
	void ServeFood();

	void OnMoveCompleted(struct FAIRequestID RequestID, const struct FPathFollowingResult& Result);

	// 現在のサイクルに応じたアクタを自動取得するヘルパー関数
	AActor* GetCurrentFridgeMoveTarget() const;
	AActor* GetCurrentFridgeActor() const;
	AActor* GetCurrentStationMoveTarget() const;
	AActor* GetCurrentStationActor() const;
};