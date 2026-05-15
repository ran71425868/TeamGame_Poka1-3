#include "BobSpawner.h"
#include "BobNPCCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/InputComponent.h"

ABobSpawner::ABobSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
    DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
    RootComponent = DefaultRoot;
    AutoReceiveInput = EAutoReceiveInput::Player0;
}

void ABobSpawner::BeginPlay()
{
    Super::BeginPlay();
    SpawnAndMoveBob();

    if (InputComponent)
    {
        InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &ABobSpawner::TestMakeBobLeave);
    }
}

void ABobSpawner::SpawnAndMoveBob()
{
    if (!BobClassToSpawn) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ABobNPCCharacter* SpawnedBob = GetWorld()->SpawnActor<ABobNPCCharacter>(BobClassToSpawn, GetActorLocation(), GetActorRotation(), SpawnParams);

    if (SpawnedBob)
    {
        // yíœzSpawnedBob->TargetCounter = this->TargetCounter;
        // ‚±‚ê‚ðÁ‚·‚±‚Æ‚ÅABob–{l‚ÌBeginPlay‚Å‚ÌŒŸõ‚É”C‚¹‚é‚æ‚¤‚É‚È‚è‚Ü‚·B

        SpawnedBob->OnCustomerLeft.AddDynamic(this, &ABobSpawner::SpawnAndMoveBob);

        TArray<FVector> WorldPathLocations;
        for (FVector Loc : PathLocations)
        {
            WorldPathLocations.Add(GetTransform().TransformPosition(Loc));
        }

        SpawnedBob->StartPathMovementWithDelay(WorldPathLocations, GetTransform().TransformPosition(ExitLocation), 2.0f);
    }
}

void ABobSpawner::TestMakeBobLeave()
{
    TArray<AActor*> FoundBobs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABobNPCCharacter::StaticClass(), FoundBobs);

    for (AActor* Actor : FoundBobs)
    {
        if (ABobNPCCharacter* Bob = Cast<ABobNPCCharacter>(Actor))
        {
            Bob->ReceiveFoodAndLeave();
        }
    }
}