#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemSpawner.h"
#include "ItemHoldComponent.generated.h"

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

	// グリッド配置関連の設定
	UPROPERTY(EditAnywhere, Category = "Interact|Grid")
	float GridSize = 100.0f; // グリッドのサイズ（100cm単位など）

	UPROPERTY(EditAnywhere, Category = "Interact|Grid")
	float PlacementZOffset = 20.0f; // 床に置いた際にめり込まないためのZ方向オフセット

	// アイテムを配置できる高さの上限（キャラクターの中心座標からの相対高さ）
	UPROPERTY(EditAnywhere, Category = "Interact|Grid")
	float MaxPlacementHeight = 50.0f;

	// ハイライト表示用のメッシュとマテリアル（BPで設定）
	UPROPERTY(EditAnywhere, Category = "Interact|Grid")
	UStaticMesh* HighlightMeshAsset;

	UPROPERTY(EditAnywhere, Category = "Interact|Grid")
	UMaterialInterface* HighlightMaterial;

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