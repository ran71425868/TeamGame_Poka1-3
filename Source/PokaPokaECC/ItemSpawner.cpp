#include "ItemSpawner.h"
#include "Engine/World.h"

// --- 【追加】音とエフェクトを再生するためのインクルード ---
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

// コンストラクタ：ゲーム開始前の初期設定
AItemSpawner::AItemSpawner()
{
	// スポナー自体は毎フレームの更新（Tick）を必要としないので処理を軽くするためにオフにする
	PrimaryActorTick.bCanEverTick = false;

	// メッシュコンポーネントを作成し、ルート（基準点）に設定
	SpawnerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnerMesh"));
	RootComponent = SpawnerMesh;

	// プレイヤーのSphereCastで「これはスポナーだ！」と判別できるようにタグを追加
	Tags.Add(FName("Spawner"));
}

// アイテムを生成する関数
AActor* AItemSpawner::SpawnItem()
{
	// BP側で「出すアイテム」が設定されていなければ、エラーを防ぐため何もしない
	if (!ItemClassToSpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemSpawner: ItemClassToSpawn is null!"));
		return nullptr;
	}

	// アイテムを生成する位置と回転を設定（箱の少し上に出現させる）
	FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 40.0f);
	FRotator SpawnRotation = GetActorRotation();

	// 世界（World）にアイテムを生成（Spawn）する
	AActor* SpawnedItem = GetWorld()->SpawnActor<AActor>(ItemClassToSpawn, SpawnLocation, SpawnRotation);
	
	// --- 【追加】スポーン時に音を鳴らす ---
	if (SpawnSound)
	{
		// スポナーの位置から音を鳴らす
		UGameplayStatics::PlaySoundAtLocation(this, SpawnSound, GetActorLocation());
	}

	if (SpawnEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SpawnEffect, SpawnLocation, SpawnRotation);
	}
	
	// 生成したアイテムを返す
	return SpawnedItem;
}