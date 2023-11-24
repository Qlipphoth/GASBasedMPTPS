// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "RandomDamageTypeExecCalc.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API URandomDamageTypeExecCalc : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	URandomDamageTypeExecCalc();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, 
		OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

};
