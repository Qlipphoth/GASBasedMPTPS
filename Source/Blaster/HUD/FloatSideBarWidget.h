// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "FloatSideBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UFloatSideBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetBuffBox(FGameplayTag BuffTag, int32 StackNum);
};
