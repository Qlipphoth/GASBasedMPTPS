// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterASC.h"

void UBlasterASC::ReceiveDamage(UBlasterASC* SourceASC, float UnmitigatedDamage, float MitigatedDamage)
{
    ReceivedDamage.Broadcast(SourceASC, UnmitigatedDamage, MitigatedDamage);
}