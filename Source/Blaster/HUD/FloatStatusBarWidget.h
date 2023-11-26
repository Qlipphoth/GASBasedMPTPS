// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "FloatStatusBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UFloatStatusBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetHealthPercentage(float HealthPercentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetManaPercentage(float ManaPercentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCharacterName(const FText& NewName);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetDebuffBox(FGameplayTag DebuffTag, int32 StackNum);

};
