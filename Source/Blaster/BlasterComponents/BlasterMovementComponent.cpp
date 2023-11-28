// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "GameplayTagContainer.h"

UBlasterMovementComponent::UBlasterMovementComponent()
{
	SprintSpeedMultiplier = 1.4f;
	ADSSpeedMultiplier = 0.5f;
}

float UBlasterMovementComponent::GetMaxSpeed() const
{
	ABlasterCharacter* Owner = Cast<ABlasterCharacter>(GetOwner());
	if (!Owner)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() No Owner"), *FString(__FUNCTION__));
		return Super::GetMaxSpeed();
	}

	if (!Owner->IsAlive())
	{
		return 0.0f;
	}

	if (Owner->GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Debuff.Stunned"))))
	{
		return 0.0f;
	}

	if (RequestToStartSprinting)
	{
		return Owner->GetMoveSpeed() * SprintSpeedMultiplier;
	}

	if (RequestToStartADS)
	{
		return Owner->GetMoveSpeed() * ADSSpeedMultiplier;
	}

	return Owner->GetMoveSpeed();
}

void UBlasterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	//The Flags parameter contains the compressed input flags that are stored in the saved move.
	//UpdateFromCompressed flags simply copies the flags from the saved move into the movement component.
	//It basically just resets the movement component to the state when the move was made so it can simulate from there.
	RequestToStartSprinting = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;

	RequestToStartADS = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
}

FNetworkPredictionData_Client * UBlasterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != NULL);

	if (!ClientPredictionData)
	{
		UBlasterMovementComponent* MutableThis = const_cast<UBlasterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FBlasterNetworkPredictionData_Client(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UBlasterMovementComponent::StartSprinting()
{
	RequestToStartSprinting = true;
}

void UBlasterMovementComponent::StopSprinting()
{
	RequestToStartSprinting = false;
}

void UBlasterMovementComponent::StartAimDownSights()
{
	RequestToStartADS = true;
}

void UBlasterMovementComponent::StopAimDownSights()
{
	RequestToStartADS = false;
}

void UBlasterMovementComponent::FBlasterSavedMove::Clear()
{
	Super::Clear();

	SavedRequestToStartSprinting = false;
	SavedRequestToStartADS = false;
}

uint8 UBlasterMovementComponent::FBlasterSavedMove::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (SavedRequestToStartSprinting)
	{
		Result |= FLAG_Custom_0;
	}

	if (SavedRequestToStartADS)
	{
		Result |= FLAG_Custom_1;
	}

	return Result;
}

bool UBlasterMovementComponent::FBlasterSavedMove::CanCombineWith(const FSavedMovePtr & NewMove, ACharacter * Character, float MaxDelta) const
{
	//Set which moves can be combined together. This will depend on the bit flags that are used.
	if (SavedRequestToStartSprinting != ((FBlasterSavedMove*)&NewMove)->SavedRequestToStartSprinting)
	{
		return false;
	}

	if (SavedRequestToStartADS != ((FBlasterSavedMove*)&NewMove)->SavedRequestToStartADS)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void UBlasterMovementComponent::FBlasterSavedMove::SetMoveFor(ACharacter * Character, float InDeltaTime, FVector const & NewAccel, FNetworkPredictionData_Client_Character & ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UBlasterMovementComponent* CharacterMovement = Cast<UBlasterMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		SavedRequestToStartSprinting = CharacterMovement->RequestToStartSprinting;
		SavedRequestToStartADS = CharacterMovement->RequestToStartADS;
	}
}

void UBlasterMovementComponent::FBlasterSavedMove::PrepMoveFor(ACharacter * Character)
{
	Super::PrepMoveFor(Character);

	UBlasterMovementComponent* CharacterMovement = Cast<UBlasterMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
	}
}

UBlasterMovementComponent::FBlasterNetworkPredictionData_Client::FBlasterNetworkPredictionData_Client(const UCharacterMovementComponent & ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UBlasterMovementComponent::FBlasterNetworkPredictionData_Client::AllocateNewMove()
{
	return FSavedMovePtr(new FBlasterSavedMove());
}
