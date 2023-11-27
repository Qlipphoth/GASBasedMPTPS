// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blaster/BlasterTypes/InputID.h"
#include "SkillSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API USkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SkillSlot")
	EBlasterGAInputID SkillSlotInputID = EBlasterGAInputID::None;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "SkillSlot")
	void TiedToSkill(class UBlasterSkill* Skill);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "SkillSlot")
	void UntiedFromSkill(class UBlasterSkill* Skill);

};
