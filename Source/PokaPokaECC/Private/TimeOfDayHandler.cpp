#include "TimeOfDayHandler.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/PostProcessVolume.h"

ATimeOfDayHandler::ATimeOfDayHandler()
{
    PrimaryActorTick.bCanEverTick = true;
    DayLengthInSeconds = 60.0f; // 1周60秒の設定
}

void ATimeOfDayHandler::BeginPlay()
{
    Super::BeginPlay();

    if (SunLight)
    {
        FRotator StartRotation = SunLight->GetActorRotation();
        StartRotation.Pitch = -60.0f; // 開始時はお昼の角度（上空）syu
        StartRotation.Yaw = 0.0f;
        StartRotation.Roll = 0.0f;
        SunLight->SetActorRotation(StartRotation);
    }

    if (SkyLight && SkyLight->GetLightComponent())
    {
        SkyLight->GetLightComponent()->SetIntensity(1500.0f);
    }

    if (KitchenVolume)
    {
        // 1️⃣ 【重複の解除】エディタ側の「露出補正（1.5）」を、正しい変数名で上書きして【ゼロ】にする
        KitchenVolume->Settings.bOverride_AutoExposureBias = true;
        KitchenVolume->Settings.AutoExposureBias = 0.0f; // これで2重上書きが確実に消滅します

        // 2️⃣ 自動露出の暴走を完全にロックする（Min/Max を両方 1.5 に固定）
        KitchenVolume->Settings.bOverride_AutoExposureMinBrightness = true;
        KitchenVolume->Settings.bOverride_AutoExposureMaxBrightness = true;
        KitchenVolume->Settings.AutoExposureMinBrightness = 1.5f;
        KitchenVolume->Settings.AutoExposureMaxBrightness = 1.5f;
    }
}

void ATimeOfDayHandler::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (SunLight && SkyLight && SkyLight->GetLightComponent())
    {
        FRotator CurrentRotation = SunLight->GetActorRotation();

        // ⭕ 太陽が「-5度」（地平線のすぐ上、一番夕方らしく暗くなる位置）になるまで回す
        if (CurrentRotation.Pitch < -5.0f)
        {
            float RotationSpeed = 360.0f / DayLengthInSeconds;
            CurrentRotation.Pitch += RotationSpeed * DeltaTime;

            CurrentRotation.Pitch = FMath::Fmod(CurrentRotation.Pitch + 180.0f, 360.0f) - 180.0f;
            SunLight->SetActorRotation(CurrentRotation);

            // 太陽の角度（-60度 〜 -5度）に合わせてAlphaを計算
            float Alpha = FMath::Clamp((CurrentRotation.Pitch - (-60.0f)) / (-5.0f - (-60.0f)), 0.0f, 1.0f);

            // ⭕ 【変更】開始時は 1500.0f（大快晴）、最終値は 「100.0f」（しっかり暗い夕方）まで落とす！
            float NewIntensity = FMath::Lerp(1500.0f, 150.0f, Alpha);
            SkyLight->GetLightComponent()->SetIntensity(NewIntensity);
        }
        else
        {
            // ⭕ 太陽が -5度 に達したら、角度も明るさ（100.0f）もそこで完全にフリーズさせて真っ暗化を防ぐ
            CurrentRotation.Pitch = -5.0f;
            SunLight->SetActorRotation(CurrentRotation);

            SkyLight->GetLightComponent()->SetIntensity(150.0f);

            PrimaryActorTick.SetTickFunctionEnable(false);
        }
        
    }
}