// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponAnimInstance.h"
#include "Weapon.h"

void UWeaponAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerWeapon = Cast<AWeapon>(GetOwningComponent()->GetOwner());
}

void UWeaponAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    if (OwnerWeapon == nullptr)
    {
        OwnerWeapon = Cast<AWeapon>(GetOwningComponent()->GetOwner());
    }
}