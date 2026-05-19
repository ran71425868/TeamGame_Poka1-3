#include "GameFramework/CharacterMovementComponent.h"
#include "PokaPokaECCCharacter.h"
#include "SkillEffectManagerComponent.h"
#include "USkillDataAsset.h" // ※ご自身の環境のデータアセットのヘッダー名に合わせてください
#include "Engine/Engine.h"

USkillEffectManagerComponent::USkillEffectManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USkillEffectManagerComponent::ExecuteSkillEffect(UUSkillDataAsset* AppliedSkill)
{
	if (!AppliedSkill) return;

	FName ID = AppliedSkill->SkillID;

	// デバッグ用に「何が発動したか」を共通で通知
	FString LogMsg = FString::Printf(TEXT("【発動箱】スキルID: %s, 効果量: %.2f"), *ID.ToString(), AppliedSkill->ModifierValue);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, LogMsg);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *LogMsg);

	// --- 20スキルの分岐処理 ---
	if (ID == FName("ArtisanPatience")) { ApplyArtisanPatience(AppliedSkill->ModifierValue); }
	else if (ID == FName("CozyAtmosphere")) { ApplyCozyAtmosphere(AppliedSkill->ModifierValue); }
	else if (ID == FName("FastFryer")) { ApplyFastFryer(AppliedSkill->ModifierValue); }
	else if (ID == FName("ForbiddenFullCourse")) { ApplyForbiddenFullCourse(AppliedSkill->ModifierValue); }
	else if (ID == FName("GratuityTipping")) { ApplyGratuityTipping(AppliedSkill->ModifierValue); }
	else if (ID == FName("GreedyTray")) { ApplyGreedyTray(AppliedSkill->ModifierValue); }
	else if (ID == FName("HighHeatBurner")) { ApplyHighHeatBurner(AppliedSkill->ModifierValue); }
	else if (ID == FName("LastSpurtRush")) { ApplyLastSpurtRush(AppliedSkill->ModifierValue); }
	else if (ID == FName("LegendaryQueue")) { ApplyLegendaryQueue(AppliedSkill->ModifierValue); }
	else if (ID == FName("LightFootwork")) { ApplyLightFootwork(AppliedSkill->ModifierValue); }
	else if (ID == FName("MichelinRush")) { ApplyMichelinRush(AppliedSkill->ModifierValue); }
	else if (ID == FName("PerfectRiceBoil")) { ApplyPerfectRiceBoil(AppliedSkill->ModifierValue); }
	else if (ID == FName("PlatingMastery")) { ApplyPlatingMastery(AppliedSkill->ModifierValue); }
	else if (ID == FName("ProMicrowave")) { ApplyProMicrowave(AppliedSkill->ModifierValue); }
	else if (ID == FName("RecipeInspiration")) { ApplyRecipeInspiration(AppliedSkill->ModifierValue); }
	else if (ID == FName("ServiceHigh")) { ApplyServiceHigh(AppliedSkill->ModifierValue); }
	else if (ID == FName("SpeedstersHaste")) { ApplySpeedstersHaste(AppliedSkill->ModifierValue); }
	else if (ID == FName("StarService")) { ApplyStarService(AppliedSkill->ModifierValue); }
	else if (ID == FName("UltraEfficiency")) { ApplyUltraEfficiency(AppliedSkill->ModifierValue); }
	else
	{
		UE_LOG(LogTemp, Error, TEXT("未実装のスキルIDです: %s"), *ID.ToString());
	}
}

// =========================================================================
// 各スキルの具体的な効果処理（実際のC++実装コードをコメントアウト状態で配置）
// =========================================================================

/* // ※将来コメントアウトを外す際、上部の #include 群に以下のようなヘッダーを追加してください。
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ChefCharacter.h" // ご自身のプレイヤーキャラクタークラス
#include "ChefGameState.h" // ご自身のゲームステートクラス
#include "CookingStation.h" // 調理器具の親クラスなど
*/



// -------------------------------------------------------------------------
// 【ショップ専用スキル】
// -------------------------------------------------------------------------

void USkillEffectManagerComponent::ApplyArtisanPatience(float ModifierValue)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("【ショップ】ArtisanPatience 発動！ (%.2f)"), ModifierValue));

	/*
	// 客の怒るまでの待ち時間を大幅に延長する
	if (AChefGameState* GameState = Cast<AChefGameState>(UGameplayStatics::GetGameState(this)))
	{
		// GameState側で管理している基本待機時間に加算
		GameState->BaseCustomerPatience += ModifierValue;
	}
	*/
}

void USkillEffectManagerComponent::ApplyLastSpurtRush(float ModifierValue)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("【ショップ】LastSpurtRush 発動！ (%.2f)"), ModifierValue));

	/*
	// 営業終了間際でプレイヤーの全速度が超絶アップするフラグを立てる
	if (AChefGameState* GameState = Cast<AChefGameState>(UGameplayStatics::GetGameState(this)))
	{
		// GameStateのTick等で、残り時間が30秒を切ったら効果を発動させるためのフラグ
		GameState->bIsLastSpurtSkillActive = true;
		GameState->LastSpurtSpeedMultiplier = ModifierValue;
	}
	*/
}

void USkillEffectManagerComponent::ApplyLegendaryQueue(float ModifierValue)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("【ショップ】LegendaryQueue 発動！ (%.2f)"), ModifierValue));

	/*
	// 客の最大来店数（行列の長さ制限）の解放
	if (AChefGameState* GameState = Cast<AChefGameState>(UGameplayStatics::GetGameState(this)))
	{
		// 小数点を切り捨てて整数（int32）として加算
		GameState->MaxCustomerQueueSize += FMath::TruncToInt(ModifierValue);
	}
	*/
}

void USkillEffectManagerComponent::ApplySpeedstersHaste(float ModifierValue)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("【ショップ】SpeedstersHaste 発動！ (%.2f)"), ModifierValue));

	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (APokaPokaECCCharacter* PlayerChar = Cast<APokaPokaECCCharacter>(PC->GetPawn()))
		{
			if (UCharacterMovementComponent* MovementComp = PlayerChar->GetCharacterMovement())
			{
				// 大幅な速度アップ（ModifierValue が大きい前提）
				MovementComp->MaxWalkSpeed += ModifierValue;
			}
		}
	}
}

void USkillEffectManagerComponent::ApplyUltraEfficiency(float ModifierValue)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("【ショップ】UltraEfficiency 発動！ (%.2f)"), ModifierValue));

	/*
	// 切る・焼く・洗うなど、プレイヤーの全てのインタラクト時間が短縮される
	if (AChefCharacter* PlayerChar = Cast<AChefCharacter>(GetOwner()))
	{
		// プレイヤー側に持たせている作業速度倍率（デフォルト1.0）を加算
		PlayerChar->InteractSpeedMultiplier += ModifierValue;
	}
	*/
}


// -------------------------------------------------------------------------
// 【ランダム排出スキル】
// -------------------------------------------------------------------------

void USkillEffectManagerComponent::ApplyCozyAtmosphere(float ModifierValue)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("===> CozyAtmosphere 発動！ (%.2f)"), ModifierValue));

	/*
	// 店内の雰囲気向上によるチップ発生率アップ
	if (AChefGameState* GameState = Cast<AChefGameState>(UGameplayStatics::GetGameState(this)))
	{
		GameState->TipProbability += ModifierValue;
	}
	*/
}

void USkillEffectManagerComponent::ApplyFastFryer(float ModifierValue)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("===> FastFryer 発動！ (%.2f)"), ModifierValue));

	/*
	// フライヤーの調理時間短縮（タグ検索の例）
	TArray<AActor*> FoundFryers;
	UGameplayStatics::GetAllActorsWithTag(this, FName("Fryer"), FoundFryers);

	for (AActor* Actor : FoundFryers)
	{
		if (ACookingStation* Fryer = Cast<ACookingStation>(Actor))
		{
			Fryer->CookingSpeedMultiplier += ModifierValue;
		}
	}
	*/
}

void USkillEffectManagerComponent::ApplyForbiddenFullCourse(float ModifierValue)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("===> ForbiddenFullCourse 発動！ (%.2f)"), ModifierValue));

	/*
	// 禁断のフルコース解禁
	if (AChefGameState* GameState = Cast<AChefGameState>(UGameplayStatics::GetGameState(this)))
	{
		GameState->bIsForbiddenMenuUnlocked = true;
	}
	*/
}

void USkillEffectManagerComponent::ApplyGratuityTipping(float ModifierValue)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("===> GratuityTipping 発動！ (%.2f)"), ModifierValue));

	/*
	// もらえるチップの金額が増加
	if (AChefGameState* GameState = Cast<AChefGameState>(UGameplayStatics::GetGameState(this)))
	{
		GameState->TipAmountMultiplier += ModifierValue;
	}
	*/
}

void USkillEffectManagerComponent::ApplyGreedyTray(float ModifierValue)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("===> GreedyTray 発動！ (%.2f)"), ModifierValue));

	/*
	// プレイヤーのお盆（インベントリ）の最大所持数増加
	if (AChefCharacter* PlayerChar = Cast<AChefCharacter>(GetOwner()))
	{
		PlayerChar->MaxTrayCapacity += FMath::TruncToInt(ModifierValue);
	}
	*/
}

void USkillEffectManagerComponent::ApplyHighHeatBurner(float ModifierValue)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("===> HighHeatBurner 発動！ (%.2f)"), ModifierValue));

	/*
	// コンロの調理時間短縮
	TArray<AActor*> FoundBurners;
	UGameplayStatics::GetAllActorsWithTag(this, FName("Burner"), FoundBurners);

	for (AActor* Actor : FoundBurners)
	{
		if (ACookingStation* Burner = Cast<ACookingStation>(Actor))
		{
			Burner->CookingSpeedMultiplier += ModifierValue;
		}
	}
	*/
}

void USkillEffectManagerComponent::ApplyLightFootwork(float ModifierValue)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("===> LightFootwork 発動！ (%.2f)"), ModifierValue));

	// 1. このコンポーネントの親（コントローラー）から、操作中のキャラクターを取得
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (APokaPokaECCCharacter* PlayerChar = Cast<APokaPokaECCCharacter>(PC->GetPawn()))
		{
			// 2. キャラクターの移動コンポーネントを取得して、歩行速度を加算
			if (UCharacterMovementComponent* MovementComp = PlayerChar->GetCharacterMovement())
			{
				MovementComp->MaxWalkSpeed += ModifierValue;

				// デバッグ用：現在の速度を表示
				UE_LOG(LogTemp, Warning, TEXT("移動速度が %f になりました！"), MovementComp->MaxWalkSpeed);
			}
		}
	}
}

void USkillEffectManagerComponent::ApplyMichelinRush(float ModifierValue)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("===> MichelinRush 発動！ (%.2f)"), ModifierValue));

	/*
	// ミシュランラッシュ（VIP客の出現確率アップ）
	if (AChefGameState* GameState = Cast<AChefGameState>(UGameplayStatics::GetGameState(this)))
	{
		GameState->VipCustomerSpawnRate += ModifierValue;
	}
	*/
}

void USkillEffectManagerComponent::ApplyPerfectRiceBoil(float ModifierValue)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("===> PerfectRiceBoil 発動！ (%.2f)"), ModifierValue));

	/*
	// ご飯が焦げなくなるフラグ
	if (AChefGameState* GameState = Cast<AChefGameState>(UGameplayStatics::GetGameState(this)))
	{
		GameState->bCanRiceBurn = false;
	}
	*/
}

void USkillEffectManagerComponent::ApplyPlatingMastery(float ModifierValue)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("===> PlatingMastery 発動！ (%.2f)"), ModifierValue));

	/*
	// 料理完成時の基礎スコア（売上）の加算
	if (AChefGameState* GameState = Cast<AChefGameState>(UGameplayStatics::GetGameState(this)))
	{
		GameState->PlatingValueBonus += ModifierValue;
	}
	*/
}

void USkillEffectManagerComponent::ApplyProMicrowave(float ModifierValue)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("===> ProMicrowave 発動！ (%.2f)"), ModifierValue));

	/*
	// 電子レンジの調理時間短縮
	TArray<AActor*> FoundMicrowaves;
	UGameplayStatics::GetAllActorsWithTag(this, FName("Microwave"), FoundMicrowaves);

	for (AActor* Actor : FoundMicrowaves)
	{
		if (ACookingStation* Microwave = Cast<ACookingStation>(Actor))
		{
			Microwave->CookingSpeedMultiplier += ModifierValue;
		}
	}
	*/
}

void USkillEffectManagerComponent::ApplyRecipeInspiration(float ModifierValue)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("===> RecipeInspiration 発動！ (%.2f)"), ModifierValue));

	/*
	// レシピの閃き（一定確率で料理のランクアップ）
	if (AChefGameState* GameState = Cast<AChefGameState>(UGameplayStatics::GetGameState(this)))
	{
		GameState->RecipeUpgradeProbability += ModifierValue;
	}
	*/
}

void USkillEffectManagerComponent::ApplyServiceHigh(float ModifierValue)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("===> ServiceHigh 発動！ (%.2f)"), ModifierValue));

	/*
	// コンボ倍率の上限解放、またはコンボ猶予時間の延長
	if (AChefGameState* GameState = Cast<AChefGameState>(UGameplayStatics::GetGameState(this)))
	{
		GameState->ComboTimeWindow += ModifierValue;
	}
	*/
}

void USkillEffectManagerComponent::ApplyStarService(float ModifierValue)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("===> StarService 発動！ (%.2f)"), ModifierValue));

	/*
	// 星級サービス（店の評価ゲージの上昇量アップ）
	if (AChefGameState* GameState = Cast<AChefGameState>(UGameplayStatics::GetGameState(this)))
	{
		GameState->ReputationGainMultiplier += ModifierValue;
	}
	*/
}