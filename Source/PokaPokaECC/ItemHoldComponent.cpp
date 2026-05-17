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

    // マグネット機能のデフォルト設定（BP側で上書き可能）
    MagnetRadius = 300.0f;
    MagnetSpeed = 10.0f;
    CollectionDistance = 70.0f;
}

void UItemHoldComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ACharacter>(GetOwner());
    // ハイライト用のメッシュコンポーネントを動的に作成してキャラクターにアタッチ
    if (OwnerCharacter)
    {
        GridHighlightMesh = NewObject<UStaticMeshComponent>(OwnerCharacter, TEXT("GridHighlightMesh"));
        if (GridHighlightMesh)
        {
            GridHighlightMesh->RegisterComponent();
            GridHighlightMesh->AttachToComponent(OwnerCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
            GridHighlightMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 衝突判定はオフ
            GridHighlightMesh->SetHiddenInGame(true); // 最初は非表示

            // BPで設定されたメッシュとマテリアルを適用
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

    // 1. 手元への引き寄せ
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

    // 2. 机への配置移動
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
    // 毎フレーム、グリッドのハイライト位置を更新
    UpdateGridHighlight();

    // ==========================================
    // 3. お金の自動引き寄せ（マグネット）処理
    // ==========================================
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

                    if (FProperty* MoneyProp = HitActor->GetClass()->FindPropertyByName(FName("MoneyAmount")))
                    {
                        if (FIntProperty* IntProp = CastField<FIntProperty>(MoneyProp))
                            CollectedMoney = IntProp->GetPropertyValue_InContainer(HitActor);
                    }
                    if (FProperty* ScoreProp = HitActor->GetClass()->FindPropertyByName(FName("ScorePoint")))
                    {
                        if (FIntProperty* IntProp = CastField<FIntProperty>(ScoreProp))
                            CollectedScore = IntProp->GetPropertyValue_InContainer(HitActor);
                    }

                    // 取得した動的な値をイベントに渡してブループリントへ通知
                    OnMoneyCollected.Broadcast(CollectedMoney, CollectedScore);

                    // ★追加：合計変数に加算する！
                    TotalCollectedMoney += CollectedMoney;
                    TotalCollectedScore += CollectedScore;

                    HitActor->Destroy();

                    // 回収した瞬間のログ出力（コンソール用）
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

    // ==========================================
    // 【常時表示UI用】 デバッグメッセージの固定表示
    // ==========================================
    if (GEngine)
    {
        // 第1引数（Key）を「100」「101」など固定の数値にすると、毎フレーム上書きされ常時表示HUDのようになります
        // 第2引数（Duration）を「0.0f」にすることで、1フレームだけ表示＝毎フレーム更新されます
        GEngine->AddOnScreenDebugMessage(100, 0.0f, FColor::Yellow, FString::Printf(TEXT("★ [Total Money] : %d 円"), TotalCollectedMoney));
        GEngine->AddOnScreenDebugMessage(101, 0.0f, FColor::Cyan, FString::Printf(TEXT("★ [Total Score] : %d pt"), TotalCollectedScore));
    }
}

// ハイライト表示用の関数
void UItemHoldComponent::UpdateGridHighlight()
{
    // アイテムを持っていない、または配置移動中の場合は非表示にして処理終了
    if (!HeldItem || bIsItemSnapping || bIsItemPlacing || !OwnerCharacter || !GridHighlightMesh)
    {
        if (GridHighlightMesh) GridHighlightMesh->SetHiddenInGame(true);
        bCanPlaceOnGrid = false;
        return;
    }

    // 1. まずキャラクターの目の前の座標を基準点とする
    FVector Start = OwnerCharacter->GetActorLocation();
    FVector Forward = OwnerCharacter->GetActorForwardVector();
    FVector TargetBase = Start + (Forward * InteractDistance);

    // 2. 先にXとYをグリッドサイズにスナップ（吸着）させる
    float SnappedX = FMath::GridSnap(TargetBase.X, GridSize);
    float SnappedY = FMath::GridSnap(TargetBase.Y, GridSize);

    // 3. スナップしたグリッドの「上空から真下」に向かってライントレースを落とす
    // Start.Z + 200.0f : キャラクターより少し高い位置から落とすことで、高い机も検知可能にする
    FVector TraceStart = FVector(SnappedX, SnappedY, Start.Z + 200.0f);
    FVector TraceEnd = FVector(SnappedX, SnappedY, Start.Z - 500.0f);

    FHitResult HitResult;
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(OwnerCharacter);
    TraceParams.AddIgnoredActor(HeldItem);

    // 真下に向かって判定を飛ばし、最初に見つかった「一番高いオブジェクト」を取得
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, TraceParams))
    {
        // ★変更: 面が上向きかどうかに加え、「ヒットした高さが、キャラクターの中心(Start.Z)＋上限値以下か」をチェック
        if (HitResult.ImpactNormal.Z > 0.5f && (HitResult.ImpactPoint.Z <= Start.Z + MaxPlacementHeight))
        {
            // ヒットしたZ座標（高さ）を使って最終的な配置位置を決定
            FVector SnappedLocation = FVector(SnappedX, SnappedY, HitResult.ImpactPoint.Z + PlacementZOffset);

            // 配置予定の空間にすでに他のトマトなどがないかチェック
            FCollisionShape CheckSphere = FCollisionShape::MakeSphere(20.0f);
            bool bIsOccupied = false;
            TArray<FOverlapResult> CheckOverlaps;

            // 土台（机や床）自体はアイテムと見なさないようチェックから除外する
            FCollisionQueryParams OverlapParams = TraceParams;
            if (HitResult.GetActor())
            {
                OverlapParams.AddIgnoredActor(HitResult.GetActor());
            }

            GetWorld()->OverlapMultiByChannel(CheckOverlaps, SnappedLocation, FQuat::Identity, ECC_Visibility, CheckSphere, OverlapParams);

            for (const FOverlapResult& OverlapCheck : CheckOverlaps)
            {
                // 除外した土台以外の「Holdable」アイテムがそこに存在していれば置けない
                if (OverlapCheck.GetActor() && OverlapCheck.GetActor()->ActorHasTag("Holdable"))
                {
                    bIsOccupied = true;
                    break;
                }
            }

            // 空間が空いていればハイライトを表示して配置可能にする
            if (!bIsOccupied)
            {
                bCanPlaceOnGrid = true;
                CurrentGridTargetLocation = SnappedLocation;

                // 向きも東西南北（90度）にスナップさせる
                float SnappedYaw = FMath::RoundToFloat(OwnerCharacter->GetActorRotation().Yaw / 90.0f) * 90.0f;
                CurrentGridTargetRotation = FRotator(0.0f, SnappedYaw, 0.0f);

                GridHighlightMesh->SetWorldLocationAndRotation(CurrentGridTargetLocation, CurrentGridTargetRotation);
                GridHighlightMesh->SetHiddenInGame(false);
                return;
            }
        }
    }

    // ヒットしなかった、すでにアイテムがある、または【高すぎる場所（冷蔵庫など）】の場合は非表示
    bCanPlaceOnGrid = false;
    GridHighlightMesh->SetHiddenInGame(true);
}

void UItemHoldComponent::PrimaryInteract()
{
    if (!OwnerCharacter) return;

    // 【デバッグ①】そもそもこの関数が呼ばれているか？（インタラクトキーが効いているか）
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("[Debug] PrimaryInteract Called!"));

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

    // ===============================================
    // アイテムを持っている時の処理（置く・渡す）
    // ===============================================
    if (HeldItem)
    {
        // 【デバッグ②】アイテムを持っていると認識されているか？
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, FString::Printf(TEXT("[Debug] Holding Item: %s"), *HeldItem->GetName()));

        AActor* FoundCounter = nullptr;
        AActor* FoundTrashCan = nullptr;
        AActor* FoundCookingStation = nullptr;
        AActor* FoundCustomer = nullptr;

        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* HitActor = Overlap.GetActor();
            if (HitActor)
            {
                // ※もし目の前のアクターが全く検知されていない場合は、ここのコメントアウトを外すと当たっている全アクター名が表示されます
                if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::White, FString::Printf(TEXT("[Hit] %s"), *HitActor->GetName()));

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
            // 【デバッグ③】「Customer」タグを持つアクターを検知できたか？
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("[Debug] Found Customer Actor!"));

            ABobNPCCharacter* Customer = Cast<ABobNPCCharacter>(FoundCustomer);
            if (Customer)
            {
                // 【デバッグ④】お客さんの現在の状態は何か？
                FString StateStr = (Customer->CurrentState == ECustomerState::Waiting) ? TEXT("Waiting") : TEXT("NOT Waiting");
                if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Magenta, FString::Printf(TEXT("[Debug] Customer State is: %s"), *StateStr));

                if (Customer->CurrentState == ECustomerState::Waiting)
                {
                    int32 FoodMoney = -1;
                    int32 FoodScore = -1;

                    if (FProperty* Prop = HeldItem->GetClass()->FindPropertyByName(FName("MoneyAmount")))
                    {
                        if (FIntProperty* IntProp = CastField<FIntProperty>(Prop)) FoodMoney = IntProp->GetPropertyValue_InContainer(HeldItem);
                    }
                    if (FProperty* Prop = HeldItem->GetClass()->FindPropertyByName(FName("ScorePoint")))
                    {
                        if (FIntProperty* IntProp = CastField<FIntProperty>(Prop)) FoodScore = IntProp->GetPropertyValue_InContainer(HeldItem);
                    }
                    else if (FProperty* SoreProp = HeldItem->GetClass()->FindPropertyByName(FName("SorePoint")))
                    {
                        if (FIntProperty* IntProp = CastField<FIntProperty>(SoreProp)) FoodScore = IntProp->GetPropertyValue_InContainer(HeldItem);
                    }

                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("【検証】渡した食材(%s)の内部データ -> 金額:%d, スコア:%d"), *HeldItem->GetName(), FoodMoney, FoodScore));
                    }

                    HeldItem->Destroy();
                    HeldItem = nullptr;
                    bIsItemSnapping = false;

                    Customer->ReceiveFoodAndLeaveWithData(FoodMoney, FoodScore);
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
    // ===============================================
    // アイテムを持っていない時の処理（拾う・取り出す）
    // ===============================================
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