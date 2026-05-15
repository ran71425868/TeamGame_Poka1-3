#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AutomaticDoor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class POKAPOKAECC_API AAutomaticDoor : public AActor
{
    GENERATED_BODY()

public:
    AAutomaticDoor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* DefaultRoot;

    // --- 新規追加：左右のヒンジ ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* LeftHinge;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RightHinge;

    // --- 既存：左右のドアメッシュ ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* LeftDoorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* RightDoorMesh;

    // --- 判定エリア ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* TriggerZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings")
    float OpenAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings")
    float DoorSpeed = 5.0f;

private:
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    bool bIsOpen;

    // ヒンジの回転を保存する変数
    FRotator LeftClosedRot;
    FRotator LeftOpenRot;
    FRotator RightClosedRot;
    FRotator RightOpenRot;
};