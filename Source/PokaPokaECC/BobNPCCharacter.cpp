#include "BobNPCCharacter.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Navigation/PathFollowingComponent.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UnrealType.h"
#include "Components/CapsuleComponent.h"

ABobNPCCharacter::ABobNPCCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

    CurrentState = ECustomerState::MovingToShop;
    CurrentPathIndex = 0;
    MoneySpawnZOffset = 100.0f;
    BaseMoneyAmount = 100;

    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    }
}

void ABobNPCCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (!TargetCounterTag.IsNone())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetCounterTag, FoundActors);

        if (FoundActors.Num() > 0)
        {
            TargetCounter = FoundActors[0];

            FString OrderMsg = FString::Printf(TEXT("Bob: FOUND Table! I want to eat [%s]!"), *DesiredFoodTag.ToString());
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, OrderMsg);
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Bob: ERROR! Could not find tag [%s]!"), *TargetCounterTag.ToString()));
        }
    }
}

void ABobNPCCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABobNPCCharacter::MoveToDestination(FVector Destination)
{
    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController && AIController->GetPathFollowingComponent())
    {
        AIController->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(this);
        AIController->GetPathFollowingComponent()->OnRequestFinished.AddUObject(this, &ABobNPCCharacter::OnMoveCompleted);
        AIController->MoveToLocation(Destination, 50.0f);
    }
}

void ABobNPCCharacter::StartPathMovementWithDelay(TArray<FVector> InPathPoints, FVector InExitLocation, float DelayTime)
{
    PathPoints = InPathPoints;
    ExitLocation = InExitLocation;
    CurrentPathIndex = 0;
    CurrentState = ECustomerState::MovingToShop;

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ABobNPCCharacter::ExecutePathMovement, DelayTime, false);
}

void ABobNPCCharacter::ExecutePathMovement()
{
    MoveToNextPathPoint();
}

void ABobNPCCharacter::MoveToNextPathPoint()
{
    if (PathPoints.IsValidIndex(CurrentPathIndex))
    {
        MoveToDestination(PathPoints[CurrentPathIndex]);
    }
    else
    {
        CurrentState = ECustomerState::Waiting;
        if (TalkingMontage) PlayAnimMontage(TalkingMontage);
    }
}

// ÅyïœçXÅzñﬂÇËílÇ bool Ç…ÇµÇ‹ÇµÇΩ
bool ABobNPCCharacter::ReceiveFoodAndLeave(FName ProvidedFoodTag, float PriceMultiplier, int32 EvaluationScore)
{
    CurrentState = ECustomerState::Leaving;

    if (ProvidedFoodTag != DesiredFoodTag)
    {
        FString DebugMsg = FString::Printf(TEXT("Bob: WRONG FOOD! I wanted [%s], but got [%s]!"), *DesiredFoodTag.ToString(), *ProvidedFoodTag.ToString());
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, DebugMsg);
        UE_LOG(LogTemp, Warning, TEXT("%s"), *DebugMsg);

        MoveToDestination(ExitLocation);
        return false; // Åyí«â¡Åzé∏îsÇµÇΩÇ±Ç∆ÇÉvÉåÉCÉÑÅ[Ç…ì`Ç¶ÇÈ
    }

    if (BaseMoneyClass && TargetCounter)
    {
        FVector CounterLocation = TargetCounter->GetActorLocation();
        FVector SpawnLocation = CounterLocation + FVector(0.0f, 0.0f, MoneySpawnZOffset);
        FRotator SpawnRotation = TargetCounter->GetActorRotation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AActor* SpawnedMoney = GetWorld()->SpawnActor<AActor>(BaseMoneyClass, SpawnLocation, SpawnRotation, SpawnParams);

        if (SpawnedMoney)
        {
            int32 FinalAmount = FMath::RoundToInt(BaseMoneyAmount * PriceMultiplier);

            if (FProperty* Property = SpawnedMoney->GetClass()->FindPropertyByName(TEXT("Amount")))
            {
                if (FIntProperty* IntProperty = CastField<FIntProperty>(Property))
                {
                    IntProperty->SetPropertyValue_InContainer(SpawnedMoney, FinalAmount);
                }
            }

            FString DebugMsg = FString::Printf(TEXT("Bob: SUCCESS! Ordered [%s], Paid Val=[%d] (Multiplier: %.2f), Points=[%d]"), *DesiredFoodTag.ToString(), FinalAmount, PriceMultiplier, EvaluationScore);
            GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, DebugMsg);
            UE_LOG(LogTemp, Warning, TEXT("%s"), *DebugMsg);
        }
    }
    else
    {
        if (!BaseMoneyClass)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Bob: ERROR! BaseMoneyClass is NONE!"));
        }
        if (!TargetCounter)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Bob: ERROR! TargetCounter is NULL!"));
        }
    }

    MoveToDestination(ExitLocation);
    return true; // Åyí«â¡Åzê¨å˜ÇµÇΩÇ±Ç∆ÇÉvÉåÉCÉÑÅ[Ç…ì`Ç¶ÇÈ
}

void ABobNPCCharacter::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    if (Result.IsSuccess())
    {
        if (CurrentState == ECustomerState::MovingToShop)
        {
            CurrentPathIndex++;
            MoveToNextPathPoint();
        }
        else if (CurrentState == ECustomerState::Leaving)
        {
            if (OnCustomerLeft.IsBound())
            {
                OnCustomerLeft.Broadcast();
            }
            Destroy();
        }
    }
}