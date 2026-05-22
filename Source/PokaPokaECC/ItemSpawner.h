#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemSpawner.generated.h"

UCLASS()
class POKAPOKAECC_API AItemSpawner : public AActor
{
	GENERATED_BODY()

public:
	AItemSpawner();

	// ▼【変更】1つだけではなく、複数のアイテムをリストとして持てるように変更
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TArray<TSubclassOf<AActor>> SpawnItemClasses;

	// デバッグ・UI表示用のアイテム名（上のSpawnItemClassesと同じ順番で設定します）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TArray<FString> ItemNames;

	// 現在選択されているインデックス
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	int32 CurrentSelectedIndex;

	// 生成処理
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	AActor* SpawnItem();

	// ▼【追加】選択を切り替える関数（Directionは 1:右, -1:左）
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void CycleSelection(int32 Direction);
};