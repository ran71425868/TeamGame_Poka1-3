#include "ItemHoldComponent.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CookingStation.h"
#include "Engine/Engine.h"
#include "UObject/UnrealType.h"
#include "BobNPCCharacter.h"

UItemHoldComponent::UItemHoldComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    MagnetRadius = 300.0f;
    MagnetSpeed = 10.0f;
    CollectionDistance = 70.0f;
}

void UItemHoldComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        GridHighlightMesh = NewObject<UStaticMeshComponent>(OwnerCharacter, TEXT("GridHighlightMesh"));
        if (GridHighlightMesh)
        {
            GridHighlightMesh->RegisterComponent();
            GridHighlightMesh->AttachToComponent(OwnerCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
            GridHighlightMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            GridHighlightMesh->SetHiddenInGame(true);

            if (HighlightMeshAsset)
            {
                GridHighlightMesh->SetStaticMesh(HighlightMeshAsset);
            }
            if (HighlightMaterial)
            {
                GridHighlightMesh->SetMaterial(0, HighlightMaterial);
            }
        }
    }
}

void UItemHoldComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsItemSnapping && HeldItem && OwnerCharacter)
    {
        FVector CurrentLoc = HeldItem->GetRootComponent()->GetRelativeLocation();
        FRotator CurrentRot = HeldItem->GetRootComponent()->GetRelativeRotation();

        FVector NewLoc = FMath::VInterpTo(CurrentLoc, FVector::ZeroVector, DeltaTime, ItemSnapSpeed);
        FRotator NewRot = FMath::RInterpTo(CurrentRot, FRotator::ZeroRotator, DeltaTime, ItemSnapSpeed);

        HeldItem->GetRootComponent()->SetRelativeLocationAndRotation(NewLoc, NewRot);

        if (CurrentLoc.Equals(FVector::ZeroVector, 2.0f))
        {
            HeldItem->GetRootComponent()->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
            bIsItemSnapping = false;
        }
    }

    if (bIsItemPlacing && PlacingItem)
    {
        FVector CurrentLoc = PlacingItem->GetActorLocation();
        FRotator CurrentRot = PlacingItem->GetActorRotation();

        FVector NewLoc = FMath::VInterpTo(CurrentLoc, PlaceTargetLocation, DeltaTime, ItemSnapSpeed);
        FRotator NewRot = FMath::RInterpTo(CurrentRot, PlaceTargetRotation, DeltaTime, ItemSnapSpeed);

        PlacingItem->SetActorLocationAndRotation(NewLoc, NewRot);

        if (CurrentLoc.Equals(PlaceTargetLocation, 2.0f))
        {
            PlacingItem->SetActorLocationAndRotation(PlaceTargetLocation, PlaceTargetRotation);
            bIsItemPlacing = false;
            PlacingItem = nullptr;
        }
    }
    UpdateGridHighlight();

    if (OwnerCharacter)
    {
        FVector PlayerLoc = OwnerCharacter->GetActorLocation();

        FCollisionShape MagnetSphere = FCollisionShape::MakeSphere(MagnetRadius);
        TArray<FOverlapResult> MagnetOverlaps;
        FCollisionQueryParams MagnetParams;
        MagnetParams.AddIgnoredActor(OwnerCharacter);

        GetWorld()->OverlapMultiByChannel(MagnetOverlaps, PlayerLoc, FQuat::Identity, ECC_Visibility, MagnetSphere, MagnetParams);

        for (const FOverlapResult& Overlap : MagnetOverlaps)
        {
            AActor* HitActor = Overlap.GetActor();
            if (HitActor && HitActor->ActorHasTag("Money"))
            {
                float Distance = FVector::Dist(PlayerLoc, HitActor->GetActorLocation());

                if (Distance <= CollectionDistance)
                {
                    int32 CollectedMoney = 0;
                    int32 CollectedScore = 0;

                    if (FProperty* MoneyProp = HitActor->GetClass()->FindPropertyByName(FName("Amount")))
                    {
                        if (FIntProperty* IntProp = CastField<FIntProperty>(MoneyProp))
                            CollectedMoney = IntProp->GetPropertyValue_InContainer(HitActor);
                    }
                    if (FProperty* ScoreProp = HitActor->GetClass()->FindPropertyByName(FName("ScorePoint")))
                    {
                        if (FIntProperty* IntProp = CastField<FIntProperty>(ScoreProp))
                            CollectedScore = IntProp->GetPropertyValue_InContainer(HitActor);
                    }

                    OnMoneyCollected.Broadcast(CollectedMoney, CollectedScore);

                    TotalCollectedMoney += CollectedMoney;
                    TotalCollectedScore += CollectedScore;

                    HitActor->Destroy();

                    UE_LOG(LogTemp, Warning, TEXT("マネー回収！ 獲得:%d, スコア:%d | 合計お金:%d, 合計スコア:%d"), CollectedMoney, CollectedScore, TotalCollectedMoney, TotalCollectedScore);
                }
                else
                {
                    FVector TargetLoc = PlayerLoc + FVector(0.0f, 0.0f, 40.0f);
                    FVector NewLoc = FMath::VInterpTo(HitActor->GetActorLocation(), TargetLoc, DeltaTime, MagnetSpeed);
                    HitActor->SetActorLocation(NewLoc);
                }
            }
        }
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(100, 0.0f, FColor::Yellow, FString::Printf(TEXT("★ [Total Money] : %d 円"), TotalCollectedMoney));
        GEngine->AddOnScreenDebugMessage(101, 0.0f, FColor::Cyan, FString::Printf(TEXT("★ [Total Score] : %d pt"), TotalCollectedScore));
    }

    // ==========================================
    // 【常時表示UI用】 デバッグメッセージの固定表示
    // ==========================================
    if (GEngine)
    {
        // 第1引数（Key）を固定の数値にすることで、毎フレーム上書きされ常時表示HUDのようになります
        GEngine->AddOnScreenDebugMessage(100, 0.0f, FColor::Yellow, FString::Printf(TEXT("★ [Total Money] : %d 円"), TotalCollectedMoney));
        GEngine->AddOnScreenDebugMessage(101, 0.0f, FColor::Cyan, FString::Printf(TEXT("★ [Total Score] : %d pt"), TotalCollectedScore));

        // ★追加：現在持っているアイテムの表示（Keyを102にする）
        if (HeldItem)
        {
            // アイテムを持っている時は緑色でアイテム名を表示
            GEngine->AddOnScreenDebugMessage(102, 0.0f, FColor::Green, FString::Printf(TEXT("★ [Held Item] : %s"), *HeldItem->GetName()));
        }
        else
        {
            // 何も持っていない時は白色で「なし」と表示
            GEngine->AddOnScreenDebugMessage(102, 0.0f, FColor::White, TEXT("★ [Held Item] : なし"));
        }
    }
}

void UItemHoldComponent::UpdateGridHighlight()
{
    if (!HeldItem || bIsItemSnapping || bIsItemPlacing || !OwnerCharacter || !GridHighlightMesh)
    {
        if (GridHighlightMesh) GridHighlightMesh->SetHiddenInGame(true);
        bCanPlaceOnGrid = false;
        return;
    }

    FVector Start = OwnerCharacter->GetActorLocation();
    FVector Forward = OwnerCharacter->GetActorForwardVector();
    FVector TargetBase = Start + (Forward * InteractDistance);

    float SnappedX = FMath::GridSnap(TargetBase.X, GridSize);
    float SnappedY = FMath::GridSnap(TargetBase.Y, GridSize);

    FVector TraceStart = FVector(SnappedX, SnappedY, Start.Z + 200.0f);
    FVector TraceEnd = FVector(SnappedX, SnappedY, Start.Z - 500.0f);

    FHitResult HitResult;
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(OwnerCharacter);
    TraceParams.AddIgnoredActor(HeldItem);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, TraceParams))
    {
        if (HitResult.ImpactNormal.Z > 0.5f && (HitResult.ImpactPoint.Z <= Start.Z + MaxPlacementHeight))
        {
            FVector SnappedLocation = FVector(SnappedX, SnappedY, HitResult.ImpactPoint.Z + PlacementZOffset);

            FCollisionShape CheckSphere = FCollisionShape::MakeSphere(20.0f);
            bool bIsOccupied = false;
            TArray<FOverlapResult> CheckOverlaps;

            FCollisionQueryParams OverlapParams = TraceParams;
            if (HitResult.GetActor())
            {
                OverlapParams.AddIgnoredActor(HitResult.GetActor());
            }

            GetWorld()->OverlapMultiByChannel(CheckOverlaps, SnappedLocation, FQuat::Identity, ECC_Visibility, CheckSphere, OverlapParams);

            for (const FOverlapResult& OverlapCheck : CheckOverlaps)
            {
                if (OverlapCheck.GetActor() && OverlapCheck.GetActor()->ActorHasTag("Holdable"))
                {
                    bIsOccupied = true;
                    break;
                }
            }

            if (!bIsOccupied)
            {
                bCanPlaceOnGrid = true;
                CurrentGridTargetLocation = SnappedLocation;

                float SnappedYaw = FMath::RoundToFloat(OwnerCharacter->GetActorRotation().Yaw / 90.0f) * 90.0f;
                CurrentGridTargetRotation = FRotator(0.0f, SnappedYaw, 0.0f);

                GridHighlightMesh->SetWorldLocationAndRotation(CurrentGridTargetLocation, CurrentGridTargetRotation);
                GridHighlightMesh->SetHiddenInGame(false);
                return;
            }
        }
    }

    bCanPlaceOnGrid = false;
    GridHighlightMesh->SetHiddenInGame(true);
}

void UItemHoldComponent::PrimaryInteract()
{
    if (!OwnerCharacter) return;

    FVector Start = OwnerCharacter->GetActorLocation();
    FVector Forward = OwnerCharacter->GetActorForwardVector();
    FVector OverlapCenter = Start + (Forward * (InteractDistance * 0.6f));

    FCollisionShape Sphere = FCollisionShape::MakeSphere(70.0f);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    if (HeldItem)
    {
        Params.AddIgnoredActor(HeldItem);
    }

    TArray<FOverlapResult> Overlaps;
    GetWorld()->OverlapMultiByChannel(Overlaps, OverlapCenter, FQuat::Identity, ECC_Visibility, Sphere, Params);

    if (HeldItem)
    {
        AActor* FoundCounter = nullptr;
        AActor* FoundTrashCan = nullptr;
        AActor* FoundCookingStation = nullptr;
        AActor* FoundCustomer = nullptr;

        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* HitActor = Overlap.GetActor();
            if (HitActor)
            {
                if (HitActor->ActorHasTag("TrashCan")) { FoundTrashCan = HitActor; break; }
                else if (HitActor->ActorHasTag("CookingStation")) { FoundCookingStation = HitActor; break; }
                else if (HitActor->ActorHasTag("Customer")) { FoundCustomer = HitActor; break; }
                else if (HitActor->ActorHasTag("Counter")) { FoundCounter = HitActor; break; }
            }
        }

        if (FoundTrashCan)
        {
            HeldItem->Destroy();
            HeldItem = nullptr;
            bIsItemSnapping = false;
        }
        else if (FoundCookingStation)
        {
            ACookingStation* Station = Cast<ACookingStation>(FoundCookingStation);
            if (Station && Station->PlaceItem(HeldItem))
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(HeldItem->GetRootComponent()))
                {
                    PrimComp->SetSimulatePhysics(false);
                    PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                }
                HeldItem = nullptr;
                bIsItemSnapping = false;
            }
        }
        else if (FoundCustomer)
        {
            ABobNPCCharacter* Customer = Cast<ABobNPCCharacter>(FoundCustomer);
            if (Customer && Customer->CurrentState == ECustomerState::Waiting)
            {
                FName FoodTag = NAME_None;
                float PriceMult = 1.0f;
                int32 FoodScore = 0;

                if (FProperty* Prop = HeldItem->GetClass()->FindPropertyByName(FName("ProvidedFoodTag")))
                {
                    if (FNameProperty* NameProp = CastField<FNameProperty>(Prop))
                        FoodTag = NameProp->GetPropertyValue_InContainer(HeldItem);
                }
                if (FProperty* Prop = HeldItem->GetClass()->FindPropertyByName(FName("PriceMultiplier")))
                {
                    if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
                        PriceMult = FloatProp->GetPropertyValue_InContainer(HeldItem);
                }
                if (FProperty* Prop = HeldItem->GetClass()->FindPropertyByName(FName("ScorePoint")))
                {
                    if (FIntProperty* IntProp = CastField<FIntProperty>(Prop))
                        FoodScore = IntProp->GetPropertyValue_InContainer(HeldItem);
                }

                HeldItem->Destroy();
                HeldItem = nullptr;
                bIsItemSnapping = false;

                // ★ここで結果（true/false）を受け取る！
                bool bIsCorrectFood = Customer->ReceiveFoodAndLeave(FoodTag, PriceMult, FoodScore);

                // ★間違っていた場合のペナルティ処理
                if (!bIsCorrectFood)
                {
                    int32 PenaltyPoint = 10; // ← 減らすポイントはここで自由に調整してください！
                    TotalCollectedScore -= PenaltyPoint;

                    // スコアをマイナス（0未満）にしたくない場合は 0 で止める
                    if (TotalCollectedScore < 0)
                    {
                        TotalCollectedScore = 0;
                    }

                    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("【ペナルティ】間違った料理を出した！ 評価が %d 下がった！"), PenaltyPoint));
                }
            }
        }
        else if (FoundCounter)
        {
            FVector TargetLoc = FoundCounter->GetActorLocation() + FVector(0.0f, 0.0f, 90.0f);
            FCollisionShape CheckSphere = FCollisionShape::MakeSphere(20.0f);
            FCollisionQueryParams CheckParams;
            CheckParams.AddIgnoredActor(OwnerCharacter);
            CheckParams.AddIgnoredActor(HeldItem);

            bool bIsOccupied = false;
            TArray<FOverlapResult> CheckOverlaps;
            GetWorld()->OverlapMultiByChannel(CheckOverlaps, TargetLoc, FQuat::Identity, ECC_Visibility, CheckSphere, CheckParams);

            for (const FOverlapResult& Overlap : CheckOverlaps)
            {
                if (Overlap.GetActor() && Overlap.GetActor()->ActorHasTag("Holdable"))
                {
                    bIsOccupied = true;
                    break;
                }
            }

            if (bIsOccupied) return;

            HeldItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
            PlaceTargetLocation = TargetLoc;
            PlaceTargetRotation = FRotator::ZeroRotator;
            if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(HeldItem->GetRootComponent()))
            {
                PrimComp->SetSimulatePhysics(false);
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            }
            PlacingItem = HeldItem;
            bIsItemPlacing = true;
            HeldItem = nullptr;
            bIsItemSnapping = false;
        }
        else
        {
            if (bCanPlaceOnGrid)
            {
                HeldItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
                PlaceTargetLocation = CurrentGridTargetLocation;
                PlaceTargetRotation = CurrentGridTargetRotation;
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(HeldItem->GetRootComponent()))
                {
                    PrimComp->SetSimulatePhysics(false);
                    PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                }
                PlacingItem = HeldItem;
                bIsItemPlacing = true;
                HeldItem = nullptr;
                bIsItemSnapping = false;
            }
            else
            {
                HeldItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(HeldItem->GetRootComponent()))
                {
                    PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    PrimComp->SetSimulatePhysics(true);
                    FVector TossDir = (Forward + FVector(0, 0, 0.5f)).GetSafeNormal();
                    PrimComp->AddImpulse(TossDir * 300.0f * PrimComp->GetMass());
                }
                HeldItem = nullptr;
                bIsItemSnapping = false;
            }
        }
    }
    else
    {
        AActor* FoundSpawner = nullptr;
        AActor* FoundCookingStation = nullptr;
        AActor* FoundHoldable = nullptr;

        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* HitActor = Overlap.GetActor();
            if (HitActor)
            {
                if (HitActor->ActorHasTag("Spawner")) { FoundSpawner = HitActor; }
                else if (HitActor->ActorHasTag("CookingStation")) { FoundCookingStation = HitActor; }
                else if (HitActor->ActorHasTag("Holdable")) { FoundHoldable = HitActor; }
            }
        }

        if (FoundCookingStation)
        {
            ACookingStation* Station = Cast<ACookingStation>(FoundCookingStation);
            if (Station)
            {
                AActor* RetrievedItem = Station->RetrieveItem();
                if (RetrievedItem)
                {
                    HeldItem = RetrievedItem;
                    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(HeldItem->GetRootComponent()))
                    {
                        PrimComp->SetSimulatePhysics(false);
                        PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                    }
                    HeldItem->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, HandSocketName);
                    bIsItemSnapping = true;
                    return;
                }
            }
        }

        if (FoundSpawner)
        {
            AItemSpawner* Spawner = Cast<AItemSpawner>(FoundSpawner);
            if (Spawner)
            {
                AActor* SpawnedItem = Spawner->SpawnItem();
                if (SpawnedItem)
                {
                    HeldItem = SpawnedItem;
                    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(HeldItem->GetRootComponent()))
                    {
                        PrimComp->SetSimulatePhysics(false);
                        PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                    }
                    HeldItem->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, HandSocketName);
                    bIsItemSnapping = true;
                    return;
                }
            }
        }

        if (FoundHoldable)
        {
            AActor* ParentActor = FoundHoldable->GetAttachParentActor();
            if (ParentActor && ParentActor->ActorHasTag("CookingStation"))
            {
                ACookingStation* Station = Cast<ACookingStation>(ParentActor);
                if (Station)
                {
                    AActor* RetrievedItem = Station->RetrieveItem();
                    if (RetrievedItem)
                    {
                        HeldItem = RetrievedItem;
                        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(HeldItem->GetRootComponent()))
                        {
                            PrimComp->SetSimulatePhysics(false);
                            PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                        }
                        HeldItem->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, HandSocketName);
                        bIsItemSnapping = true;
                        return;
                    }
                }
            }

            if (FoundHoldable == PlacingItem)
            {
                bIsItemPlacing = false;
                PlacingItem = nullptr;
            }
            HeldItem = FoundHoldable;
            if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(HeldItem->GetRootComponent()))
            {
                PrimComp->SetSimulatePhysics(false);
                PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
            HeldItem->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, HandSocketName);
            bIsItemSnapping = true;
        }
    }
}