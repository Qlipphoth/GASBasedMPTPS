// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/HUD/FloatStatusBarWidget.h"
#include "Blaster/BlasterGAS/BlasterASC.h"
#include "Blaster/BlasterGAS/BlasterAttributeSetBase.h"
#include "Blaster/BlasterTypes/InputID.h"
#include "Net/UnrealNetwork.h"

ABlasterPlayerState::ABlasterPlayerState()
{
    // Create ability system component, and set it to be explicitly replicated
    AbilitySystemComponent = CreateDefaultSubobject<UBlasterASC>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);

    // Mixed mode means we only are replicated the GEs to ourself, not the GEs to simulated proxies. 
	// If another GDPlayerState (Hero) receives a GE, we won't be told about it by the Server. 
	// Attributes, GameplayTags, and GameplayCues will still replicate to us.
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    // Create the attribute set, this replicates by default
	// Adding it as a subobject of the owning actor of an AbilitySystemComponent
	// automatically registers the AttributeSet with the AbilitySystemComponent
    AttributeSetBase = CreateDefaultSubobject<UBlasterAttributeSetBase>(TEXT("AttributeSetBase"));

    // Set PlayerState's NetUpdateFrequency to the same as the Character.
	// Default is very low for PlayerStates and introduces perceived lag in the ability system.
	// 100 is probably way too high for a shipping game, you can adjust to fit your needs.
    NetUpdateFrequency = 100.0f;

}

void ABlasterPlayerState::BeginPlay()
{
    Super::BeginPlay();

    if (AbilitySystemComponent)
    {
        // Attribute change callbacks
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            AttributeSetBase->GetHealthAttribute()).AddUObject(this, &ABlasterPlayerState::HealthChanged);
		MaxHealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            AttributeSetBase->GetMaxHealthAttribute()).AddUObject(this, &ABlasterPlayerState::MaxHealthChanged);
		HealthRegenRateChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            AttributeSetBase->GetHealthRegenRateAttribute()).AddUObject(this, &ABlasterPlayerState::HealthRegenRateChanged);
		ManaChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            AttributeSetBase->GetManaAttribute()).AddUObject(this, &ABlasterPlayerState::ManaChanged);
		MaxManaChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            AttributeSetBase->GetMaxManaAttribute()).AddUObject(this, &ABlasterPlayerState::MaxManaChanged);
		ManaRegenRateChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            AttributeSetBase->GetManaRegenRateAttribute()).AddUObject(this, &ABlasterPlayerState::ManaRegenRateChanged);
		StaminaChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            AttributeSetBase->GetStaminaAttribute()).AddUObject(this, &ABlasterPlayerState::StaminaChanged);
		MaxStaminaChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            AttributeSetBase->GetMaxStaminaAttribute()).AddUObject(this, &ABlasterPlayerState::MaxStaminaChanged);
		StaminaRegenRateChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            AttributeSetBase->GetStaminaRegenRateAttribute()).AddUObject(this, &ABlasterPlayerState::StaminaRegenRateChanged);
		AttackPowerChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            AttributeSetBase->GetAttackPowerAttribute()).AddUObject(this, &ABlasterPlayerState::AttackPowerChanged);
        AttackSpeedChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            AttributeSetBase->GetAttackSpeedAttribute()).AddUObject(this, &ABlasterPlayerState::AttackSpeedChanged);
        DamageTypeChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			AttributeSetBase->GetDamageTypeAttribute()).AddUObject(this, &ABlasterPlayerState::DamageTypeChanged);
		HitTypeChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			AttributeSetBase->GetHitTypeAttribute()).AddUObject(this, &ABlasterPlayerState::HitTypeChanged);
		MoveSpeedChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            AttributeSetBase->GetMoveSpeedAttribute()).AddUObject(this, &ABlasterPlayerState::MoveSpeedChanged);
        JumpSpeedChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            AttributeSetBase->GetJumpSpeedAttribute()).AddUObject(this, &ABlasterPlayerState::JumpSpeedChanged);
    
        // Tags change callbacks
    }
}


void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Defeats);
	DOREPLIFETIME(ABlasterPlayerState, Team);
}

void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	// SetScore(Score + ScoreAmount);
    SetScore(GetScore() + ScoreAmount);
	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			// Controller->SetHUDScore(Score);
            Controller->SetHUDScore(GetScore());
		}
	}
}

/// @brief 当 Score 属性发生变化时，更新 HUD 的分数显示
void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			// Controller->SetHUDScore(Score);
            Controller->SetHUDScore(GetScore());
		}
	}
}

void ABlasterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ABlasterPlayerState::OnRep_Defeats()
{
	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ABlasterPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;

	ABlasterCharacter* BCharacter = Cast <ABlasterCharacter>(GetPawn());
	if (BCharacter)
	{
		BCharacter->SetTeamColor(Team);
	}
}

void ABlasterPlayerState::OnRep_Team()
{
	ABlasterCharacter* BCharacter = Cast <ABlasterCharacter>(GetPawn());
	if (BCharacter)
	{
		BCharacter->SetTeamColor(Team);
	}
}

#pragma region Getters

UAbilitySystemComponent* ABlasterPlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

UBlasterAttributeSetBase* ABlasterPlayerState::GetAttributeSetBase() const
{
    return AttributeSetBase;
}

bool ABlasterPlayerState::IsAlive() const
{
    return GetHealth() > 0.0f;
}

float ABlasterPlayerState::GetHealth() const
{
    return AttributeSetBase->GetHealth();
}

float ABlasterPlayerState::GetMaxHealth() const
{
    return AttributeSetBase->GetMaxHealth();
}

float ABlasterPlayerState::GetHealthRegenRate() const
{
    return AttributeSetBase->GetHealthRegenRate();
}

float ABlasterPlayerState::GetMana() const
{
    return AttributeSetBase->GetMana();
}

float ABlasterPlayerState::GetMaxMana() const
{
    return AttributeSetBase->GetMaxMana();
}

float ABlasterPlayerState::GetManaRegenRate() const
{
    return AttributeSetBase->GetManaRegenRate();
}

float ABlasterPlayerState::GetStamina() const
{
    return AttributeSetBase->GetStamina();
}

float ABlasterPlayerState::GetMaxStamina() const
{
    return AttributeSetBase->GetMaxStamina();
}

float ABlasterPlayerState::GetStaminaRegenRate() const
{
    return AttributeSetBase->GetStaminaRegenRate();
}

float ABlasterPlayerState::GetAttackPower() const
{
    return AttributeSetBase->GetAttackPower();
}

float ABlasterPlayerState::GetAttackSpeed() const
{
    return AttributeSetBase->GetAttackSpeed();
}

float ABlasterPlayerState::GetDamageType() const
{
	return AttributeSetBase->GetDamageType();
}

float ABlasterPlayerState::GetHitType() const
{
	return AttributeSetBase->GetHitType();
}

float ABlasterPlayerState::GetMoveSpeed() const
{
    return AttributeSetBase->GetMoveSpeed();
}

float ABlasterPlayerState::GetJumpSpeed() const
{
    return AttributeSetBase->GetJumpSpeed();
}

#pragma endregion

#pragma region Attribute changed callbacks

void ABlasterPlayerState::HealthChanged(const FOnAttributeChangeData& Data)
{
    float Health = Data.NewValue;
	Character = Cast<ABlasterCharacter>(GetPawn());

	// Status Bar & HUD
	if (Character)
	{
		UFloatStatusBarWidget* StatusBar = Character->GetFloatingStatusBar();
		if (StatusBar)
		{
			StatusBar->SetVisibility(ESlateVisibility::Visible);
			StatusBar->SetHealthPercentage(Health / GetMaxHealth());
		}

		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
            Controller->SetHUDHealth(Health, GetMaxHealth());
		}
	}
}

void ABlasterPlayerState::MaxHealthChanged(const FOnAttributeChangeData& Data)
{
    float MaxHealth = Data.NewValue;
	Character = Cast<ABlasterCharacter>(GetPawn());

	// Status Bar & HUD
	if (Character)
	{
		UFloatStatusBarWidget* StatusBar = Character->GetFloatingStatusBar();
		if (StatusBar)
		{
			StatusBar->SetHealthPercentage(GetHealth() / MaxHealth);
		}

		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDHealth(GetHealth(), MaxHealth);
		}
	}
}

void ABlasterPlayerState::HealthRegenRateChanged(const FOnAttributeChangeData& Data)
{
    // SetHealthRegenRate(Data.NewValue);
}

void ABlasterPlayerState::ManaChanged(const FOnAttributeChangeData& Data)
{
    // SetMana(Data.NewValue);
}

void ABlasterPlayerState::MaxManaChanged(const FOnAttributeChangeData& Data)
{
    // SetMaxMana(Data.NewValue);
}

void ABlasterPlayerState::ManaRegenRateChanged(const FOnAttributeChangeData& Data)
{
    // SetManaRegenRate(Data.NewValue);
}

void ABlasterPlayerState::StaminaChanged(const FOnAttributeChangeData& Data)
{
    // SetStamina(Data.NewValue);
}

void ABlasterPlayerState::MaxStaminaChanged(const FOnAttributeChangeData& Data)
{
    // SetMaxStamina(Data.NewValue);
}

void ABlasterPlayerState::StaminaRegenRateChanged(const FOnAttributeChangeData& Data)
{
    // SetStaminaRegenRate(Data.NewValue);
}

void ABlasterPlayerState::AttackPowerChanged(const FOnAttributeChangeData& Data)
{
    // SetAttackPower(Data.NewValue);
}

void ABlasterPlayerState::AttackSpeedChanged(const FOnAttributeChangeData& Data)
{
    // SetAttackSpeed(Data.NewValue);
}

void ABlasterPlayerState::DamageTypeChanged(const FOnAttributeChangeData& Data)
{
	// SetDamageType(Data.NewValue);
}

void ABlasterPlayerState::HitTypeChanged(const FOnAttributeChangeData& Data)
{
	// SetHitType(Data.NewValue);
}

void ABlasterPlayerState::MoveSpeedChanged(const FOnAttributeChangeData& Data)
{
    // SetMoveSpeed(Data.NewValue);
}

void ABlasterPlayerState::JumpSpeedChanged(const FOnAttributeChangeData& Data)
{
    // SetJumpSpeed(Data.NewValue);
}

#pragma endregion

#pragma region Helper functions

void ABlasterPlayerState::ShowAbilityConfirmCancelText(bool ShowText)
{
    // ABlasterPlayerControllerGAS* PC = Cast<ABlasterPlayerControllerGAS>(GetOwner());
    // if (PC)
    // {
    //     PC->ShowAbilityConfirmCancelText(ShowText);
    // }
}

#pragma endregion


