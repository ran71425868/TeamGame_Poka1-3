#include "TitleChefAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UTitleChefAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	// 今回は初期化処理は空で問題ありませんが、キャッシュしたい変数などがあればここに書きます。
}

void UTitleChefAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// アニメーションを再生しているキャラクター（オーナー）を安全に取得
	APawn* Pawn = TryGetPawnOwner();
	if (Pawn)
	{
		// 1. 速度（Speed）の計算
		// 3D空間のVelocity（XYZの速度ベクトル）の長さを計算し、Speed変数に格納します
		Speed = Pawn->GetVelocity().Length();

		// 2. 落下判定の取得
		// 汎用の ACharacter クラスとしてキャストし、MovementComponent にアクセスします
		if (ACharacter* Character = Cast<ACharacter>(Pawn))
		{
			if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
			{
				bIsFalling = MovementComp->IsFalling();
			}
		}
	}
}