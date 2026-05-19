#include "TitleChefNPC.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "ItemSpawner.h"
#include "CookingStation.h"
#include "BobNPCCharacter.h"

ATitleChefNPC::ATitleChefNPC()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	CurrentStep = 0;
	CurrentCycle = 0; // 最初はコンロからスタート
	HeldItem = nullptr;

	bIsLerpingItem = false;
	bIsLerpingToStation = false;

	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
}

void ATitleChefNPC::BeginPlay()
{
	Super::BeginPlay();
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UPathFollowingComponent* PathComp = AIController->GetPathFollowingComponent())
		{
			PathComp->OnRequestFinished.AddUObject(this, &ATitleChefNPC::OnMoveCompleted);
		}
	}
	MoveToFridge();
}

void ATitleChefNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. 手元へ吸い付く処理
	if (bIsLerpingItem && HeldItem)
	{
		FVector TargetLoc = GetMesh()->GetSocketLocation(FName("hand_r"));
		FVector CurrentLoc = HeldItem->GetActorLocation();
		FVector NewLoc = FMath::VInterpTo(CurrentLoc, TargetLoc, DeltaTime, 10.0f);
		HeldItem->SetActorLocation(NewLoc);

		if (FVector::Dist(NewLoc, TargetLoc) < 5.0f)
		{
			HeldItem->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("hand_r"));
			bIsLerpingItem = false;
		}
	}

	// 2. 現在アクティブな調理器具へ吸い付く処理
	AActor* ActiveStation = GetCurrentStationActor();
	if (bIsLerpingToStation && HeldItem && ActiveStation)
	{
		FVector TargetLoc = ActiveStation->GetActorLocation() + FVector(0.0f, 0.0f, 30.0f);
		FVector CurrentLoc = HeldItem->GetActorLocation();
		FVector NewLoc = FMath::VInterpTo(CurrentLoc, TargetLoc, DeltaTime, 10.0f);
		HeldItem->SetActorLocation(NewLoc);

		if (FVector::Dist(NewLoc, TargetLoc) < 10.0f)
		{
			bIsLerpingToStation = false;
			if (ACookingStation* Station = Cast<ACookingStation>(ActiveStation))
			{
				if (Station->PlaceItem(HeldItem)) HeldItem = nullptr;
			}
		}
	}

	// 3. お金のマグネット回収処理
	if (TargetMoneyClass)
	{
		TArray<AActor*> FoundMoney;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), TargetMoneyClass, FoundMoney);
		for (AActor* Money : FoundMoney)
		{
			FVector TargetLoc = GetActorLocation();
			FVector MoneyLoc = Money->GetActorLocation();

			if (FVector::Dist(TargetLoc, MoneyLoc) < 50.0f)
			{
				Money->Destroy();
			}
			else
			{
				Money->SetActorLocation(FMath::VInterpTo(MoneyLoc, TargetLoc, DeltaTime, 8.0f));
			}
		}
	}
}

void ATitleChefNPC::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (Result.IsSuccess())
	{
		if (CurrentStep == 0) TakeIngredient();
		else if (CurrentStep == 1) StartCooking();
		else if (CurrentStep == 2) ServeFood();
	}
}

// ---------------------------------------------------------
// サイクルに応じたアクタ切り替え用ヘルパー関数
// ---------------------------------------------------------
AActor* ATitleChefNPC::GetCurrentFridgeMoveTarget() const
{
	if (CurrentCycle == 0) return StoveFridgeMoveTarget;
	if (CurrentCycle == 1) return FryerFridgeMoveTarget;
	return RiceFridgeMoveTarget;
}

AActor* ATitleChefNPC::GetCurrentFridgeActor() const
{
	if (CurrentCycle == 0) return StoveFridgeActor;
	if (CurrentCycle == 1) return FryerFridgeActor;
	return RiceFridgeActor;
}

AActor* ATitleChefNPC::GetCurrentStationMoveTarget() const
{
	if (CurrentCycle == 0) return StoveMoveTarget;
	if (CurrentCycle == 1) return FryerMoveTarget;
	return RiceMoveTarget;
}

AActor* ATitleChefNPC::GetCurrentStationActor() const
{
	if (CurrentCycle == 0) return StoveActor;
	if (CurrentCycle == 1) return FryerActor;
	return RiceCookerActor;
}

// ---------------------------------------------------------
// 1. 冷蔵庫での取り出し（共通ロジック）
// ---------------------------------------------------------
void ATitleChefNPC::MoveToFridge()
{
	CurrentStep = 0;
	AActor* Target = GetCurrentFridgeMoveTarget();
	if (Target)
	{
		if (AAIController* AIController = Cast<AAIController>(GetController()))
			AIController->MoveToActor(Target, 50.0f);
	}
}

void ATitleChefNPC::TakeIngredient()
{
	AActor* Fridge = GetCurrentFridgeActor();
	if (Fridge)
	{
		FRotator TargetRot = (Fridge->GetActorLocation() - GetActorLocation()).Rotation();
		TargetRot.Pitch = 0.0f; TargetRot.Roll = 0.0f;
		SetActorRotation(TargetRot);
	}
	GetWorld()->GetTimerManager().SetTimer(ActionTimer, this, &ATitleChefNPC::GrabIngredient, 0.8f, false);
}

void ATitleChefNPC::GrabIngredient()
{
	AActor* Fridge = GetCurrentFridgeActor();
	if (Fridge)
	{
		if (AItemSpawner* Spawner = Cast<AItemSpawner>(Fridge))
		{
			HeldItem = Spawner->SpawnItem();
			if (HeldItem)
			{
				if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(HeldItem->GetRootComponent()))
				{
					PrimComp->SetSimulatePhysics(false);
					PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				}
				bIsLerpingItem = true;
			}
		}
	}
	GetWorld()->GetTimerManager().SetTimer(ActionTimer, this, &ATitleChefNPC::MoveToStation, 0.5f, false);
}

// ---------------------------------------------------------
// 2. 調理器具での処理（共通ロジック）
// ---------------------------------------------------------
void ATitleChefNPC::MoveToStation()
{
	CurrentStep = 1;
	AActor* Target = GetCurrentStationMoveTarget();
	if (Target)
	{
		if (AAIController* AIController = Cast<AAIController>(GetController()))
			AIController->MoveToActor(Target, 50.0f);
	}
}

void ATitleChefNPC::StartCooking()
{
	AActor* Station = GetCurrentStationActor();
	if (Station)
	{
		FRotator TargetRot = (Station->GetActorLocation() - GetActorLocation()).Rotation();
		TargetRot.Pitch = 0.0f; TargetRot.Roll = 0.0f;
		SetActorRotation(TargetRot);
	}
	GetWorld()->GetTimerManager().SetTimer(ActionTimer, this, &ATitleChefNPC::PutFoodOnStation, 0.5f, false);
}

void ATitleChefNPC::PutFoodOnStation()
{
	float WaitTime = 5.2f;
	AActor* ActiveStation = GetCurrentStationActor();

	if (ActiveStation && HeldItem)
	{
		HeldItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		bIsLerpingToStation = true;

		if (ACookingStation* Station = Cast<ACookingStation>(ActiveStation))
		{
			if (FProperty* Prop = Station->GetClass()->FindPropertyByName(FName("CookTime")))
			{
				if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
					WaitTime = FloatProp->GetPropertyValue_InContainer(Station) + 1.0f;
			}
		}
	}
	GetWorld()->GetTimerManager().SetTimer(ActionTimer, this, &ATitleChefNPC::PickupFood, WaitTime, false);
}

void ATitleChefNPC::PickupFood()
{
	AActor* ActiveStation = GetCurrentStationActor();
	if (ActiveStation)
	{
		if (ACookingStation* Station = Cast<ACookingStation>(ActiveStation))
		{
			AActor* RetrievedItem = Station->RetrieveItem();
			if (RetrievedItem)
			{
				HeldItem = RetrievedItem;
				if (HeldItem->GetRootComponent()) HeldItem->GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

				if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(HeldItem->GetRootComponent()))
				{
					PrimComp->SetSimulatePhysics(false);
					PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				}
				bIsLerpingItem = true;
			}
		}
	}
	GetWorld()->GetTimerManager().SetTimer(ActionTimer, this, &ATitleChefNPC::MoveToCounter, 0.5f, false);
}

// ---------------------------------------------------------
// 3. カウンターでの処理（共通ロジック）
// ---------------------------------------------------------
void ATitleChefNPC::MoveToCounter()
{
	CurrentStep = 2;
	if (CounterMoveTarget)
	{
		if (AAIController* AIController = Cast<AAIController>(GetController()))
			AIController->MoveToActor(CounterMoveTarget, 50.0f);
	}
}

void ATitleChefNPC::ServeFood()
{
	if (CounterMoveTarget)
	{
		FRotator TargetRot = (CounterMoveTarget->GetActorLocation() - GetActorLocation()).Rotation();
		TargetRot.Pitch = 0.0f; TargetRot.Roll = 0.0f;
		SetActorRotation(TargetRot);
	}

	TArray<AActor*> FoundBobs;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABobNPCCharacter::StaticClass(), FoundBobs);
	bool bBobFound = false;

	for (AActor* Actor : FoundBobs)
	{
		ABobNPCCharacter* Customer = Cast<ABobNPCCharacter>(Actor);
		if (CounterMoveTarget && Customer && Customer->CurrentState == ECustomerState::Waiting && FVector::Distance(Actor->GetActorLocation(), CounterMoveTarget->GetActorLocation()) < 300.0f)
		{
			bBobFound = true;
			if (HeldItem)
			{
				FName FoodTag = NAME_None;
				float PriceMult = 1.0f;
				int32 FoodScore = 0;

				if (FProperty* Prop = HeldItem->GetClass()->FindPropertyByName(FName("ProvidedFoodTag")))
				{
					if (FNameProperty* NameProp = CastField<FNameProperty>(Prop)) FoodTag = NameProp->GetPropertyValue_InContainer(HeldItem);
				}
				if (FProperty* Prop = HeldItem->GetClass()->FindPropertyByName(FName("PriceMultiplier")))
				{
					if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop)) PriceMult = FloatProp->GetPropertyValue_InContainer(HeldItem);
				}
				if (FProperty* Prop = HeldItem->GetClass()->FindPropertyByName(FName("ScorePoint")))
				{
					if (FIntProperty* IntProp = CastField<FIntProperty>(Prop)) FoodScore = IntProp->GetPropertyValue_InContainer(HeldItem);
				}

				Customer->ReceiveFoodAndLeave(FoodTag, PriceMult, FoodScore);
				HeldItem->Destroy();
				HeldItem = nullptr;
			}
			break;
		}
	}

	if (bBobFound)
	{
		// 無事にお客さんに渡せたら、サイクル番号を進める (0 -> 1 -> 2 -> 0)
		CurrentCycle = (CurrentCycle + 1) % 3;

		GetWorld()->GetTimerManager().SetTimer(ActionTimer, this, &ATitleChefNPC::MoveToFridge, 1.5f, false);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(ActionTimer, this, &ATitleChefNPC::ServeFood, 0.5f, false);
	}
}