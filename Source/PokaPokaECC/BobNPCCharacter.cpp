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

    // 初期の基本設定
    MoneySpawnZOffset = 100.0f;
    BaseMoneyAmount = 100; // 基本を100円に設定

    if (GetCapsuleComponent())
    {
        // プレイヤーのインタラクト（ECC_Visibility）の光線を確実にブロックする
        GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    }
}

void ABobNPCCharacter::BeginPlay()
{
    Super::BeginPlay();

    // =========================================================================
    // 【重要】チーム開発対策：BPでの設定ミスを防ぐため、C++側でコリジョンを強制上書き！
    // =========================================================================
    if (GetCapsuleComponent())
    {
        // 1. カプセルの当たり判定自体を強制的に有効化（すり抜け防止）
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        // 2. インタラクトの光線（Visibility）を確実にブロックさせる
        GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    }

    // 念のため、見た目（Skeletal Mesh）の光線判定もブロックにしておく
    if (GetMesh())
    {
        GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    }
    // =========================================================================

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

// 【変更】金額を割合（PriceMultiplier）で計算して、1つのお金BPの内部変数を書き換えます
void ABobNPCCharacter::ReceiveFoodAndLeave(float PriceMultiplier, int32 EvaluationScore)
{
    CurrentState = ECustomerState::Leaving;

    if (BaseMoneyClass && TargetCounter)
    {
        FVector CounterLocation = TargetCounter->GetActorLocation();
        // 確実にカウンターの真上に出す計算
        FVector SpawnLocation = CounterLocation + FVector(0.0f, 0.0f, MoneySpawnZOffset);
        FRotator SpawnRotation = TargetCounter->GetActorRotation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        // 共通のお金BPをスポーン
        AActor* SpawnedMoney = GetWorld()->SpawnActor<AActor>(BaseMoneyClass, SpawnLocation, SpawnRotation, SpawnParams);

        if (SpawnedMoney)
        {
            // ① 割合（倍率）を掛け算して最終的な金額を計算（四捨五入して整数にする）
            int32 FinalAmount = FMath::RoundToInt(BaseMoneyAmount * PriceMultiplier);

            // ② スポーンしたお金のBPの中にある "Amount" という名前の変数（整数型）を探して、計算した金額を無理やり上書きする
            if (FProperty* Property = SpawnedMoney->GetClass()->FindPropertyByName(TEXT("Amount")))
            {
                if (FIntProperty* IntProperty = CastField<FIntProperty>(Property))
                {
                    IntProperty->SetPropertyValue_InContainer(SpawnedMoney, FinalAmount);
                }
            }

            // デバッグメッセージ（金額と倍率と評価を表示）
            FString DebugMsg = FString::Printf(TEXT("Bob: SUCCESS! Spawned Money Val=[%d] (Multiplier: %.2f), Points=[%d]"), FinalAmount, PriceMultiplier, EvaluationScore);
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

void ABobNPCCharacter::ReceiveFoodAndLeaveWithData(int32 FoodPrice, int32 FoodScore)
{
    CurrentState = ECustomerState::Leaving;

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
            // 生成したお金のBPに、トマトの金額とスコアを上書きセットする！
            if (FProperty* MoneyProp = SpawnedMoney->GetClass()->FindPropertyByName(FName("MoneyAmount")))
            {
                if (FIntProperty* IntProp = CastField<FIntProperty>(MoneyProp))
                    IntProp->SetPropertyValue_InContainer(SpawnedMoney, FoodPrice);
            }
            if (FProperty* ScoreProp = SpawnedMoney->GetClass()->FindPropertyByName(FName("ScorePoint")))
            {
                if (FIntProperty* IntProp = CastField<FIntProperty>(ScoreProp))
                    IntProp->SetPropertyValue_InContainer(SpawnedMoney, FoodScore);
            }
            // ※もし変数名がSorePointだった場合の救済措置
            else if (FProperty* SoreProp = SpawnedMoney->GetClass()->FindPropertyByName(FName("SorePoint")))
            {
                if (FIntProperty* IntProp = CastField<FIntProperty>(SoreProp))
                    IntProp->SetPropertyValue_InContainer(SpawnedMoney, FoodScore);
            }

            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Bob: 料理受け取った！(金額:%d, スコア:%d)"), FoodPrice, FoodScore));
        }
    }
    MoveToDestination(ExitLocation);
}