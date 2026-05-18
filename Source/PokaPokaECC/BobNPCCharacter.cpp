#include "BobNPCCharacter.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Navigation/PathFollowingComponent.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UnrealType.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"   // 【追加】
#include "ItemHoldComponent.h"         // 【追加】プレイヤーのスコアを減らすため

ABobNPCCharacter::ABobNPCCharacter()
{
    // 【重要】向きを毎フレーム調整するために、Tick（更新処理）をONにしました！
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

    // 【追加】目の前の「受け取り判定」の箱を作成して設定
    ReceiveArea = CreateDefaultSubobject<UBoxComponent>(TEXT("ReceiveArea"));
    ReceiveArea->SetupAttachment(RootComponent);
    ReceiveArea->SetBoxExtent(FVector(60.0f, 60.0f, 60.0f));     // 箱の大きさ
    ReceiveArea->SetRelativeLocation(FVector(100.0f, 0.0f, 0.0f)); // 客の100cm前に配置
    ReceiveArea->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void ABobNPCCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 箱の判定に何かが触れたら、OnReceiveAreaOverlap 関数を呼ぶように登録
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

    // 【追加】もし待機中 ＆ カウンターが見つかっているなら、カウンターの方へ体を向ける！
    if (CurrentState == ECustomerState::Waiting && TargetCounter)
    {
        FVector Direction = TargetCounter->GetActorLocation() - GetActorLocation();
        Direction.Z = 0.0f; // 上下には傾かないようにする

        if (!Direction.IsNearlyZero())
        {
            FRotator TargetRot = Direction.Rotation();
            // 今の向きから目標の向きへ、滑らかに回転させる（6.0fは回転スピード）
            SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 6.0f));
        }
    }
}

// 【追加】目の前の箱にアイテムが入ってきた時の処理（台に置かれた or 投げられた）
void ABobNPCCharacter::OnReceiveAreaOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 待機中じゃない時、または相手が空っぽの時は無視
    if (CurrentState != ECustomerState::Waiting || !OtherActor) return;

    FName FoodTag = NAME_None;
    bool bIsFood = false;

    // 触れたものが「料理（ProvidedFoodTagを持っているか）」を確認
    if (FProperty* Prop = OtherActor->GetClass()->FindPropertyByName(FName("ProvidedFoodTag")))
    {
        if (FNameProperty* NameProp = CastField<FNameProperty>(Prop))
        {
            FoodTag = NameProp->GetPropertyValue_InContainer(OtherActor);
            bIsFood = true;
        }
    }

    // もし料理だったら、自動で受け取る！
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

        // 置かれた料理を消滅させる
        OtherActor->Destroy();

        // Bobに評価させる
        bool bIsCorrectFood = ReceiveFoodAndLeave(FoodTag, PriceMult, FoodScore);

        // 間違った料理を置かれていたら、ここでペナルティを発生させる
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
    }
}

bool ABobNPCCharacter::ReceiveFoodAndLeave(FName ProvidedFoodTag, float PriceMultiplier, int32 EvaluationScore)
{
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