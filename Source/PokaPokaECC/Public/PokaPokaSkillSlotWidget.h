#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "USkillDataAsset.h"
#include "PokaPokaSkillSlotWidget.generated.h"

class UButton;
class UBorder;
class UTextBlock;
class UImage;

UCLASS()
class POKAPOKAECC_API UPokaPokaSkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeSlot(UUSkillDataAsset* InSkillData, int32 InIndex);
	void SetCardFocused(bool bFocused);

	UPROPERTY(BlueprintReadOnly, Category = "Skill Slot")
	UUSkillDataAsset* SkillData;

	UPROPERTY(BlueprintReadOnly, Category = "Skill Slot")
	int32 SlotIndex;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Skill Slot")
	void OnSlotInitialized();

	UPROPERTY(meta = (BindWidget))
	UButton* CardBtn;

	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* BackgroundBorder;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_SkillName;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_Description;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Image_SkillIcon;

	UFUNCTION()
	void OnCardClicked();
};