// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAttributeSetBase.h"
#include "Net/UnrealNetwork.h"

UBlasterAttributeSetBase::UBlasterAttributeSetBase()
{

}

#pragma region Overrides

void UBlasterAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    // This is called whenever attributes change, so for max health/mana 
    // we want to scale the current totals to match
    Super::PreAttributeChange(Attribute, NewValue);
}

void UBlasterAttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);
}

#pragma endregion

#pragma region Replicated Attributes

void UBlasterAttributeSetBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UBlasterAttributeSetBase, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UBlasterAttributeSetBase, MaxHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UBlasterAttributeSetBase, HealthRegenRate, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UBlasterAttributeSetBase, Mana, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UBlasterAttributeSetBase, MaxMana, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UBlasterAttributeSetBase, ManaRegenRate, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UBlasterAttributeSetBase, Stamina, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UBlasterAttributeSetBase, MaxStamina, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UBlasterAttributeSetBase, StaminaRegenRate, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UBlasterAttributeSetBase, AttackPower, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UBlasterAttributeSetBase, AttackSpeed, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UBlasterAttributeSetBase, MoveSpeed, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UBlasterAttributeSetBase, JumpSpeed, COND_None, REPNOTIFY_Always);
}

void UBlasterAttributeSetBase::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UBlasterAttributeSetBase, Health, OldHealth);
}

void UBlasterAttributeSetBase::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UBlasterAttributeSetBase, MaxHealth, OldMaxHealth);
}

void UBlasterAttributeSetBase::OnRep_HealthRegenRate(const FGameplayAttributeData& OldHealthRegenRate)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UBlasterAttributeSetBase, HealthRegenRate, OldHealthRegenRate);
}

void UBlasterAttributeSetBase::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UBlasterAttributeSetBase, Mana, OldMana);
}

void UBlasterAttributeSetBase::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UBlasterAttributeSetBase, MaxMana, OldMaxMana);
}

void UBlasterAttributeSetBase::OnRep_ManaRegenRate(const FGameplayAttributeData& OldManaRegenRate)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UBlasterAttributeSetBase, ManaRegenRate, OldManaRegenRate);
}

void UBlasterAttributeSetBase::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UBlasterAttributeSetBase, Stamina, OldStamina);
}

void UBlasterAttributeSetBase::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UBlasterAttributeSetBase, MaxStamina, OldMaxStamina);
}

void UBlasterAttributeSetBase::OnRep_StaminaRegenRate(const FGameplayAttributeData& OldStaminaRegenRate)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UBlasterAttributeSetBase, StaminaRegenRate, OldStaminaRegenRate);
}

void UBlasterAttributeSetBase::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UBlasterAttributeSetBase, AttackPower, OldAttackPower);
}

void UBlasterAttributeSetBase::OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UBlasterAttributeSetBase, AttackSpeed, OldAttackSpeed);
}

void UBlasterAttributeSetBase::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UBlasterAttributeSetBase, MoveSpeed, OldMoveSpeed);
}

void UBlasterAttributeSetBase::OnRep_JumpSpeed(const FGameplayAttributeData& OldJumpSpeed)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UBlasterAttributeSetBase, JumpSpeed, OldJumpSpeed);
}

#pragma endregion

#pragma region Helper Functions

void UBlasterAttributeSetBase::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{

}

#pragma endregion

