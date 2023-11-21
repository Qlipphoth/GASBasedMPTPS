// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "../../GameplayEnums.h"
#include "BlasterGA.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterGA : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UBlasterGA();

	// Abilities with this set will automatically activate when the input is pressed
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EBlasterGAInputID AbilityInputID = EBlasterGAInputID::EID_None;
	
	// Value to associate an ability with an slot without tying it to an automatically activated input.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EBlasterGAInputID AbilityID = EBlasterGAInputID::EID_None;

	// Tells an ability to activate immediately when its granted. 
	// Used for passive abilities and abilities forced on others.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool bActivateAbilityOnGranted = false;

	// If an ability is marked as 'ActivateAbilityOnGranted', activate them immediately when given here
	// Epic's comment: Projects may want to initiate passives or do other "BeginPlay" type of logic here.
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
};
