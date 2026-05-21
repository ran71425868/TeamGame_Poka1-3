#include "ItemSpawner.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

AItemSpawner::AItemSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	CurrentSelectedIndex = 0;
}

AActor* AItemSpawner::SpawnItem()
{
	// yŒ´ˆö’²¸—pƒfƒoƒbƒOz
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			FString::Printf(TEXT("Spawner Debug -> Array Num: %d, Selected Index: %d"), SpawnItemClasses.Num(), CurrentSelectedIndex));
	}

	if (SpawnItemClasses.Num() == 0) return nullptr;

	if (SpawnItemClasses.IsValidIndex(CurrentSelectedIndex) && SpawnItemClasses[CurrentSelectedIndex])
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FVector SpawnLocation = GetActorLocation() + FVector(0, 0, 50.0f);
		FRotator SpawnRotation = GetActorRotation();

		return GetWorld()->SpawnActor<AActor>(SpawnItemClasses[CurrentSelectedIndex], SpawnLocation, SpawnRotation, SpawnParams);
	}

	return nullptr;
}

void AItemSpawner::CycleSelection(int32 Direction)
{
	if (SpawnItemClasses.Num() <= 1) return;

	CurrentSelectedIndex += Direction;

	if (CurrentSelectedIndex >= SpawnItemClasses.Num())
	{
		CurrentSelectedIndex = 0;
	}
	else if (CurrentSelectedIndex < 0)
	{
		CurrentSelectedIndex = SpawnItemClasses.Num() - 1;
	}

	if (GEngine)
	{
		// yC³z“ú–{Œê‚É‚æ‚é•¶Žš‰»‚¯ƒNƒ‰ƒbƒVƒ…‚ð–h‚®‚½‚ßA‰pŒêi”¼Špj‚É•ÏX‚µ‚Ü‚µ‚½
		FString SelectedName = TEXT("None");
		if (ItemNames.IsValidIndex(CurrentSelectedIndex))
		{
			SelectedName = ItemNames[CurrentSelectedIndex];
		}

		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, FString::Printf(TEXT("Selected Item: %s"), *SelectedName));
	}
}