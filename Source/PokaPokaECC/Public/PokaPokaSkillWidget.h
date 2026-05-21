#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "USkillDataAsset.h"
#include "PokaPokaSkillWidget.generated.h"

// 前方宣言
class UTextBlock;
class UButton;
class UBorder;
class UPanelWidget;
class UPokaPokaSkillSlotWidget;

// UIの進行状態を管理する列挙型
UENUM(BlueprintType)
enum class ESkillMenuState : uint8
{
	SelectingSkill UMETA(DisplayName = "Selecting Skill"),
	PopupConfirm   UMETA(DisplayName = "Popup Confirm"),
	ResultSelect   UMETA(DisplayName = "Result Select")
};

UCLASS()
class POKAPOKAECC_API UPokaPokaSkillWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// コンストラクタ
	UPokaPokaSkillWidget(const FObjectInitializer& ObjectInitializer);

	// PlayerControllerから直接呼ばれるエンターキーの受け皿
	UFUNCTION(BlueprintCallable, Category = "Skill UI")
	void OnEnterKeyPressed();

	// ポップアップを表示する関数
	UFUNCTION(BlueprintCallable, Category = "Skill UI")
	void ShowSkillPopup(UUSkillDataAsset* SelectedSkill);

	// 次の日・タイトル選択画面（EndGameBox）を表示する関数
	UFUNCTION(BlueprintCallable, Category = "Skill UI")
	void ShowResultScreen();

	// C++側で自力でカードを生成して並べる関数
	void GenerateUI();

	// ★追加：並べられた全子カードのフォーカス状態をC++側で一括更新する関数
	void UpdateCardFocusVisuals();

	// 生成するカード（WBP_SkillSlot）の設計図を指定する変数
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill UI")
	TSubclassOf<UPokaPokaSkillSlotWidget> SkillSlotClass;

	// ブループリントと共有するC++側の変数
	UPROPERTY(BlueprintReadWrite, Category = "Skill UI")
	int32 CurrentIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Skill UI")
	TArray<UUSkillDataAsset*> AvailableSkills;

protected:
	virtual void NativeConstruct() override;

	// キーボード入力を受け取るためのC++オーバーライド関数
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual bool NativeSupportsKeyboardFocus() const override { return true; }

	// --- 状態管理用変数 ---
	UPROPERTY(BlueprintReadOnly, Category = "Skill UI")
	ESkillMenuState CurrentMenuState = ESkillMenuState::SelectingSkill;

	UPROPERTY(BlueprintReadOnly, Category = "Skill UI")
	int32 PopupSelectedIndex = 0; // 0:決定, 1:キャンセル

	UPROPERTY(BlueprintReadOnly, Category = "Skill UI")
	int32 ResultSelectedIndex = 0; // 0:次の日程へ(Next), 1:タイトルへ(Title)

	// --- 見た目の更新用イベント（主にボタンのフォーカス更新用） ---
	UFUNCTION(BlueprintImplementableEvent, Category = "Skill UI")
	void OnSkillSelectionChanged(int32 NewIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Skill UI")
	void OnPopupSelectionChanged(int32 NewIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Skill UI")
	void OnResultSelectionChanged(int32 NewIndex);

	// --- ブループリントと紐づくUIパーツ ---
	UPROPERTY(meta = (BindWidget))
	UPanelWidget* SkillContainer;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PopupNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PopupDescText;

	UPROPERTY(meta = (BindWidget))
	UBorder* PopupBorder;

	UPROPERTY(meta = (BindWidget))
	UButton* ConfirmBtn;

	UPROPERTY(meta = (BindWidget))
	UButton* CancelBtn;

	UPROPERTY(meta = (BindWidget))
	UPanelWidget* EndGameBox;

	UPROPERTY(meta = (BindWidget))
	UButton* NextBtn;

	UPROPERTY(meta = (BindWidget))
	UButton* TitleBtn;

private:
	// 選択中のスキルを保持する変数
	UPROPERTY()
	UUSkillDataAsset* CurrentSelectedSkill;

	// ボタンクリック時のデリゲート関数
	UFUNCTION()
	void OnConfirmClicked();

	UFUNCTION()
	void OnCancelClicked();

	UFUNCTION()
	void OnNextClicked();

	UFUNCTION()
	void OnTitleClicked();
};