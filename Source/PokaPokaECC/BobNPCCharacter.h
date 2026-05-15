#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Navigation/PathFollowingComponent.h"
#include "BobNPCCharacter.generated.h"

class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCustomerLeftDelegate);

UENUM(BlueprintType)
enum class ECustomerState : uint8
{
    MovingToShop UMETA(DisplayName = "Moving To Shop"),
    Waiting      UMETA(DisplayName = "Waiting"),
    Leaving      UMETA(DisplayName = "Leaving")
};

UCLASS()
class POKAPOKAECC_API ABobNPCCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ABobNPCCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
    ECustomerState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    TArray<FVector> PathPoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    int32 CurrentPathIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    FVector ExitLocation;

    // ==========================================
    // ここからが設定用の変数です
    // ==========================================

    // 【ここに入力！】探す机の名前（タグ）を文字で入力する場所
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    FName TargetCounterTag;

    // 見つけた机が自動で入る場所（ReadOnlyなのでエディタからは触れません）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop")
    AActor* TargetCounter;

    // 出すお金のBP
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    TSubclassOf<AActor> MoneyClassToSpawn;

    // お金を出す高さ（机からどれくらい浮かすか）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    float MoneySpawnZOffset;

    // ==========================================

    UPROPERTY(BlueprintAssignable, Category = "Event")
    FOnCustomerLeftDelegate OnCustomerLeft;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* TalkingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* YellingMontage;

    UFUNCTION(BlueprintCallable, Category = "AI")
    void MoveToDestination(FVector Destination);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartPathMovementWithDelay(TArray<FVector> InPathPoints, FVector InExitLocation, float DelayTime);

    UFUNCTION(BlueprintCallable, Category = "Event")
    void ReceiveFoodAndLeave();

private:
    UFUNCTION()
    void ExecutePathMovement();

    void MoveToNextPathPoint();

    void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);
};