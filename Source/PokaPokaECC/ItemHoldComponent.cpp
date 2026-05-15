#include "ItemHoldComponent.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CookingStation.h"

UItemHoldComponent::UItemHoldComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
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

        // 目の前に何があるか探す
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* HitActor = Overlap.GetActor();
            if (HitActor)
            {
                if (HitActor->ActorHasTag("TrashCan")) { FoundTrashCan = HitActor; break; }
                else if (HitActor->ActorHasTag("CookingStation")) { FoundCookingStation = HitActor; break; } // 【追加】
                else if (HitActor->ActorHasTag("Counter")) { FoundCounter = HitActor; break; }
            }
        }

        if (FoundTrashCan)
        {
            HeldItem->Destroy();
            HeldItem = nullptr;
            bIsItemSnapping = false;
        }
        else if (FoundCookingStation) // 【追加】コンロ等にアイテムをセットする処理
        {
            ACookingStation* Station = Cast<ACookingStation>(FoundCookingStation);
            // StationのPlaceItem関数を呼び出し、成功したら手持ちを空にする
            if (Station && Station->PlaceItem(HeldItem))
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(HeldItem->GetRootComponent()))
                {
                    PrimComp->SetSimulatePhysics(false);
                    PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 調理中は触れなくする
                }
                HeldItem = nullptr;
                bIsItemSnapping = false;
            }
        }
        // カウンターに対する処理（先ほど作った重複チェック入り）
        else if (FoundCounter)
        {
            // 置く予定の座標を計算
            FVector TargetLoc = FoundCounter->GetActorLocation() + FVector(0.0f, 0.0f, 90.0f);

            // 置く場所にすでにアイテムがないかチェックする
            FCollisionShape CheckSphere = FCollisionShape::MakeSphere(20.0f); // チェック用の小さな球
            FCollisionQueryParams CheckParams;
            CheckParams.AddIgnoredActor(OwnerCharacter); // 自分は無視
            CheckParams.AddIgnoredActor(HeldItem);       // 今手に持っているアイテムも無視

            bool bIsOccupied = false; // すでに場所が埋まっているかのフラグ
            TArray<FOverlapResult> CheckOverlaps;
            GetWorld()->OverlapMultiByChannel(CheckOverlaps, TargetLoc, FQuat::Identity, ECC_Visibility, CheckSphere, CheckParams);

            // 置く予定の場所に何かあったら、それがアイテムか確認
            for (const FOverlapResult& Overlap : CheckOverlaps)
            {
                if (Overlap.GetActor() && Overlap.GetActor()->ActorHasTag("Holdable"))
                {
                    bIsOccupied = true; // アイテムがあった！
                    break;
                }
            }

            // もしすでにアイテムが置かれていたら、ここで処理を中断（置かない）
            if (bIsOccupied)
            {
                UE_LOG(LogTemp, Warning, TEXT("ここにはすでにアイテムが置かれています！"));
                return;
            }

            HeldItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
            PlaceTargetLocation = FoundCounter->GetActorLocation() + FVector(0.0f, 0.0f, 90.0f);
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
            // 何もない空間の場合、Tickで計算済みの座標（bCanPlaceOnGrid）を使って配置する
            if (bCanPlaceOnGrid)
            {
                HeldItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

                // Tickでハイライト用に計算した座標と回転をそのまま配置先に使う
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
                // 床が見つからなかった場合（空中など）は「投げる」
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
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* HitActor = Overlap.GetActor();
            if (HitActor)
            {
                if (HitActor->ActorHasTag("Spawner"))
                {
                    AItemSpawner* Spawner = Cast<AItemSpawner>(HitActor);
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
                            break;
                        }
                    }
                }
                else if (HitActor->ActorHasTag("CookingStation")) // 【追加】コンロ等から完成品を取り出す処理
                {
                    ACookingStation* Station = Cast<ACookingStation>(HitActor);
                    if (Station)
                    {
                        // 器具からアイテムを取り出す（生焼けの場合はnullptrが返ってくる）
                        AActor* RetrievedItem = Station->RetrieveItem();
                        if (RetrievedItem)
                        {
                            HeldItem = RetrievedItem; // 取り出したものを手に持つ
                            if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(HeldItem->GetRootComponent()))
                            {
                                PrimComp->SetSimulatePhysics(false);
                                PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                            }
                            HeldItem->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, HandSocketName);
                            bIsItemSnapping = true;
                            break;
                        }
                    }
                }
                // 2. 目の前のオブジェクトが「置かれているアイテム」だった場合
                else if (HitActor->ActorHasTag("Holdable"))
                {
                    if (HitActor == PlacingItem)
                    {
                        bIsItemPlacing = false;
                        PlacingItem = nullptr;
                    }
                    HeldItem = HitActor;
                    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(HeldItem->GetRootComponent()))
                    {
                        PrimComp->SetSimulatePhysics(false);
                        PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                    }
                    HeldItem->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, HandSocketName);
                    bIsItemSnapping = true;
                    break; // 処理を完了してループを抜ける
                }
            }
        }
    }
}