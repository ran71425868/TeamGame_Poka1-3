// Copyright Epic Games, Inc. All Rights Reserved.

#include "PokaPokaECCCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

// 分割したコンポーネントをインクルード
#include "ItemHoldComponent.h"

APokaPokaECCCharacter::APokaPokaECCCharacter()
{
	// コリジョンカプセルのサイズ設定
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// コントローラーの回転に合わせてキャラクターを回転させない（トップビュー用）
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// キャラクターの移動設定：入力方向にキャラクターが向き、歩行速度などを設定
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// トップビュー用：カメラブーム（SpringArm）の作成
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 1200.0f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bDoCollisionTest = false;

	// カメラの回転を固定（親の回転を引き継がない）
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;

	// トップビュー用：フォローカメラの作成
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// ★追加：アイテム保持コンポーネントの作成
	ItemHoldComp = CreateDefaultSubobject<UItemHoldComponent>(TEXT("ItemHoldComp"));
}

void APokaPokaECCCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Enhanced Input の入力マッピングを追加
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void APokaPokaECCCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// ジャンプ
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// 移動
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APokaPokaECCCharacter::Move);

		// ★インタラクト（コンポーネントの機能を呼び出す）
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APokaPokaECCCharacter::Interact);
	}
}

void APokaPokaECCCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// 固定カメラ（CameraManager）の回転を基準に移動方向を決定
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (APlayerCameraManager* CameraManager = PC->PlayerCameraManager)
			{
				const FRotator CameraRotation = CameraManager->GetCameraRotation();
				const FRotator YawRotation(0, CameraRotation.Yaw, 0);

				const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
				const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

				AddMovementInput(ForwardDirection, MovementVector.Y);
				AddMovementInput(RightDirection, MovementVector.X);
			}
		}
	}
}

void APokaPokaECCCharacter::Interact(const FInputActionValue& Value)
{
	// キャラクター自身でロジックを持たず、コンポーネントに委託する
	if (ItemHoldComp)
	{
		ItemHoldComp->PrimaryInteract();
	}
}