#include "CookingStation.h"
#include "TimerManager.h"
#include "Engine/World.h"

ACookingStation::ACookingStation()
{
	PrimaryActorTick.bCanEverTick = false;

	StationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StationMesh"));
	RootComponent = StationMesh;

	// ソケット1（左用）の作成
	ItemSocket1 = CreateDefaultSubobject<USceneComponent>(TEXT("ItemSocket1"));
	ItemSocket1->SetupAttachment(RootComponent);
	ItemSocket1->SetRelativeLocation(FVector(0.0f, -30.0f, 50.0f));

	// ソケット2（右用）の作成
	ItemSocket2 = CreateDefaultSubobject<USceneComponent>(TEXT("ItemSocket2"));
	ItemSocket2->SetupAttachment(RootComponent);
	ItemSocket2->SetRelativeLocation(FVector(0.0f, 30.0f, 50.0f));

	Tags.Add(FName("CookingStation"));

	CookTime = 5.0f;
	BurnTime = 5.0f;

	CurrentState1 = ECookingState::Empty;
	CurrentState2 = ECookingState::Empty;
	CurrentItem1 = nullptr;
	CurrentItem2 = nullptr;
}

// 食材を置く処理
bool ACookingStation::PlaceItem(AActor* ItemToPlace)
{
	if (ItemToPlace == nullptr) return false;

	// まず左（ソケット1）が空いているかチェック！
	if (CurrentState1 == ECookingState::Empty)
	{
		CurrentItem1 = ItemToPlace;
		CurrentItem1->SetActorLocationAndRotation(ItemSocket1->GetComponentLocation(), ItemSocket1->GetComponentRotation());
		CurrentItem1->AttachToComponent(ItemSocket1, FAttachmentTransformRules::KeepWorldTransform);
		CurrentState1 = ECookingState::Cooking;
		GetWorldTimerManager().SetTimer(CookingTimerHandle1, this, &ACookingStation::OnCookingFinished1, CookTime, false);
		return true;
	}
	// 左が埋まっていたら、右（ソケット2）が空いているかチェック！
	else if (CurrentState2 == ECookingState::Empty)
	{
		CurrentItem2 = ItemToPlace;
		CurrentItem2->SetActorLocationAndRotation(ItemSocket2->GetComponentLocation(), ItemSocket2->GetComponentRotation());
		CurrentItem2->AttachToComponent(ItemSocket2, FAttachmentTransformRules::KeepWorldTransform);
		CurrentState2 = ECookingState::Cooking;
		GetWorldTimerManager().SetTimer(CookingTimerHandle2, this, &ACookingStation::OnCookingFinished2, CookTime, false);
		return true;
	}

	// 両方埋まっている場合は置けない
	return false;
}

// --- ソケット1のタイマー処理 ---
void ACookingStation::OnCookingFinished1()
{
	if (CurrentItem1) CurrentItem1->Destroy();
	if (CookedItemClass)
	{
		CurrentItem1 = GetWorld()->SpawnActor<AActor>(CookedItemClass, ItemSocket1->GetComponentLocation(), ItemSocket1->GetComponentRotation());
		CurrentItem1->AttachToComponent(ItemSocket1, FAttachmentTransformRules::KeepWorldTransform);
	}
	CurrentState1 = ECookingState::Done;
	GetWorldTimerManager().SetTimer(CookingTimerHandle1, this, &ACookingStation::OnBurnt1, BurnTime, false);
}

void ACookingStation::OnBurnt1()
{
	if (CurrentItem1) CurrentItem1->Destroy();
	if (BurntItemClass)
	{
		CurrentItem1 = GetWorld()->SpawnActor<AActor>(BurntItemClass, ItemSocket1->GetComponentLocation(), ItemSocket1->GetComponentRotation());
		CurrentItem1->AttachToComponent(ItemSocket1, FAttachmentTransformRules::KeepWorldTransform);
	}
	CurrentState1 = ECookingState::Burnt;
}

// --- ソケット2のタイマー処理 ---
void ACookingStation::OnCookingFinished2()
{
	if (CurrentItem2) CurrentItem2->Destroy();
	if (CookedItemClass)
	{
		CurrentItem2 = GetWorld()->SpawnActor<AActor>(CookedItemClass, ItemSocket2->GetComponentLocation(), ItemSocket2->GetComponentRotation());
		CurrentItem2->AttachToComponent(ItemSocket2, FAttachmentTransformRules::KeepWorldTransform);
	}
	CurrentState2 = ECookingState::Done;
	GetWorldTimerManager().SetTimer(CookingTimerHandle2, this, &ACookingStation::OnBurnt2, BurnTime, false);
}

void ACookingStation::OnBurnt2()
{
	if (CurrentItem2) CurrentItem2->Destroy();
	if (BurntItemClass)
	{
		CurrentItem2 = GetWorld()->SpawnActor<AActor>(BurntItemClass, ItemSocket2->GetComponentLocation(), ItemSocket2->GetComponentRotation());
		CurrentItem2->AttachToComponent(ItemSocket2, FAttachmentTransformRules::KeepWorldTransform);
	}
	CurrentState2 = ECookingState::Burnt;
}

// 食材を取り出す処理
AActor* ACookingStation::RetrieveItem()
{
	// 1. まず「完成品（Done）」を優先して探す
	if (CurrentState1 == ECookingState::Done)
	{
		AActor* ItemToReturn = CurrentItem1;
		GetWorldTimerManager().ClearTimer(CookingTimerHandle1);
		CurrentItem1 = nullptr;
		CurrentState1 = ECookingState::Empty;
		return ItemToReturn;
	}
	else if (CurrentState2 == ECookingState::Done)
	{
		AActor* ItemToReturn = CurrentItem2;
		GetWorldTimerManager().ClearTimer(CookingTimerHandle2);
		CurrentItem2 = nullptr;
		CurrentState2 = ECookingState::Empty;
		return ItemToReturn;
	}

	// 2. 完成品がなければ「焦げたもの（Burnt）」を探す
	if (CurrentState1 == ECookingState::Burnt)
	{
		AActor* ItemToReturn = CurrentItem1;
		CurrentItem1 = nullptr;
		CurrentState1 = ECookingState::Empty;
		return ItemToReturn;
	}
	else if (CurrentState2 == ECookingState::Burnt)
	{
		AActor* ItemToReturn = CurrentItem2;
		CurrentItem2 = nullptr;
		CurrentState2 = ECookingState::Empty;
		return ItemToReturn;
	}

	// どちらも調理中や空っぽなら何も取り出せない
	return nullptr;
}