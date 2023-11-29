// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuffBoxWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UBuffBoxWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (BindWidget))
	class UImage* BuffImage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (BindWidget))
	class UTextBlock* BuffStackNumText;

	// For Local Player
	UFUNCTION(BlueprintCallable)
	void SetBuffStackNum(int32 StackNum);

	// For Other Players
	UFUNCTION(BlueprintCallable)
	void SetBuff(bool bIsVisible);
};
