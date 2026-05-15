#include "BobNPCCharacter.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Navigation/PathFollowingComponent.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"

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
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("Bob: FOUND Table with tag [%s]!"), *TargetCounterTag.ToString()));
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

void ABobNPCCharacter::ReceiveFoodAndLeave()
{
    CurrentState = ECustomerState::Leaving;

    if (MoneyClassToSpawn && TargetCounter)
    {
        FVector BobLocation = GetActorLocation();
        FVector CounterLocation = TargetCounter->GetActorLocation();

        FVector SpawnLocation = CounterLocation + FVector(0.0f, 0.0f, MoneySpawnZOffset);
        FRotator SpawnRotation = TargetCounter->GetActorRotation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AActor* SpawnedMoney = GetWorld()->SpawnActor<AActor>(MoneyClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);

        if (SpawnedMoney)
        {
            // 成功メッセージ（緑色）
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Bob: SUCCESS! Spawned Money!"));
        }
    }
    else
    {
        // 失敗メッセージ（赤色）
        if (!MoneyClassToSpawn)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Bob: ERROR! MoneyClassToSpawn is NONE!"));
        }
        if (!TargetCounter)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Bob: ERROR! TargetCounter is NULL when receiving food!"));
        }
    }

    MoveToDestination(ExitLocation);
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