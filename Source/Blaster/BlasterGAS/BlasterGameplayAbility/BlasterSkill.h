// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterGA.h"
#include "GameplayTagContainer.h"
#include "BlasterSkill.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterSkill : public UBlasterGA
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Skill")
	class UTexture2D* SkillIcon;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Skill")
	float DurationTime;

	/*
	** WARNING: Must be consistent with the GA Configuration
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Skill")
	FGameplayTag SkillCooldownTag;

};
