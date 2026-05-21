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
#include "Components/WidgetComponent.h"
#include "Engine/Texture2D.h"
#include "PokaPokaECCGameMode.h"

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

    OrderWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("OrderWidgetComp"));
    OrderWidgetComp->SetupAttachment(RootComponent);
    OrderWidgetComp->SetRelativeLocation(FVector(0.0f, 0.0f, 130.0f));
    OrderWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
    OrderWidgetComp->SetDrawSize(FVector2D(120.0f, 120.0f));
    OrderWidgetComp->SetVisibility(false);
}

void ABobNPCCharacter::BeginPlay()
{
    Super::BeginPlay();

    ReceiveArea->OnComponentBeginOverlap.AddDynamic(this, &ABobNPCCharacter::OnReceiveAreaOverlap);

    if (APokaPokaECCGameMode* GM = Cast<APokaPokaECCGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        int32 Today = GM->CurrentDay;
        TArray<FName> AvailableFoods;

        for (const TPair<FName, int32>& Pair : FoodUnlockDays)
        {
            if (Today >= Pair.Value)
            {
                AvailableFoods.Add(Pair.Key);
            }
        }

        if (AvailableFoods.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, AvailableFoods.Num() - 1);
            DesiredFoodTag = AvailableFoods[RandomIndex];
        }
    }

    if (FoodIconMap.Contains(DesiredFoodTag))
    {
        CurrentOrderIcon = FoodIconMap[DesiredFoodTag];
    }

    if (!TargetCounterTag.IsNone())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetCounterTag, FoundActors);

        if (FoundActors.Num() > 0)
        {
            TargetCounter = FoundActors[0];
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
                    HoldComp->TotalCollectedScore -= 10;
                    if (HoldComp->TotalCollectedScore < 0) HoldComp->TotalCollectedScore = 0;
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

        // 到着時に少し待つアニメーションなどを鳴らしたい場合はそのまま残してOKです
        if (TalkingMontage) PlayAnimMontage(TalkingMontage);

        GetWorld()->GetTimerManager().SetTimer(PatienceTimerHandle, this, &ABobNPCCharacter::OnPatienceDepleted, PatienceTime, false);

        if (OrderWidgetComp)
        {
            UpdateOrderUI(CurrentOrderIcon);
            OrderWidgetComp->SetVisibility(true);
        }
    }
}

// ★追加：タイマーによって実際に帰る処理
void ABobNPCCharacter::LeaveShop()
{
    MoveToDestination(ExitLocation);
}

bool ABobNPCCharacter::ReceiveFoodAndLeave(FName ProvidedFoodTag, float PriceMultiplier, int32 EvaluationScore)
{
    GetWorld()->GetTimerManager().ClearTimer(PatienceTimerHandle);
    CurrentState = ECustomerState::Leaving;

    if (OrderWidgetComp) OrderWidgetComp->SetVisibility(false);

    float WaitTime = 0.1f; // アニメーションがない場合の最小待機時間

    // ❌ 違う商品を渡された場合
    if (ProvidedFoodTag != DesiredFoodTag)
    {
        if (YellingMontage)
        {
            WaitTime = PlayAnimMontage(YellingMontage); // 怒るアニメーションを再生し、長さを取得
        }

        // アニメーションが終わったら帰る
        FTimerHandle LeaveTimer;
        GetWorld()->GetTimerManager().SetTimer(LeaveTimer, this, &ABobNPCCharacter::LeaveShop, FMath::Max(WaitTime, 0.1f), false);
        return false;
    }

    // ⭕ 正しい商品を渡された場合
    if (TalkingMontage)
    {
        WaitTime = PlayAnimMontage(TalkingMontage); // Talkingアニメーションを再生し、長さを取得
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
        }
    }

    // アニメーションが終わったら帰る
    FTimerHandle LeaveTimer;
    GetWorld()->GetTimerManager().SetTimer(LeaveTimer, this, &ABobNPCCharacter::LeaveShop, FMath::Max(WaitTime, 0.1f), false);
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

void ABobNPCCharacter::OnPatienceDepleted()
{
    if (CurrentState != ECustomerState::Waiting) return;

    CurrentState = ECustomerState::Leaving;

    if (OrderWidgetComp) OrderWidgetComp->SetVisibility(false);

    float WaitTime = 0.1f;
    if (YellingMontage)
    {
        WaitTime = PlayAnimMontage(YellingMontage); // 怒るアニメーションを再生
    }

    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (PlayerChar)
    {
        UItemHoldComponent* HoldComp = PlayerChar->FindComponentByClass<UItemHoldComponent>();
        if (HoldComp)
        {
            HoldComp->TotalCollectedScore -= 20;
            if (HoldComp->TotalCollectedScore < 0) HoldComp->TotalCollectedScore = 0;
        }
    }

    // アニメーションが終わってから帰る
    FTimerHandle LeaveTimer;
    GetWorld()->GetTimerManager().SetTimer(LeaveTimer, this, &ABobNPCCharacter::LeaveShop, FMath::Max(WaitTime, 0.1f), false);
}