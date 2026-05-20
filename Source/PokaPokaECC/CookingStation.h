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

	// --- 【追加】調理中の音を鳴らすためのコンポーネント ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UAudioComponent* AudioComponent1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UAudioComponent* AudioComponent2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings")
	float CookTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings")
	float BurnTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings")
	TSubclassOf<AActor> CookedItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings")
	TSubclassOf<AActor> BurntItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings")
	FName AcceptedItemTag;

	// この器具に同時における最大数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings", meta = (ClampMin = "1", ClampMax = "2"))
	int32 MaxCapacity;

	// --- 【追加】BPエディタで設定するサウンドアセット ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings|Audio")
	class USoundBase* CookingSound;

	// --- 【追加】完成した時に一度だけ鳴らすサウンド ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings|Audio")
	class USoundBase* KamseiSound;
	// --- 【追加】BPエディタで設定するパーティクル/Niagaraアセット ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings|FX")
	class UNiagaraSystem* CookingEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings|FX")
	FVector EffectScale;

	// --- 【変更】状態管理とタイマーを2つに増やす ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooking State")
	ECookingState CurrentState1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooking State")
	ECookingState CurrentState2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooking State")
	AActor* CurrentItem1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooking State")
	AActor* CurrentItem2;

	// --- 【追加】生成されたエフェクトを後で消すためのコンポーネント変数 ---
	UPROPERTY()
	class UNiagaraComponent* SpawnedEffect1;

	UPROPERTY()
	class UNiagaraComponent* SpawnedEffect2;

	FTimerHandle CookingTimerHandle1;
	FTimerHandle CookingTimerHandle2;

	// タイマーから呼ばれる関数も1と2に分ける
	void OnCookingFinished1();
	void OnBurnt1();
	void OnCookingFinished2();
	void OnBurnt2();

	// BeginPlayのオーバーライド宣言を追加
	virtual void BeginPlay() override;
public:
	UFUNCTION(BlueprintCallable, Category = "Cooking")
	bool PlaceItem(AActor* ItemToPlace);

	UFUNCTION(BlueprintCallable, Category = "Cooking")
	AActor* RetrieveItem();

	// 調理速度の倍率（1.0が通常、数値が小さいほど速くなるなどの計算に使います）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings")
	float CookingSpeedMultiplier = 1.0f;
};