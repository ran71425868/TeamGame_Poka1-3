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
            // yC³zˆø”‚ª•Ï‚í‚Á‚½‚Ì‚ÅAƒfƒoƒbƒO—p‚Æ‚µ‚Äu“™”{(1.0f)A•]‰¿0v‚ð“n‚·‚æ‚¤‚É•ÏX
            Bob->ReceiveFoodAndLeave(1.0f, 0);
        }
    }
}