// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "Blaster/BlasterTypes/Team.h"
#include "Blaster/BlasterTypes/InputID.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABlasterPlayerState();

	// Implement IAbilitySystemInterface
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	class UBlasterAttributeSetBase* GetAttributeSetBase() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|UI")
	void ShowAbilityConfirmCancelText(bool ShowText);

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	/**
	* Replication notifies
	*/
	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Defeats();

	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount);

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	class UBlasterASC* AbilitySystemComponent;

	UPROPERTY()
	class UBlasterAttributeSetBase* AttributeSetBase;

	FDelegateHandle HealthChangedDelegateHandle;
	FDelegateHandle MaxHealthChangedDelegateHandle;
	FDelegateHandle HealthRegenRateChangedDelegateHandle;
	FDelegateHandle ManaChangedDelegateHandle;
	FDelegateHandle MaxManaChangedDelegateHandle;
	FDelegateHandle ManaRegenRateChangedDelegateHandle;
	FDelegateHandle StaminaChangedDelegateHandle;
	FDelegateHandle MaxStaminaChangedDelegateHandle;
	FDelegateHandle StaminaRegenRateChangedDelegateHandle;
	FDelegateHandle AttackPowerChangedDelegateHandle;
	FDelegateHandle AttackSpeedChangedDelegateHandle;
	FDelegateHandle DamageTypeChangedDelegateHandle;
	FDelegateHandle HitTypeChangedDelegateHandle;
	FDelegateHandle MoveSpeedChangedDelegateHandle;
	FDelegateHandle JumpSpeedChangedDelegateHandle;

	// Attribute changed callbacks
	virtual void HealthChanged(const FOnAttributeChangeData& Data);
	virtual void MaxHealthChanged(const FOnAttributeChangeData& Data);
	virtual void HealthRegenRateChanged(const FOnAttributeChangeData& Data);
	virtual void ManaChanged(const FOnAttributeChangeData& Data);
	virtual void MaxManaChanged(const FOnAttributeChangeData& Data);
	virtual void ManaRegenRateChanged(const FOnAttributeChangeData& Data);
	virtual void StaminaChanged(const FOnAttributeChangeData& Data);
	virtual void MaxStaminaChanged(const FOnAttributeChangeData& Data);
	virtual void StaminaRegenRateChanged(const FOnAttributeChangeData& Data);
	virtual void AttackPowerChanged(const FOnAttributeChangeData& Data);
	virtual void AttackSpeedChanged(const FOnAttributeChangeData& Data);
	virtual void DamageTypeChanged(const FOnAttributeChangeData& Data);
	virtual void HitTypeChanged(const FOnAttributeChangeData& Data);
	virtual void MoveSpeedChanged(const FOnAttributeChangeData& Data);
	virtual void JumpSpeedChanged(const FOnAttributeChangeData& Data);

	// Tag change callbacks
	virtual void IgnitedTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	virtual void ElectrifiedTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	virtual void StunnedTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	virtual void PoisonedTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	// Tag change callbacks
	UFUNCTION(Client, Reliable)
	virtual void FlameProjectileBuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	virtual void FlameProjectileBuffTagChanged_Implementation(const FGameplayTag CallbackTag, int32 NewCount);

	UFUNCTION(Client, Reliable)
	virtual void FlashProjectileBuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	virtual void FlashProjectileBuffTagChanged_Implementation(const FGameplayTag CallbackTag, int32 NewCount);

	UFUNCTION(Client, Reliable)
	virtual void PoisonProjectileBuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	virtual void PoisonProjectileBuffTagChanged_Implementation(const FGameplayTag CallbackTag, int32 NewCount);

	UFUNCTION(Client, Reliable)
	virtual void RandomProjectileBuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	virtual void RandomProjectileBuffTagChanged_Implementation(const FGameplayTag CallbackTag, int32 NewCount);

	UFUNCTION(Client, Reliable)
	virtual void RageBuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	virtual void RageBuffTagChanged_Implementation(const FGameplayTag CallbackTag, int32 NewCount);

	UFUNCTION(Client, Reliable)
	virtual void InfiniteAmmoTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	virtual void InfiniteAmmoTagChanged_Implementation(const FGameplayTag CallbackTag, int32 NewCount);

	// Initial Skills
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|PlayerState|PlayerSkills")
	TArray<TSubclassOf<class UBlasterSkill>> InitialSkills;

	// Player Skills
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS|PlayerState|PlayerSkills")
	TArray<FGameplayAbilitySpec> Skills{ FGameplayAbilitySpec(), FGameplayAbilitySpec(), FGameplayAbilitySpec(), FGameplayAbilitySpec() };

	// Skills Key Mapping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|PlayerState|PlayerSkills")
	TArray<EBlasterGAInputID> SkillsKeyMapping{ EBlasterGAInputID::Ability1, EBlasterGAInputID::Ability2, EBlasterGAInputID::Ability3, EBlasterGAInputID::Ability4 };

private:

	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;

	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam;

	UFUNCTION()
	void OnRep_Team();

public:
	ETeam GetTeam() const { return Team; }
	void SetTeam(ETeam TeamToSet);

	/**
	* Getters for attributes from GDAttributeSetBase. Returns Current Value unless otherwise specified.
	*/

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|Attributes")
	float GetHealthRegenRate() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|Attributes")
	float GetMana() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|Attributes")
	float GetMaxMana() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|Attributes")
	float GetManaRegenRate() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|Attributes")
	float GetStamina() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|Attributes")
	float GetMaxStamina() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|Attributes")
	float GetStaminaRegenRate() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|Attributes")
	float GetAttackPower() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|Attributes")
	float GetAttackSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|Attributes")
	float GetDamageType() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|Attributes")
	float GetHitType() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|Attributes")
	float GetMoveSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|Attributes")
	float GetJumpSpeed() const;

	// Player Skills
	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|PlayerSkills")
	TArray<FGameplayAbilitySpec> GetSkills() { return Skills; }

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|PlayerSkills")
	int32 GetSkillCount() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|PlayerSkills")
	bool CanAddSkill() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|PlayerSkills")
	void SetSkill(class UBlasterSkill* NewSkill, int32 Index);

	UFUNCTION(BlueprintCallable, Category = "GAS|PlayerState|PlayerSkills")
	void AddInitialSkills();

};
