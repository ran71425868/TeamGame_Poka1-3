#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Navigation/PathFollowingComponent.h"
#include "BobNPCCharacter.generated.h"

class UAnimMontage;
class UBoxComponent;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    FName TargetCounterTag;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop")
    AActor* TargetCounter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    FName DesiredFoodTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    TSubclassOf<AActor> BaseMoneyClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    int32 BaseMoneyAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    float MoneySpawnZOffset;

    // --- Åyí«â¡Åzéûä‘åoâﬂÇ≈ãAÇÈÇΩÇﬂÇÃê›íË ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    float PatienceTime = 30.0f;

    FTimerHandle PatienceTimerHandle;

    UFUNCTION()
    void OnPatienceDepleted();
    // ----------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop")
    UBoxComponent* ReceiveArea;

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
    bool ReceiveFoodAndLeave(FName ProvidedFoodTag, float PriceMultiplier, int32 EvaluationScore);

    UFUNCTION()
    void OnReceiveAreaOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    UFUNCTION()
    void ExecutePathMovement();

    void MoveToNextPathPoint();

    void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);
};