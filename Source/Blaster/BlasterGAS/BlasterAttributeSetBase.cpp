// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAttributeSetBase.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
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

    FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
    UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();
    const FGameplayTagContainer& SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
    FGameplayTagContainer SpecAssetTags;
    Data.EffectSpec.GetAllAssetTags(SpecAssetTags);

    // Get the Target actor, which should be our owner
    AActor* TargetActor = nullptr;
    AController* TargetController = nullptr;
    ABlasterCharacter* TargetCharacter = nullptr;
    if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
    {
        TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
        TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
        TargetCharacter = Cast<ABlasterCharacter>(TargetActor);
    }

    // Get the Source actor
    AActor* SourceActor = nullptr;
    AController* SourceController = nullptr;
    ABlasterCharacter* SourceCharacter = nullptr;
    if (Source && Source->AbilityActorInfo.IsValid() && Source->AbilityActorInfo->AvatarActor.IsValid())
    {
        SourceActor = Source->AbilityActorInfo->AvatarActor.Get();
        SourceController = Source->AbilityActorInfo->PlayerController.Get();
        if (SourceController == nullptr && SourceActor != nullptr)
		{
			if (APawn* Pawn = Cast<APawn>(SourceActor))
			{
				SourceController = Pawn->GetController();
			}
		}

        // Use the controller to find the source pawn
        if (SourceController)
        {
            SourceCharacter = Cast<ABlasterCharacter>(SourceController->GetPawn());
        }
        else
        {
            SourceCharacter = Cast<ABlasterCharacter>(SourceActor);
        }

        // Set the causer actor based on context if it's set
        if (Context.GetEffectCauser())
        {
            SourceActor = Context.GetEffectCauser();
        }
    }

    if (Data.EvaluatedData.Attribute == GetDamageAttribute())
    {
        // Store a local copy of the amount of damage done and clear the damage attribute
		const float LocalDamageDone = GetDamage();
		SetDamage(0.f);

        if (LocalDamageDone > 0.f)
        {
            // If character was alive before damage is added, handle damage
			// This prevents damage being added to dead things and replaying death animations
			bool WasAlive = true;

			if (TargetCharacter)
			{
				WasAlive = TargetCharacter->IsAlive();
			}

            // Apply the health change and then clamp it
            const float OldHealth = GetHealth();
            SetHealth(FMath::Clamp(OldHealth - LocalDamageDone, 0.0f, GetMaxHealth()));

            if (TargetCharacter && WasAlive)
            {
                // PlayHitReact, this is a NetMulticast RPC so it will run on both the server and clients
                TargetCharacter->PlayHitReactMontage();

                // Show damage number
                TargetCharacter->ShowDamageNumber(LocalDamageDone, GetHitType());

                if (!TargetCharacter->IsAlive())
                {
                    // TargetCharacter was alive before this damage and now is not alive
                    BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->
		                GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;

                    if (BlasterGameMode)
                    {
                        // Tell the game mode the character was eliminated
                        BlasterGameMode->PlayerEliminated(
                            TargetCharacter, 
                            Cast<ABlasterPlayerController>(TargetController), 
                            Cast<ABlasterPlayerController>(SourceController)
                        );
                    }
                }
            }
        }
    }
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
    DOREPLIFETIME_CONDITION_NOTIFY(UBlasterAttributeSetBase, DamageType, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UBlasterAttributeSetBase, HitType, COND_None, REPNOTIFY_Always);
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

void UBlasterAttributeSetBase::OnRep_DamageType(const FGameplayAttributeData& OldDamageType)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UBlasterAttributeSetBase, DamageType, OldDamageType);
}

void UBlasterAttributeSetBase::OnRep_HitType(const FGameplayAttributeData& OldHitType)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UBlasterAttributeSetBase, HitType, OldHitType);
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

