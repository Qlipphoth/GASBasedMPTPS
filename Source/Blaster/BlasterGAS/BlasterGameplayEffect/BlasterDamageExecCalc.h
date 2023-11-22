// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "BlasterDamageExecCalc.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterDamageExecCalc : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UBlasterDamageExecCalc();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, 
		OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

};
