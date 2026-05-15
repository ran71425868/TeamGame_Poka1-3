// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "PokaPokaECCCharacter.generated.h"

// クラスの前方宣言（コンポーネントを使用するために必要）
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UItemHoldComponent;

UCLASS(config = Game)
class APokaPokaECCCharacter : public ACharacter
{
	GENERATED_BODY()

	/** トップビュー用のカメラブーム */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** フォローカメラ */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** アイテム保持機能を担当するコンポーネント */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UItemHoldComponent* ItemHoldComp;

	/** マッピングコンテキスト */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** ジャンプ入力アクション */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** 移動入力アクション */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** インタラクト（持つ・置く）入力アクション */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

public:
	APokaPokaECCCharacter();

protected:
	/** 移動の入力処理 */
	void Move(const FInputActionValue& Value);

	/** インタラクトの入力処理（コンポーネントを呼び出す） */
	void Interact(const FInputActionValue& Value);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ACharacter interface
	virtual void BeginPlay() override;

public:
	/** カメラブームを返す */
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** カメラを返す */
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** アイテム保持コンポーネントを返す */
	FORCEINLINE class UItemHoldComponent* GetItemHoldComp() const { return ItemHoldComp; }
};