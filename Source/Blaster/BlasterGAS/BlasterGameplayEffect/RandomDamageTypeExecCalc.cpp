// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomDamageTypeExecCalc.h"
#include "Blaster/BlasterGAS/BlasterAttributeSetBase.h"
#include "Blaster/BlasterGAS/BlasterASC.h"

// Declare the attributes to capture and define how we want to capture them from the Source and Target.
struct RandomDamageTypeStatics
{
    DECLARE_ATTRIBUTE_CAPTUREDEF(DamageType);

    RandomDamageTypeStatics()
    {
        DEFINE_ATTRIBUTE_CAPTUREDEF(UBlasterAttributeSetBase, DamageType, Source, false);
    }
};

static const RandomDamageTypeStatics& DamageTypeStatics()
{
    static RandomDamageTypeStatics DmgTypeStatics;
    return DmgTypeStatics;
}

URandomDamageTypeExecCalc::URandomDamageTypeExecCalc()
{
    RelevantAttributesToCapture.Add(DamageTypeStatics().DamageTypeDef);
}

void URandomDamageTypeExecCalc::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& ExecutionParams, 
    OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();

    int32 RandomDamageType = FMath::RandRange(1, 3);

    // Set the DamageType to the random value
    OutExecutionOutput.AddOutputModifier(
        FGameplayModifierEvaluatedData(
            DamageTypeStatics().DamageTypeProperty, 
            EGameplayModOp::Override, 
            RandomDamageType
        )
    );
}