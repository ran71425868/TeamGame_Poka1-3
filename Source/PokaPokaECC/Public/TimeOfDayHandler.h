#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimeOfDayHandler.generated.h"

class ADirectionalLight;
class ASkyLight;

UCLASS()
class POKAPOKAECC_API ATimeOfDayHandler : public AActor
{
    GENERATED_BODY()

public:
    ATimeOfDayHandler();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, Category = "Lighting")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, Category = "Lighting")
    ASkyLight* SkyLight;

    UPROPERTY(EditAnywhere, Category = "Lighting")
    float DayLengthInSeconds = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    class APostProcessVolume* KitchenVolume;
};