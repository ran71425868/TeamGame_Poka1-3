#include "SkillEffectManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PokaPokaECCCharacter.h"
#include "PokaPokaECCGameInstance.h"
#include "USkillDataAsset.h" 
#include "CookingStation.h"

USkillEffectManagerComponent::USkillEffectManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USkillEffectManagerComponent::ExecuteSkillEffect(UUSkillDataAsset* AppliedSkill)
{
	if (!AppliedSkill) return;

	FName ID = AppliedSkill->SkillID;

	// デバッグ用に「何が発動したか」を共通で通知
	FString LogMsg = FString::Printf(TEXT("【ID: %s, : %.2f"), *ID.ToString(), AppliedSkill->ModifierValue);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, LogMsg);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *LogMsg);

	// --- 12スキルの分岐処理 ---
	if (ID == FName("ArtisanPatience")) { ApplyArtisanPatience(AppliedSkill->ModifierValue); }
	else if (ID == FName("CozyAtmosphere")) { ApplyCozyAtmosphere(AppliedSkill->ModifierValue); }
	else if (ID == FName("FastFryer")) { ApplyFastFryer(AppliedSkill->ModifierValue); }
	else if (ID == FName("ForbiddenFullCourse")) { ApplyForbiddenFullCourse(AppliedSkill->ModifierValue); }
	else if (ID == FName("GratuityTipping")) { ApplyGratuityTipping(AppliedSkill->ModifierValue); }
	else if (ID == FName("HighHeatBurner")) { ApplyHighHeatBurner(AppliedSkill->ModifierValue); }
	else if (ID == FName("LightFootwork")) { ApplyLightFootwork(AppliedSkill->ModifierValue); }
	else if (ID == FName("PlatingMastery")) { ApplyPlatingMastery(AppliedSkill->ModifierValue); }
	else if (ID == FName("RecipeInspiration")) { ApplyRecipeInspiration(AppliedSkill->ModifierValue); }
	else if (ID == FName("SpeedstersHaste")) { ApplySpeedstersHaste(AppliedSkill->ModifierValue); }
	else if (ID == FName("StarService")) { ApplyStarService(AppliedSkill->ModifierValue); }
	else if (ID == FName("UltraEfficiency")) { ApplyUltraEfficiency(AppliedSkill->ModifierValue); }
	else
	{
		UE_LOG(LogTemp, Error, TEXT("er:%s"), *ID.ToString());
	}
}

// =========================================================================
// 各スキルの具体的な効果処理
// =========================================================================

void USkillEffectManagerComponent::ApplyArtisanPatience(float ModifierValue)
{
	if (UPokaPokaECCGameInstance* GI = Cast<UPokaPokaECCGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->CustomerPatienceBonus += ModifierValue;
	}
}

void USkillEffectManagerComponent::ApplySpeedstersHaste(float ModifierValue)
{
	if (UPokaPokaECCGameInstance* GI = Cast<UPokaPokaECCGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->MovementSpeedBonus += ModifierValue;
	}

	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (APokaPokaECCCharacter* PlayerChar = Cast<APokaPokaECCCharacter>(PC->GetPawn()))
		{
			if (UCharacterMovementComponent* MovementComp = PlayerChar->GetCharacterMovement())
			{
				MovementComp->MaxWalkSpeed += ModifierValue;
			}
		}
	}
}

void USkillEffectManagerComponent::ApplyUltraEfficiency(float ModifierValue)
{
	if (UPokaPokaECCGameInstance* GI = Cast<UPokaPokaECCGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->InteractSpeedMultiplier += ModifierValue;
	}

	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (APokaPokaECCCharacter* PlayerChar = Cast<APokaPokaECCCharacter>(PC->GetPawn()))
		{
			// PlayerChar->InteractSpeedMultiplier += ModifierValue;
		}
	}
}

void USkillEffectManagerComponent::ApplyCozyAtmosphere(float ModifierValue)
{
	if (UPokaPokaECCGameInstance* GI = Cast<UPokaPokaECCGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->TipProbabilityBonus += ModifierValue;
	}
}

void USkillEffectManagerComponent::ApplyFastFryer(float ModifierValue)
{
	if (UPokaPokaECCGameInstance* GI = Cast<UPokaPokaECCGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->FryerSpeedMultiplier += ModifierValue;
	}

	TArray<AActor*> FoundStations;
	UGameplayStatics::GetAllActorsOfClass(this, ACookingStation::StaticClass(), FoundStations);

	for (AActor* Actor : FoundStations)
	{
		if (ACookingStation* Station = Cast<ACookingStation>(Actor))
		{
			if (Station->ActorHasTag(FName("Fryer")))
			{
				// Station->CookingSpeedMultiplier += ModifierValue;
			}
		}
	}
}

void USkillEffectManagerComponent::ApplyForbiddenFullCourse(float ModifierValue)
{
	if (UPokaPokaECCGameInstance* GI = Cast<UPokaPokaECCGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->bIsForbiddenMenuUnlocked = true;
	}
}

void USkillEffectManagerComponent::ApplyGratuityTipping(float ModifierValue)
{
	if (UPokaPokaECCGameInstance* GI = Cast<UPokaPokaECCGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->TipAmountMultiplier += ModifierValue;
	}
}

void USkillEffectManagerComponent::ApplyHighHeatBurner(float ModifierValue)
{
	if (UPokaPokaECCGameInstance* GI = Cast<UPokaPokaECCGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->BurnerSpeedMultiplier += ModifierValue;
	}

	TArray<AActor*> FoundStations;
	UGameplayStatics::GetAllActorsOfClass(this, ACookingStation::StaticClass(), FoundStations);
	for (AActor* Actor : FoundStations)
	{
		if (ACookingStation* Station = Cast<ACookingStation>(Actor))
		{
			if (Station->ActorHasTag(FName("Burner")))
			{
				// Station->CookingSpeedMultiplier += ModifierValue;
			}
		}
	}
}

void USkillEffectManagerComponent::ApplyLightFootwork(float ModifierValue)
{
	if (UPokaPokaECCGameInstance* GI = Cast<UPokaPokaECCGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->MovementSpeedBonus += ModifierValue;
	}

	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (APokaPokaECCCharacter* PlayerChar = Cast<APokaPokaECCCharacter>(PC->GetPawn()))
		{
			if (UCharacterMovementComponent* MovementComp = PlayerChar->GetCharacterMovement())
			{
				MovementComp->MaxWalkSpeed += ModifierValue;
			}
		}
	}
}

void USkillEffectManagerComponent::ApplyPlatingMastery(float ModifierValue)
{
	if (UPokaPokaECCGameInstance* GI = Cast<UPokaPokaECCGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->PlatingValueBonus += ModifierValue;
	}
}

void USkillEffectManagerComponent::ApplyRecipeInspiration(float ModifierValue)
{
	if (UPokaPokaECCGameInstance* GI = Cast<UPokaPokaECCGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->RecipeUpgradeProbability += ModifierValue;
	}
}

void USkillEffectManagerComponent::ApplyStarService(float ModifierValue)
{
	if (UPokaPokaECCGameInstance* GI = Cast<UPokaPokaECCGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->ReputationGainMultiplier += ModifierValue;
	}
}