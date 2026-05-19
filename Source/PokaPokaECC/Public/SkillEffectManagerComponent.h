#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
class UUSkillDataAsset; // 前方宣言
#include "SkillEffectManagerComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class POKAPOKAECC_API USkillEffectManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USkillEffectManagerComponent();

	// C++（コントローラー）から呼ばれる、スキル効果を一手に引き受ける窓口
	UFUNCTION(BlueprintCallable, Category = "Skill|Effect")
	void ExecuteSkillEffect(UUSkillDataAsset* AppliedSkill);

protected:
	// --- 全20スキルの効果発動関数 ---

	void ApplyArtisanPatience(float ModifierValue);
	void ApplyCozyAtmosphere(float ModifierValue);
	void ApplyFastFryer(float ModifierValue);
	void ApplyForbiddenFullCourse(float ModifierValue);
	void ApplyGratuityTipping(float ModifierValue);
	void ApplyGreedyTray(float ModifierValue);
	void ApplyHighHeatBurner(float ModifierValue);
	void ApplyLastSpurtRush(float ModifierValue);
	void ApplyLegendaryQueue(float ModifierValue);
	void ApplyLightFootwork(float ModifierValue);
	void ApplyMichelinRush(float ModifierValue);
	void ApplyPerfectRiceBoil(float ModifierValue);
	void ApplyPlatingMastery(float ModifierValue);
	void ApplyProMicrowave(float ModifierValue);
	void ApplyRecipeInspiration(float ModifierValue);
	void ApplyServiceHigh(float ModifierValue);
	void ApplySpeedstersHaste(float ModifierValue);
	void ApplyStarService(float ModifierValue);
	void ApplyUltraEfficiency(float ModifierValue);
};