#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemSpawner.h"
#include "ItemHoldComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMoneyCollectedDelegate, int32, MoneyAmount, int32, ScoreAmount);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class POKAPOKAECC_API UItemHoldComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UItemHoldComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// キャラクターから呼ばれるメインの入力処理
	void PrimaryInteract();

	// 設定値
	UPROPERTY(EditAnywhere, Category = "Interact")
	FName HandSocketName = FName("HoldSocket");

	UPROPERTY(EditAnywhere, Category = "Interact")
	float InteractDistance = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Interact")
	float ItemSnapSpeed = 15.0f;

	// ゴミ箱サウンド
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact|Sound")
	class USoundBase* TrashSound;
	//  アイテムを持ったときに再生するサウンド
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact|Sound")
	class USoundBase* PickUpSound;
	// アイテムを置いたときに再生するサウンド
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact|Sound")
	class USoundBase* PlaceSound;
	// アイテムを持った時に再生するエフェクト
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact|Effect")
	class UParticleSystem* PickUpEffect;

	// グリッド配置関連の設定
	UPROPERTY(EditAnywhere, Category = "Interact|Grid")
	float GridSize = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Interact|Grid")
	float PlacementZOffset = 20.0f;

	// アイテムを配置できる高さの上限
	UPROPERTY(EditAnywhere, Category = "Interact|Grid")
	float MaxPlacementHeight = 50.0f;

	// ハイライト表示用のメッシュとマテリアル
	UPROPERTY(EditAnywhere, Category = "Interact|Grid")
	UStaticMesh* HighlightMeshAsset;

	UPROPERTY(EditAnywhere, Category = "Interact|Grid")
	UMaterialInterface* HighlightMaterial;

	// マグネットが反応する範囲（半径cm）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnet")
	float MagnetRadius = 300.0f;

	// お金がプレイヤーに飛んでくるスピード
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnet")
	float MagnetSpeed = 10.0f;

	// お金を「回収した」と判定するプレイヤーとの距離（cm）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnet")
	float CollectionDistance = 70.0f;

	// お金を回収した時にブループリント側で所持金やスコアを増やすためのイベント
	UPROPERTY(BlueprintAssignable, Category = "ItemHold|Events")
	FOnMoneyCollectedDelegate OnMoneyCollected;

	// 統計情報
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TotalCollectedMoney = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TotalCollectedScore = 0;

private:
	// 内部状態
	UPROPERTY()
	AActor* HeldItem = nullptr;

	UPROPERTY()
	AActor* PlacingItem = nullptr;

	// C++で動的に生成するハイライト用コンポーネント
	UPROPERTY()
	UStaticMeshComponent* GridHighlightMesh;

	bool bIsItemSnapping = false;
	bool bIsItemPlacing = false;
	FVector PlaceTargetLocation = FVector::ZeroVector;
	FRotator PlaceTargetRotation = FRotator::ZeroRotator;

	// 持ち主（キャラクター）への参照をキャッシュ
	class ACharacter* OwnerCharacter;

	// Tickで計算した配置座標を保持しておく変数群
	FVector CurrentGridTargetLocation;
	FRotator CurrentGridTargetRotation;
	bool bCanPlaceOnGrid = false;

	// ハイライト位置を毎フレーム更新する関数
	void UpdateGridHighlight();
};