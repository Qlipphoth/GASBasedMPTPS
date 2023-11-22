// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGA.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

UBlasterGA::UBlasterGA()
{
    // Default to Instance Per Actor
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    // Default tags that block this ability from activating
}

void UBlasterGA::OnAvatarSet(const FGameplayAbilityActorInfo * ActorInfo, const FGameplayAbilitySpec & Spec)
{
    Super::OnAvatarSet(ActorInfo, Spec);

    if (bActivateAbilityOnGranted)
    {
        bool ActivatedAbility = ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, false);
    }
}