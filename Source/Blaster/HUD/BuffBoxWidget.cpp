// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffBoxWidget.h"
#include "Components/TextBlock.h"

void UBuffBoxWidget::SetBuffStackNum(int32 StackNum)
{
    if (BuffStackNumText)
    {
        if (StackNum <= 0)
        {
            SetVisibility(ESlateVisibility::Collapsed);
        }
        else
        {
            SetVisibility(ESlateVisibility::Visible);
            BuffStackNumText->SetText(FText::FromString(FString::FromInt(StackNum)));
        }
    }
}

void UBuffBoxWidget::SetBuff()
{
    if (BuffImage)
    {
        SetVisibility(ESlateVisibility::Visible);
    }
    if (BuffStackNumText)
    {
        // Set Empty Text
        BuffStackNumText->SetText(FText::GetEmpty());
    }
}