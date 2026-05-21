#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "USkillDataAsset.h"
#include "PokaPokaSkillWidget.generated.h"

class UTextBlock;
class UButton;
class UBorder;
class UPanelWidget;
class UPokaPokaSkillSlotWidget;
class UImage; // ★画像表示用に追加

UENUM(BlueprintType)
enum class ESkillMenuState : uint8
{
	SelectingSkill UMETA(DisplayName = "Selecting Skill"),
	ResultSelect   UMETA(DisplayName = "Result Select")
};

UCLASS()
class POKAPOKAECC_API UPokaPokaSkillWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPokaPokaSkillWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Skill UI")
	void OnEnterKeyPressed();

	UFUNCTION(BlueprintCallable, Category = "Skill UI")
	void ShowResultScreen();

	void GenerateUI();
	void UpdateCardFocusVisuals();

	// ★追加：テキストや画像、フォーカスを一括で更新する関数
	void UpdateSkillDisplay();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill UI")
	TSubclassOf<UPokaPokaSkillSlotWidget> SkillSlotClass;

	UPROPERTY(BlueprintReadWrite, Category = "Skill UI")
	int32 CurrentIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Skill UI")
	TArray<UUSkillDataAsset*> AvailableSkills;

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual bool NativeSupportsKeyboardFocus() const override { return true; }

	UPROPERTY(BlueprintReadOnly, Category = "Skill UI")
	ESkillMenuState CurrentMenuState = ESkillMenuState::SelectingSkill;

	UPROPERTY(BlueprintReadOnly, Category = "Skill UI")
	int32 ResultSelectedIndex = 0;

	UFUNCTION(BlueprintImplementableEvent, Category = "Skill UI")
	void OnSkillSelectionChanged(int32 NewIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Skill UI")
	void OnResultSelectionChanged(int32 NewIndex);

	UPROPERTY(meta = (BindWidget))
	UPanelWidget* SkillContainer;

	UPROPERTY(meta = (BindWidget))
	UPanelWidget* EndGameBox;

	UPROPERTY(meta = (BindWidget))
	UButton* NextBtn;

	UPROPERTY(meta = (BindWidget))
	UButton* TitleBtn;

	// ==========================================
	// ★追加：移植した詳細情報・背景表示用パーツ
	// ==========================================
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_SelectedSkillName;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_SelectedSkillDescription;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Image_SelectedSkillIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* BackgroundDim;

private:
	UPROPERTY()
	UUSkillDataAsset* CurrentSelectedSkill;

	UFUNCTION()
	void OnNextClicked();

	UFUNCTION()
	void OnTitleClicked();
};