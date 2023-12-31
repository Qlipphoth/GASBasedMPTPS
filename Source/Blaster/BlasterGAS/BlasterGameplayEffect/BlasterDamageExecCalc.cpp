// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterDamageExecCalc.h"
#include "Blaster/BlasterGAS/BlasterAttributeSetBase.h"
#include "Blaster/BlasterGAS/BlasterASC.h"

// Declare the attributes to capture and define how we want to capture them from the Source and Target.
struct BlasterDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);
    DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
    DECLARE_ATTRIBUTE_CAPTUREDEF(HitType);

	BlasterDamageStatics()
	{
		// Snapshot happens at time of GESpec creation

		// Capture optional Damage set on the damage GE as a CalculationModifier under the ExecutionCalculation
		DEFINE_ATTRIBUTE_CAPTUREDEF(UBlasterAttributeSetBase, Damage, Source, true);

        // Capture Source's AttackPower. Snapshot
        DEFINE_ATTRIBUTE_CAPTUREDEF(UBlasterAttributeSetBase, AttackPower, Source, false);

        // Capture Target's HitType. Not Snapshot
        DEFINE_ATTRIBUTE_CAPTUREDEF(UBlasterAttributeSetBase, HitType, Target, false);
	}
};

static const BlasterDamageStatics& DamageStatics()
{
    static BlasterDamageStatics DmgStatics;
    return DmgStatics;
}

UBlasterDamageExecCalc::UBlasterDamageExecCalc()
{
    RelevantAttributesToCapture.Add(DamageStatics().DamageDef);
    RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
    RelevantAttributesToCapture.Add(DamageStatics().HitTypeDef);
}

void UBlasterDamageExecCalc::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& ExecutionParams, 
    OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
	UAbilitySystemComponent* SourceAbilitySystemComponent = ExecutionParams.GetSourceAbilitySystemComponent();

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// Gather the tags from the source and target as that can affect which buffs should be used
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

    float AttackPower = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
        DamageStatics().AttackPowerDef, EvaluationParameters, AttackPower);
    AttackPower = FMath::Max<float>(AttackPower, 0.f);

    float Damage = 0.f;
    // Capture optional damage value set on the damage GE as a CalculationModifier under the ExecutionCalculation
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
        DamageStatics().DamageDef, EvaluationParameters, Damage);
    
    // Add SetByCaller damage if it exists
    Damage += FMath::Max<float>(Spec.GetSetByCallerMagnitude(
        FGameplayTag::RequestGameplayTag(FName("Data.Damage")), false, -1.0f), 0.0f);

    float HitType = FMath::Clamp<float>(Spec.GetSetByCallerMagnitude(
        FGameplayTag::RequestGameplayTag(FName("Data.DamageType")), false, -1.0f), 0.0f, 3.0f);

    float UnmitigatedDamage = Damage; // Can multiply any damage boosters here

    float MitigatedDamage = UnmitigatedDamage * AttackPower;

    if (MitigatedDamage > 0.f)
	{
		// Set the Target's damage meta attribute
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(
				DamageStatics().DamageProperty, 
				EGameplayModOp::Additive, 
				MitigatedDamage
			)
		);

        // Set the Target's HitType meta attribute
        OutExecutionOutput.AddOutputModifier(
            FGameplayModifierEvaluatedData(
                DamageStatics().HitTypeProperty,
                EGameplayModOp::Override,
                HitType
            )
        );
	}

    // Broadcast Damage Event to Target ASC
    UBlasterASC* TargetBlasterASC = Cast<UBlasterASC>(TargetAbilitySystemComponent);
    if (TargetBlasterASC)
    {
        UBlasterASC* SourceBlasterASC = Cast<UBlasterASC>(SourceAbilitySystemComponent);
        TargetBlasterASC->ReceiveDamage(SourceBlasterASC, UnmitigatedDamage, MitigatedDamage);
    }

}

