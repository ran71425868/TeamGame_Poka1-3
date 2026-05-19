#include "BobNPCCharacter.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Navigation/PathFollowingComponent.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UnrealType.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "ItemHoldComponent.h"

ABobNPCCharacter::ABobNPCCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

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

    ReceiveArea = CreateDefaultSubobject<UBoxComponent>(TEXT("ReceiveArea"));
    ReceiveArea->SetupAttachment(RootComponent);
    ReceiveArea->SetBoxExtent(FVector(60.0f, 60.0f, 60.0f));
    ReceiveArea->SetRelativeLocation(FVector(100.0f, 0.0f, 0.0f));
    ReceiveArea->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void ABobNPCCharacter::BeginPlay()
{
    Super::BeginPlay();

    ReceiveArea->OnComponentBeginOverlap.AddDynamic(this, &ABobNPCCharacter::OnReceiveAreaOverlap);

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
    }
}

void ABobNPCCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentState == ECustomerState::Waiting && TargetCounter)
    {
        FVector Direction = TargetCounter->GetActorLocation() - GetActorLocation();
        Direction.Z = 0.0f;

        if (!Direction.IsNearlyZero())
        {
            FRotator TargetRot = Direction.Rotation();
            SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 6.0f));
        }
    }
}

void ABobNPCCharacter::OnReceiveAreaOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (CurrentState != ECustomerState::Waiting || !OtherActor) return;

    FName FoodTag = NAME_None;
    bool bIsFood = false;

    if (FProperty* Prop = OtherActor->GetClass()->FindPropertyByName(FName("ProvidedFoodTag")))
    {
        if (FNameProperty* NameProp = CastField<FNameProperty>(Prop))
        {
            FoodTag = NameProp->GetPropertyValue_InContainer(OtherActor);
            bIsFood = true;
        }
    }

    if (bIsFood && FoodTag != NAME_None)
    {
        float PriceMult = 1.0f;
        int32 FoodScore = 0;

        if (FProperty* Prop = OtherActor->GetClass()->FindPropertyByName(FName("PriceMultiplier")))
        {
            if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
                PriceMult = FloatProp->GetPropertyValue_InContainer(OtherActor);
        }
        if (FProperty* Prop = OtherActor->GetClass()->FindPropertyByName(FName("ScorePoint")))
        {
            if (FIntProperty* IntProp = CastField<FIntProperty>(Prop))
                FoodScore = IntProp->GetPropertyValue_InContainer(OtherActor);
        }

        OtherActor->Destroy();

        bool bIsCorrectFood = ReceiveFoodAndLeave(FoodTag, PriceMult, FoodScore);

        if (!bIsCorrectFood)
        {
            ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
            if (PlayerChar)
            {
                UItemHoldComponent* HoldComp = PlayerChar->FindComponentByClass<UItemHoldComponent>();
                if (HoldComp)
                {
                    int32 PenaltyPoint = 10;
                    HoldComp->TotalCollectedScore -= PenaltyPoint;
                    if (HoldComp->TotalCollectedScore < 0) HoldComp->TotalCollectedScore = 0;

                    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("【ペナルティ】台に間違った料理を置かれた！ 評価が %d 下がった！"), PenaltyPoint));
                }
            }
        }
    }
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

        // 【追加】カウンターに着いたらタイマー開始
        GetWorld()->GetTimerManager().SetTimer(PatienceTimerHandle, this, &ABobNPCCharacter::OnPatienceDepleted, PatienceTime, false);
    }
}

bool ABobNPCCharacter::ReceiveFoodAndLeave(FName ProvidedFoodTag, float PriceMultiplier, int32 EvaluationScore)
{
    // 【追加】料理を受け取ったらタイマーを止める
    GetWorld()->GetTimerManager().ClearTimer(PatienceTimerHandle);

    CurrentState = ECustomerState::Leaving;

    if (ProvidedFoodTag != DesiredFoodTag)
    {
        FString DebugMsg = FString::Printf(TEXT("Bob: WRONG FOOD! I wanted [%s], but got [%s]!"), *DesiredFoodTag.ToString(), *ProvidedFoodTag.ToString());
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, DebugMsg);
        UE_LOG(LogTemp, Warning, TEXT("%s"), *DebugMsg);

        MoveToDestination(ExitLocation);
        return false;
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

    MoveToDestination(ExitLocation);
    return true;
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

// 【追加】待たされすぎて時間切れになった時の処理
void ABobNPCCharacter::OnPatienceDepleted()
{
    if (CurrentState != ECustomerState::Waiting) return;

    CurrentState = ECustomerState::Leaving;

    if (YellingMontage) PlayAnimMontage(YellingMontage);

    FString DebugMsg = TEXT("Bob: TOO LATE! I'm leaving!");
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, DebugMsg);
    UE_LOG(LogTemp, Warning, TEXT("%s"), *DebugMsg);

    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (PlayerChar)
    {
        UItemHoldComponent* HoldComp = PlayerChar->FindComponentByClass<UItemHoldComponent>();
        if (HoldComp)
        {
            int32 PenaltyPoint = 20;
            HoldComp->TotalCollectedScore -= PenaltyPoint;
            if (HoldComp->TotalCollectedScore < 0) HoldComp->TotalCollectedScore = 0;
        }
    }

    MoveToDestination(ExitLocation);
}