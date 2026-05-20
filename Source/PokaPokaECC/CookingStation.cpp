#include "CookingStation.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "PokaPokaECCGameInstance.h"
#include "Components/AudioComponent.h"

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

	// --- 【追加】オーディオコンポーネントの初期化 ---
	AudioComponent1 = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent1"));
	AudioComponent1->SetupAttachment(ItemSocket1); // ソケット1の位置から鳴るようにする
	AudioComponent1->bAutoActivate = false;        // 最初は鳴らさない

	AudioComponent2 = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent2"));
	AudioComponent2->SetupAttachment(ItemSocket2); // ソケット2の位置から鳴るようにする
	AudioComponent2->bAutoActivate = false;        // 最初は鳴らさない

	Tags.Add(FName("CookingStation"));

	CookTime = 5.0f;
	BurnTime = 5.0f;
	MaxCapacity = 2;

	CurrentState1 = ECookingState::Empty;
	CurrentState2 = ECookingState::Empty;
	CurrentItem1 = nullptr;
	CurrentItem2 = nullptr;

	SpawnedEffect1 = nullptr;
	SpawnedEffect2 = nullptr;

	
	EffectScale = FVector(1.0f, 1.0f, 1.0f);// デフォルトの大きさ


}

void ACookingStation::BeginPlay()
{
	Super::BeginPlay();

	// --- GameInstanceから永続バフを取得して自身に適用 ---
	if (UPokaPokaECCGameInstance* GI = Cast<UPokaPokaECCGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		// 自身に設定されているタグを判定して、専用の倍率を適用する
		if (ActorHasTag(FName("Fryer")))
		{
			CookingSpeedMultiplier = GI->FryerSpeedMultiplier;
		}
		else if (ActorHasTag(FName("Burner")))
		{
			CookingSpeedMultiplier = GI->BurnerSpeedMultiplier;
		}
	}

	// 実際の調理タイマーを設定する際は、CookTime に CookingSpeedMultiplier を掛けた値をセットするようにします。
	// 例: GetWorldTimerManager().SetTimer(Handle, ..., CookTime * CookingSpeedMultiplier, false);
}

// 食材を置く処理
bool ACookingStation::PlaceItem(AActor* ItemToPlace)
{
	if (ItemToPlace == nullptr) return false;

	// （AcceptedItemTag が空欄 "None" の場合は、今まで通り何でも置ける仕様にしておきます）
	if (!AcceptedItemTag.IsNone() && !ItemToPlace->ActorHasTag(AcceptedItemTag))
	{
		// デバッグ強化：何が原因で弾かれたのかを画面に詳細に出力する
		FString ExpectedTag = AcceptedItemTag.ToString();
		FString ItemName = ItemToPlace->GetName();

		// アイテムが実際に持っているすべてのタグを取得して文字にする
		FString AllTags = TEXT("");
		for (const FName& Tag : ItemToPlace->Tags)
		{
			AllTags += Tag.ToString() + TEXT(", ");
		}
		if (AllTags.IsEmpty()) AllTags = TEXT("タグなし(None)");

		// 画面に表示するメッセージを作る
		FString DebugMsg = FString::Printf(TEXT("【エラー】要求:[%s] | アイテム:[%s] | 所持タグ:[%s] "), *ExpectedTag, *ItemName, *AllTags);

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, DebugMsg);

		return false;
	}

	// まず左（ソケット1）が空いているかチェック！
	if (CurrentState1 == ECookingState::Empty)
	{
		CurrentItem1 = ItemToPlace;
		CurrentItem1->SetActorLocationAndRotation(ItemSocket1->GetComponentLocation(), ItemSocket1->GetComponentRotation());
		CurrentItem1->AttachToComponent(ItemSocket1, FAttachmentTransformRules::KeepWorldTransform);
		CurrentState1 = ECookingState::Cooking;
		GetWorldTimerManager().SetTimer(CookingTimerHandle1, this, &ACookingStation::OnCookingFinished1, CookTime, false);
		
		// --- 【追加】ソケット1での調理開始音を再生 ---
		if (CookingSound)
		{
			AudioComponent1->SetSound(CookingSound);
			AudioComponent1->Play();
		}

		if (CookingEffect)//ソケット1の位置に火のエフェクトを
		{
			SpawnedEffect1 = UNiagaraFunctionLibrary::SpawnSystemAttached(
				CookingEffect, ItemSocket1, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true
			);
			if (SpawnedEffect1)
			{
				SpawnedEffect1->SetRelativeScale3D(EffectScale);
			}
		}
		return true;
	}
	// 左が埋まっていたら、右（ソケット2）が空いているかチェック！
	else if (MaxCapacity >= 2 && CurrentState2 == ECookingState::Empty) 
	{
		CurrentItem2 = ItemToPlace;
		CurrentItem2->SetActorLocationAndRotation(ItemSocket2->GetComponentLocation(), ItemSocket2->GetComponentRotation());
		CurrentItem2->AttachToComponent(ItemSocket2, FAttachmentTransformRules::KeepWorldTransform);
		CurrentState2 = ECookingState::Cooking;
		GetWorldTimerManager().SetTimer(CookingTimerHandle2, this, &ACookingStation::OnCookingFinished2, CookTime, false);
		
		// --- 【追加】ソケット2での調理開始音を再生 ---
		if (CookingSound)
		{
			AudioComponent2->SetSound(CookingSound);
			AudioComponent2->Play();
		}
		// --- 【追加】ソケット2の位置に火のエフェクトを出す ---
		if (CookingEffect)
		{
			SpawnedEffect2 = UNiagaraFunctionLibrary::SpawnSystemAttached(
				CookingEffect, ItemSocket2, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true
			);
			if (SpawnedEffect1)
			{
				SpawnedEffect1->SetRelativeScale3D(EffectScale);
			}
		}
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
	
	// --- 【追加】完成のタイミングで KamseiSound を一度だけ鳴らす ---
	if (KamseiSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, KamseiSound, ItemSocket1->GetComponentLocation());
	}
	// --- 【追加】完成したらソケット1の音を止める ---
	//AudioComponent1->Stop();
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
	// --- 【追加】念のためここでも音を確実に止める ---
	//AudioComponent1->Stop();
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
	// --- 【追加】完成のタイミングで KamseiSound を一度だけ鳴らす ---
	if (KamseiSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, KamseiSound, ItemSocket2->GetComponentLocation());
	}
	// --- 【追加】完成のタイミングで KamseiSound を一度だけ鳴らす ---
	if (KamseiSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, KamseiSound, ItemSocket2->GetComponentLocation());
	}
	// --- 【追加】完成したらソケット2の音を止める ---
	//AudioComponent2->Stop();
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
	// --- 【追加】念のためここでも音を確実に止める ---
//	AudioComponent2->Stop();
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
		AudioComponent1->Stop(); // 【追加】取り出した時も止める
		
		if (SpawnedEffect1)// --- 【追加】取り出したら火のエフェクトを消す ---
		{
			SpawnedEffect1->DestroyComponent();
			SpawnedEffect1 = nullptr;
		}
		return ItemToReturn;
	}
	else if (CurrentState2 == ECookingState::Done)
	{
		AActor* ItemToReturn = CurrentItem2;
		GetWorldTimerManager().ClearTimer(CookingTimerHandle2);
		CurrentItem2 = nullptr;
		CurrentState2 = ECookingState::Empty;
		AudioComponent2->Stop(); // 【追加】取り出した時も止める
		// --- 【追加】取り出したら火のエフェクトを消す ---
		if (SpawnedEffect2)
		{
			SpawnedEffect2->DestroyComponent();
			SpawnedEffect2 = nullptr;
		}
		return ItemToReturn;
	}

	// 2. 完成品がなければ「焦げたもの（Burnt）」を探す
	if (CurrentState1 == ECookingState::Burnt)
	{
		AActor* ItemToReturn = CurrentItem1;
		CurrentItem1 = nullptr;
		CurrentState1 = ECookingState::Empty;
		AudioComponent1->Stop(); // 【追加】取り出した時も止める
		// --- 【追加】取り出したら火のエフェクトを消す ---
		if (SpawnedEffect1)
		{
			SpawnedEffect1->DestroyComponent();
			SpawnedEffect1 = nullptr;
		}
		return ItemToReturn;
	}
	else if (CurrentState2 == ECookingState::Burnt)
	{
		AActor* ItemToReturn = CurrentItem2;
		CurrentItem2 = nullptr;
		CurrentState2 = ECookingState::Empty;
		AudioComponent2->Stop(); // 【追加】取り出した時も止める
		
		// --- 【追加】取り出したら火のエフェクトを消す ---
		if (SpawnedEffect2)
		{
			SpawnedEffect2->DestroyComponent();
			SpawnedEffect2 = nullptr;
		}
	
		return ItemToReturn;
	}

	// どちらも調理中や空っぽなら何も取り出せない
	return nullptr;
}