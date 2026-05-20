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
// ★追加：UIを表示するためのインクルード
#include "Components/WidgetComponent.h"
#include "Engine/Texture2D.h"

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

    // ★追加：頭上のポップ（UI）の初期設定
    OrderWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("OrderWidgetComp"));
    OrderWidgetComp->SetupAttachment(RootComponent);
    OrderWidgetComp->SetRelativeLocation(FVector(0.0f, 0.0f, 130.0f)); // 頭の少し上に配置
    OrderWidgetComp->SetWidgetSpace(EWidgetSpace::Screen); // 常にカメラ（プレイヤー）の方を向く
    OrderWidgetComp->SetDrawSize(FVector2D(120.0f, 120.0f));
    OrderWidgetComp->SetVisibility(false); // お店に着くまでは隠しておく
}

void ABobNPCCharacter::BeginPlay()
{
    Super::BeginPlay();

    ReceiveArea->OnComponentBeginOverlap.AddDynamic(this, &ABobNPCCharacter::OnReceiveAreaOverlap);

    // ★追加：設定された「欲しい料理のタグ」から、対応する画像を探しておく
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
        if (TalkingMontage) PlayAnimMontage(TalkingMontage);

        GetWorld()->GetTimerManager().SetTimer(PatienceTimerHandle, this, &ABobNPCCharacter::OnPatienceDepleted, PatienceTime, false);

        // ★追加：カウンターに着いた時、ブループリントに「画像を出して！」と命令し、ポップを表示する
        if (OrderWidgetComp)
        {
            UpdateOrderUI(CurrentOrderIcon);
            OrderWidgetComp->SetVisibility(true);
        }
    }
}

bool ABobNPCCharacter::ReceiveFoodAndLeave(FName ProvidedFoodTag, float PriceMultiplier, int32 EvaluationScore)
{
    GetWorld()->GetTimerManager().ClearTimer(PatienceTimerHandle);
    CurrentState = ECustomerState::Leaving;

    // ★追加：料理をもらって帰る時はポップを隠す
    if (OrderWidgetComp) OrderWidgetComp->SetVisibility(false);

    if (ProvidedFoodTag != DesiredFoodTag)
    {
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

void ABobNPCCharacter::OnPatienceDepleted()
{
    if (CurrentState != ECustomerState::Waiting) return;

    CurrentState = ECustomerState::Leaving;

    // ★追加：怒って帰る時もポップを隠す
    if (OrderWidgetComp) OrderWidgetComp->SetVisibility(false);

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
            HoldComp->TotalCollectedScore -= 20;
            if (HoldComp->TotalCollectedScore < 0) HoldComp->TotalCollectedScore = 0;
        }
    }

    MoveToDestination(ExitLocation);
}