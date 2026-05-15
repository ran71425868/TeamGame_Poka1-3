#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CookingStation.generated.h"

UENUM(BlueprintType)
enum class ECookingState : uint8
{
	Empty       UMETA(DisplayName = "空っぽ"),
	Cooking     UMETA(DisplayName = "調理中"),
	Done        UMETA(DisplayName = "完成"),
	Burnt       UMETA(DisplayName = "焦げ（失敗）")
};

UCLASS()
class POKAPOKAECC_API ACookingStation : public AActor
{
	GENERATED_BODY()

public:
	ACookingStation();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* StationMesh;

	// --- 【変更】ソケットを2つに増やす ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* ItemSocket1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* ItemSocket2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings")
	float CookTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings")
	float BurnTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings")
	TSubclassOf<AActor> CookedItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings")
	TSubclassOf<AActor> BurntItemClass;

	// --- 【変更】状態管理とタイマーを2つに増やす ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooking State")
	ECookingState CurrentState1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooking State")
	ECookingState CurrentState2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooking State")
	AActor* CurrentItem1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooking State")
	AActor* CurrentItem2;

	FTimerHandle CookingTimerHandle1;
	FTimerHandle CookingTimerHandle2;

	// タイマーから呼ばれる関数も1と2に分ける
	void OnCookingFinished1();
	void OnBurnt1();
	void OnCookingFinished2();
	void OnBurnt2();

public:
	UFUNCTION(BlueprintCallable, Category = "Cooking")
	bool PlaceItem(AActor* ItemToPlace);

	UFUNCTION(BlueprintCallable, Category = "Cooking")
	AActor* RetrieveItem();
};