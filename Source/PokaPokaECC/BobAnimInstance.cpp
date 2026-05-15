#include "BobAnimInstance.h"
#include "GameFramework/Pawn.h"

void UBobAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (APawn* OwnerPawn = TryGetPawnOwner())
    {
        Speed = OwnerPawn->GetVelocity().Size();
    }
    else
    {
        Speed = 0.0f;
    }
}