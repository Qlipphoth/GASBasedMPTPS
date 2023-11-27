// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RedTeamScore;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BlueTeamScore;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreSpacerText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GrenadesText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;

	UPROPERTY(meta = (BindWidget))
	class UImage* HighPingImage;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HighPingAnimation;

public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);

	void HighPingWarning();
	void StopHighPingWarning();

	void SetHUDRedTeamScore(int32 RedScore);
	void SetHUDBlueTeamScore(int32 BlueScore);
	void HideTeamScores();
	void InitTeamScores();

	// DebuffBar is set Completely in BP, to demonstrate how to do it in BP
	// BuffBar is set partially in BP, to demonstrate different ways of doing it
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetBuffBar(FGameplayTag BuffTag, int32 StackNum);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnSkillSet(class UBlasterSkill* Skill, int32 Index);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnSkillUnset(int32 Index);

private:
	virtual void NativeConstruct() override;

	FGameplayTag FlameProjectileBuffTag;
	FGameplayTag FlashProjectileBuffTag;
	FGameplayTag PoisonProjectileBuffTag;
	FGameplayTag RandomProjectileBuffTag;
	
	FGameplayTag RageBuffTag;
	FGameplayTag AttackBuffTag;
	FGameplayTag ShieldBuffTag;
	FGameplayTag SpeedBuffTag;
	FGameplayTag HealBuffTag;
};
