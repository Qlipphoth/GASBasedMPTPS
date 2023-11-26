// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "BlasterASC.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FReceivedDamageDelegate, 
	UBlasterASC*, SourceASC,
	float, UnmitigatedDamage, 
	float, MitigatedDamage
);


/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterASC : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	bool bCharacterAbilitiesGiven = false;
	bool bStartupEffectsApplied = false;

	FReceivedDamageDelegate ReceivedDamage;

	// Called from GDDamageExecCalculation. Broadcasts on ReceivedDamage whenever this ASC receives damage.
	virtual void ReceiveDamage(UBlasterASC* SourceASC, float UnmitigatedDamage, float MitigatedDamage);
};
