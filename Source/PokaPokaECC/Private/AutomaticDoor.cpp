#include "AutomaticDoor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "BobNPCCharacter.h" // Bobが近づいた時だけ開くようにするためのインクルード

AAutomaticDoor::AAutomaticDoor()
{
    PrimaryActorTick.bCanEverTick = true;

    DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
    RootComponent = DefaultRoot;

    // --- 左ドアのセットアップ ---
    LeftHinge = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHinge"));
    LeftHinge->SetupAttachment(RootComponent); // ルートにアタッチ

    LeftDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftDoorMesh"));
    LeftDoorMesh->SetupAttachment(LeftHinge); // ヒンジにアタッチ
    LeftDoorMesh->SetCanEverAffectNavigation(false); // AIの経路を塞がないようにする設定

    // --- 右ドアのセットアップ ---
    RightHinge = CreateDefaultSubobject<USceneComponent>(TEXT("RightHinge"));
    RightHinge->SetupAttachment(RootComponent);

    RightDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightDoorMesh"));
    RightDoorMesh->SetupAttachment(RightHinge); // ヒンジにアタッチ
    RightDoorMesh->SetCanEverAffectNavigation(false); // AIの経路を塞がないようにする設定

    // --- トリガーゾーンのセットアップ ---
    TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
    TriggerZone->SetupAttachment(RootComponent);
    TriggerZone->SetBoxExtent(FVector(150.f, 200.f, 100.f));
    TriggerZone->SetCollisionProfileName(TEXT("Trigger"));
    TriggerZone->SetCanEverAffectNavigation(false); // トリガーボックスもAI経路に影響させない

    bIsOpen = false;
}

void AAutomaticDoor::BeginPlay()
{
    Super::BeginPlay();

    // それぞれの「ヒンジ」の初期位置と目標位置を計算
    LeftClosedRot = LeftHinge->GetRelativeRotation();
    LeftOpenRot = LeftClosedRot + FRotator(0.f, OpenAngle, 0.f);

    RightClosedRot = RightHinge->GetRelativeRotation();
    RightOpenRot = RightClosedRot + FRotator(0.f, -OpenAngle, 0.f); // 右は反対向きに開く

    TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &AAutomaticDoor::OnOverlapBegin);
    TriggerZone->OnComponentEndOverlap.AddDynamic(this, &AAutomaticDoor::OnOverlapEnd);
}

void AAutomaticDoor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 目標の回転を決定
    FRotator TargetLeft = bIsOpen ? LeftOpenRot : LeftClosedRot;
    FRotator TargetRight = bIsOpen ? RightOpenRot : RightClosedRot;

    // メッシュではなく「ヒンジ」を滑らかに回転させる
    LeftHinge->SetRelativeRotation(FMath::RInterpTo(LeftHinge->GetRelativeRotation(), TargetLeft, DeltaTime, DoorSpeed));
    RightHinge->SetRelativeRotation(FMath::RInterpTo(RightHinge->GetRelativeRotation(), TargetRight, DeltaTime, DoorSpeed));
}

void AAutomaticDoor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Bob（またはBobを継承したNPC）が近づいた時
    if (OtherActor && OtherActor->IsA(ABobNPCCharacter::StaticClass()))
    {
        bIsOpen = true;
    }
}

void AAutomaticDoor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Bob（またはBobを継承したNPC）が離れた時
    if (OtherActor && OtherActor->IsA(ABobNPCCharacter::StaticClass()))
    {
        bIsOpen = false;
    }
}