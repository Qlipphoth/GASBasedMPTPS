// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "GameplayTagContainer.h"

void UCharacterOverlay::NativeConstruct()
{
    Super::NativeConstruct();

    if (HighPingImage)
    {
        HighPingImage->SetOpacity(0.f);
    }

    FlameProjectileBuffTag = FGameplayTag::RequestGameplayTag(FName("Ability.Projectile.Buff.Flame"));
    FlashProjectileBuffTag = FGameplayTag::RequestGameplayTag(FName("Ability.Projectile.Buff.Flash"));
    PoisonProjectileBuffTag = FGameplayTag::RequestGameplayTag(FName("Ability.Projectile.Buff.Poison"));
    RandomProjectileBuffTag = FGameplayTag::RequestGameplayTag(FName("Ability.Projectile.Buff.Random"));

}

void UCharacterOverlay::SetHUDHealth(float Health, float MaxHealth)
{
    if (HealthBar)
    {
        HealthBar->SetPercent(Health / MaxHealth);
    }
    if (HealthText)
    {
        HealthText->SetText(FText::FromString(FString::Printf(
            TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth)
        )));
    }
}

void UCharacterOverlay::SetHUDShield(float Shield, float MaxShield)
{
    if (ShieldBar)
    {
        ShieldBar->SetPercent(Shield / MaxShield);
    }
    if (ShieldText)
    {
        ShieldText->SetText(FText::FromString(FString::Printf(
            TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield)
        )));
    }
}

void UCharacterOverlay::SetHUDScore(float Score)
{
    if (ScoreAmount)
    {
        ScoreAmount->SetText(FText::FromString(FString::Printf(
            TEXT("%d"), FMath::CeilToInt(Score)
        )));
    }
}

void UCharacterOverlay::SetHUDDefeats(int32 Defeats)
{
    if (DefeatsAmount)
    {
        DefeatsAmount->SetText(FText::FromString(FString::Printf(
            TEXT("%d"), Defeats
        )));
    }
}

void UCharacterOverlay::SetHUDWeaponAmmo(int32 Ammo)
{
    if (WeaponAmmoAmount)
    {
        WeaponAmmoAmount->SetText(FText::FromString(FString::Printf(
            TEXT("%d"), Ammo
        )));
    }
}

void UCharacterOverlay::SetHUDCarriedAmmo(int32 Ammo)
{
    if (CarriedAmmoAmount)
    {
        CarriedAmmoAmount->SetText(FText::FromString(FString::Printf(
            TEXT("%d"), Ammo
        )));
    }
}

void UCharacterOverlay::SetHUDMatchCountdown(float CountdownTime)
{
    if (MatchCountdownText)
    {
        int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

        MatchCountdownText->SetText(FText::FromString(FString::Printf(
            TEXT("%02d:%02d"), Minutes, Seconds
        )));
    }
}

void UCharacterOverlay::SetHUDGrenades(int32 Grenades)
{
    if (GrenadesText)
    {
        GrenadesText->SetText(FText::FromString(FString::Printf(
            TEXT("%d"), Grenades
        )));
    }
}

void UCharacterOverlay::HighPingWarning()
{
    if (HighPingImage)
    {
        HighPingImage->SetOpacity(1.f);
        if (HighPingAnimation)
        {
            PlayAnimation(HighPingAnimation, 0.f, 5);
        }
    }
}

void UCharacterOverlay::StopHighPingWarning()
{
    if (HighPingImage)
    {
        HighPingImage->SetOpacity(0.f);
        if (IsAnimationPlaying(HighPingAnimation))
        {
            StopAnimation(HighPingAnimation);
        }
    }
}

void UCharacterOverlay::SetHUDRedTeamScore(int32 RedScore)
{
    if (RedTeamScore)
    {
        RedTeamScore->SetText(FText::FromString(FString::Printf(
            TEXT("%d"), RedScore
        )));
    }
}

void UCharacterOverlay::SetHUDBlueTeamScore(int32 BlueScore)
{
    if (BlueTeamScore)
    {
        BlueTeamScore->SetText(FText::FromString(FString::Printf(
            TEXT("%d"), BlueScore
        )));
    }
}

void UCharacterOverlay::HideTeamScores()
{
    if (RedTeamScore)
    {
        RedTeamScore->SetVisibility(ESlateVisibility::Hidden);
    }
    if (BlueTeamScore)
    {
        BlueTeamScore->SetVisibility(ESlateVisibility::Hidden);
    }
    if (ScoreSpacerText)
    {
        ScoreSpacerText->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UCharacterOverlay::InitTeamScores()
{
    FString Zero("0");
	FString Spacer("|");

    if (RedTeamScore)
    {
        RedTeamScore->SetVisibility(ESlateVisibility::Visible);
        RedTeamScore->SetText(FText::FromString(Zero));
    }
    if (BlueTeamScore)
    {
        BlueTeamScore->SetVisibility(ESlateVisibility::Visible);
        BlueTeamScore->SetText(FText::FromString(Zero));
    }
    if (ScoreSpacerText)
    {
        ScoreSpacerText->SetVisibility(ESlateVisibility::Visible);
        ScoreSpacerText->SetText(FText::FromString(Spacer));
    }
}