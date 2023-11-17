// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "DrawDebugHelpers.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if (BlasterCharacter == nullptr) return;

	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->
		GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	bIsCrouched = BlasterCharacter->bIsCrouched;
	bAiming = BlasterCharacter->IsAiming();
	TurningInPlace = BlasterCharacter->GetTurningInPlace();
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
	bElimmed = BlasterCharacter->IsElimmed();

	// Flag
	bHoldingTheFlag = BlasterCharacter->IsHoldingTheFlag();

	// Offset Yaw for Strafing
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();  // 摄像机的旋转
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());  // 角色朝向的旋转
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	// GEngine->AddOnScreenDebugMessage(0, 0.f, FColor::Blue, FString::Printf(TEXT("YawOffset: %f"), YawOffset));

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		// ERelativeTransformSpace::RTS_World : 获得世界坐标系下的位置
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(
			FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;

		BlasterCharacter->GetMesh()->TransformToBoneSpace(
			FName("hand_r"), LeftHandTransform.GetLocation(),
			 FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (BlasterCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = BlasterCharacter->GetMesh()->GetSocketTransform(
				FName("hand_r"), ERelativeTransformSpace::RTS_World);

			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(
				RightHandTransform.GetLocation(), 
				RightHandTransform.GetLocation() + 
				(RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 60.f);
		}

		// bUseFABRIK = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading;
		// bUseAimOffsets = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading;
		// bTransformRightHand = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading;

		bUseFABRIK = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;

		// 根据本地是否正处于 Reload 状态来决定是否使用 FAIBRIK
		bool bFABRIKOverride = 
			BlasterCharacter->IsLocallyControlled() &&
			BlasterCharacter->GetCombatState() != ECombatState::ECS_ThrowingGrenade &&
			BlasterCharacter->bFinishedSwapping;

		if (bFABRIKOverride)
		{
			bUseFABRIK = !BlasterCharacter->IsLocallyReloading();
		}

		bUseAimOffsets = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
		bTransformRightHand = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
	}
}