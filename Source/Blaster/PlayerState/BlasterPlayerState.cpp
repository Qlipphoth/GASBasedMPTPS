// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Blaster/HUD/FloatStatusBarWidget.h"
#include "Blaster/BlasterGAS/BlasterASC.h"
#include "Blaster/BlasterGAS/BlasterAttributeSetBase.h"
#include "Blaster/BlasterTypes/InputID.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "Blaster/BlasterGAS/BlasterGameplayAbility/BlasterSkill.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blaster/BlasterComponents/CombatComponent.h"

ABlasterPlayerState::ABlasterPlayerState()
{
    // Create ability system component, and set it to be explicitly replicated
    AbilitySystemComponent = CreateDefaultSubobject<UBlasterASC>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);

    // Mixed mode means we only are replicated the GEs to ourself, not the GEs to simulated proxies. 
	// If another GDPlayerState (Hero) receives a GE, we won't be told about it by the Server. 
	// Attributes, GameplayTags, and GameplayCues will still replicate to us.

	// Full mode means we are replicating all GEs to all clients.
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Full);

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
		AbilitySystemComponent->RegisterGameplayTagEvent(
			FGameplayTag::RequestGameplayTag(FName("State.Debuff.Ignited")), 
			EGameplayTagEventType::AnyCountChange).AddUObject(this, &ABlasterPlayerState::IgnitedTagChanged);
		AbilitySystemComponent->RegisterGameplayTagEvent(
			FGameplayTag::RequestGameplayTag(FName("State.Debuff.Electrified")),
			EGameplayTagEventType::AnyCountChange).AddUObject(this, &ABlasterPlayerState::ElectrifiedTagChanged);
		AbilitySystemComponent->RegisterGameplayTagEvent(
			FGameplayTag::RequestGameplayTag(FName("State.Debuff.Stunned")),
			EGameplayTagEventType::AnyCountChange).AddUObject(this, &ABlasterPlayerState::StunnedTagChanged);
		AbilitySystemComponent->RegisterGameplayTagEvent(
			FGameplayTag::RequestGameplayTag(FName("State.Debuff.Poisoned")),
			EGameplayTagEventType::AnyCountChange).AddUObject(this, &ABlasterPlayerState::PoisonedTagChanged);
    
		// Tags change callbacks
		AbilitySystemComponent->RegisterGameplayTagEvent(
			FGameplayTag::RequestGameplayTag(FName("Ability.Projectile.Buff.Flame")),
			EGameplayTagEventType::AnyCountChange).AddUObject(this, &ABlasterPlayerState::FlameProjectileBuffTagChanged);
		AbilitySystemComponent->RegisterGameplayTagEvent(
			FGameplayTag::RequestGameplayTag(FName("Ability.Projectile.Buff.Flash")),
			EGameplayTagEventType::AnyCountChange).AddUObject(this, &ABlasterPlayerState::FlashProjectileBuffTagChanged);
		AbilitySystemComponent->RegisterGameplayTagEvent(
			FGameplayTag::RequestGameplayTag(FName("Ability.Projectile.Buff.Poison")),
			EGameplayTagEventType::AnyCountChange).AddUObject(this, &ABlasterPlayerState::PoisonProjectileBuffTagChanged);
		AbilitySystemComponent->RegisterGameplayTagEvent(
			FGameplayTag::RequestGameplayTag(FName("Ability.Projectile.Buff.Random")),
			EGameplayTagEventType::AnyCountChange).AddUObject(this, &ABlasterPlayerState::RandomProjectileBuffTagChanged);

		// AbilitySystemComponent->RegisterGameplayTagEvent(
		// 	FGameplayTag::RequestGameplayTag(FName("Ability.Projectile.Buff")),
		// 	EGameplayTagEventType::AnyCountChange).AddUObject(this, &ABlasterPlayerState::ProjectileBuffTagChanged);

		AbilitySystemComponent->RegisterGameplayTagEvent(
			FGameplayTag::RequestGameplayTag(FName("State.Buff.InfiniteAmmo")),
			EGameplayTagEventType::AnyCountChange).AddUObject(this, &ABlasterPlayerState::InfiniteAmmoTagChanged);

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
	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character && Character->GetCombat())
	{
		Character->GetCombat()->SetFireRate(Data.NewValue);
	}
}

void ABlasterPlayerState::DamageTypeChanged(const FOnAttributeChangeData& Data)
{
	// SetDamageType(Data.NewValue);
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("HitTypeChanged: %f"), Data.NewValue));

	// TODO: 显示 UI
}

void ABlasterPlayerState::HitTypeChanged(const FOnAttributeChangeData& Data)
{
	// SetHitType(Data.NewValue);
}

void ABlasterPlayerState::MoveSpeedChanged(const FOnAttributeChangeData& Data)
{
    // SetMoveSpeed(Data.NewValue);
	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
	}
}

void ABlasterPlayerState::JumpSpeedChanged(const FOnAttributeChangeData& Data)
{
    // SetJumpSpeed(Data.NewValue);
	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = Data.NewValue;
	}
}

#pragma endregion

#pragma region Tag change callbacks

void ABlasterPlayerState::IgnitedTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		// DebuffBox
		UFloatStatusBarWidget* StatusBar = Character->GetFloatingStatusBar();
		if (StatusBar)
		{
			StatusBar->SetDebuffBox(CallbackTag, NewCount);
		}

		// Niagara
		if (NewCount > 0)
		{
			Character->GetIgnitedComponent()->Activate();
		}
		else
		{
			Character->GetIgnitedComponent()->Deactivate();
		}
	}
}

void ABlasterPlayerState::ElectrifiedTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		// DebuffBox
		UFloatStatusBarWidget* StatusBar = Character->GetFloatingStatusBar();
		if (StatusBar)
		{
			StatusBar->SetDebuffBox(CallbackTag, NewCount);
		}

		// Niagara
		if (NewCount > 0)
		{
			Character->GetElectrifiedComponent()->Activate();
		}
		else
		{
			Character->GetElectrifiedComponent()->Deactivate();
		}
	}
}

void ABlasterPlayerState::StunnedTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("StunnedTagChanged: %d"), NewCount));
	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		// DebuffBox
		UFloatStatusBarWidget* StatusBar = Character->GetFloatingStatusBar();
		if (StatusBar)
		{
			StatusBar->SetDebuffBox(CallbackTag, NewCount);
		}

		// Niagara
		if (NewCount > 0)
		{
			Character->GetStunnedComponent()->Activate();
		}
		else
		{
			Character->GetStunnedComponent()->Deactivate();
		}
	}
}

void ABlasterPlayerState::PoisonedTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("PoisonedTagChanged: %d"), NewCount));
	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		// DebuffBox
		UFloatStatusBarWidget* StatusBar = Character->GetFloatingStatusBar();
		if (StatusBar)
		{
			StatusBar->SetDebuffBox(CallbackTag, NewCount);
		}

		// Niagara
		if (NewCount > 0)
		{
			Character->GetPoisonedComponent()->Activate();
		}
		else
		{
			Character->GetPoisonedComponent()->Deactivate();
		}
	}
}

void ABlasterPlayerState::FlameProjectileBuffTagChanged_Implementation(const FGameplayTag CallbackTag, int32 NewCount)

{
	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetBuffBar(CallbackTag, NewCount);
		}
	}
}

void ABlasterPlayerState::FlashProjectileBuffTagChanged_Implementation(const FGameplayTag CallbackTag, int32 NewCount)
{
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("FlashProjectileBuffTagChanged: %d"), NewCount));
	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetBuffBar(CallbackTag, NewCount);
		}
	}
}

void ABlasterPlayerState::PoisonProjectileBuffTagChanged_Implementation(const FGameplayTag CallbackTag, int32 NewCount)
{
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("PoisonProjectileBuffTagChanged: %d"), NewCount));
	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetBuffBar(CallbackTag, NewCount);
		}
	}
}

void ABlasterPlayerState::RandomProjectileBuffTagChanged_Implementation(const FGameplayTag CallbackTag, int32 NewCount)
{
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("RandomProjectileBuffTagChanged: %d"), NewCount));
	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetBuffBar(CallbackTag, NewCount);
		}
	}
}

void ABlasterPlayerState::ProjectileBuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetBuffBar(CallbackTag, NewCount);
		}
	}
}

void ABlasterPlayerState::InfiniteAmmoTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		Character->SetInfiniteAmmo(NewCount > 0);
	}
}

#pragma endregion

#pragma region Skills

int32 ABlasterPlayerState::GetSkillCount() const
{
	for (int32 i = 0; i < Skills.Num(); i++)
	{
		if (Skills[i].Ability == nullptr)
		{
			return i;
		}
	}
	return Skills.Num();
}

bool ABlasterPlayerState::CanAddSkill() const
{
	return GetSkillCount() < Skills.Num();
}

void ABlasterPlayerState::SetSkill(UBlasterSkill* NewSkill, int32 Index)
{
	if (Index < 0 || Index >= Skills.Num() || AbilitySystemComponent == nullptr)
	{
		return;
	}

	Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			// 相当于丢弃 Skill[Index]
			if (NewSkill == nullptr)
			{
				if (Skills[Index].Ability != nullptr)
				{
					// 清除 AbilitySystemComponent 中的 Ability
					AbilitySystemComponent->ClearAbility(Skills[Index].Handle);
					// UI 取消显示
					if (Controller)
					{
						Controller->OnSkillUnset(Index);
					}
				}
			}
			// 替换技能
			else
			{
				if (Skills[Index].Ability != nullptr)
				{
					AbilitySystemComponent->ClearAbility(Skills[Index].Handle);
					if (Controller)
					{
						Controller->OnSkillUnset(Index);
					}
				}

				if (Controller)
				{
					Controller->OnSkillSet(NewSkill, Index);
				}

				Skills[Index] = FGameplayAbilitySpec(NewSkill, 1, static_cast<int32>(SkillsKeyMapping[Index]), this);
				if (HasAuthority()) AbilitySystemComponent->GiveAbility(Skills[Index]);
			}
		}
	}	
}

void ABlasterPlayerState::AddInitialSkills()
{
	for (int32 i = 0; i < Skills.Num(); i++)
	{
		if (i >= InitialSkills.Num())
		{
			break;
		}
		SetSkill(InitialSkills[i].GetDefaultObject(), i);
	}
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
