// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/BlasterComponents/BuffComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "BlasterAnimInstance.h"
#include "Blaster/Blaster.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/SphereComponent.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerStart/TeamPlayerStart.h"
#include "Blaster/BlasterGAS/BlasterASC.h"
#include "Blaster/BlasterGAS/BlasterAttributeSetBase.h"
#include "Blaster/BlasterGAS/BlasterGameplayAbility/BlasterGA.h"
#include "Blaster/BlasterTypes/InputID.h"
#include "Blaster/HUD/FloatStatusBarWidget.h"
#include "Blaster/HUD/DamageTextWidgetCompotent.h"

#pragma region Initialization

ABlasterCharacter::ABlasterCharacter()
{
 	PrimaryActorTick.bCanEverTick = true;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;  // 设置角色不跟随控制器旋转
	GetCharacterMovement()->bOrientRotationToMovement = true;  // 设置角色朝向移动方向

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;  // 设置了此项才能够蹲下
	// 设置胶囊体与网格不会与摄像机碰撞
	GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	GetMesh()->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);  // 设置 Mesh 的碰撞类型为自定义的 channel1

	// 设置角色旋转速度  (Pitch, Yaw, Roll)
	// GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 850.f, 0.f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	// 设置网络同步频率
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetHitBoxes();
	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
	// 隐藏 Grenaed
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}

	bInvincible	= false;

	// Only needed for Heroes placed in world and when the player is the Server.
	// On respawn, they are set up in PossessedBy.
	// When the player a client, the floating status bars are all set up in OnRep_PlayerState.
	InitializeFloatingStatusBar();
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	GetCharacterMovement()->JumpZVelocity = 1600.f;
	GetCharacterMovement()->GravityScale = 3.f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 350.f;

	if (Combat)
	{
		Combat->Character = this;
	}
	if (Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeeds(
			GetCharacterMovement()->MaxWalkSpeed, 
			GetCharacterMovement()->MaxWalkSpeedCrouched
		);
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<ABlasterPlayerController>(Controller);
		}
	}
}

void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			OnPlayerStateInitialized();

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(BlasterPlayerState))
			{
				MulticastGainedTheLead();
			}
		}
	}
}

void ABlasterCharacter::UpdateHUDAmmo()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController && Combat && Combat->EquippedWeapon)
	{
		BlasterPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		BlasterPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

#pragma endregion

#pragma region ReplicatedProps

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	// Component 不需要注册进 ReplicatedProps

	DOREPLIFETIME(ABlasterCharacter, HP);
	DOREPLIFETIME(ABlasterCharacter, Shield);

	// DOREPLIFETIME_CONDITION(ABlasterCharacter, AO_Yaw, COND_SkipOwner);
	// DOREPLIFETIME_CONDITION(ABlasterCharacter, AO_Pitch, COND_SkipOwner);
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	// SimProxiesTurn();
	// TimeSinceLastMovementReplication = 0.f;
}

#pragma endregion

#pragma region Overrides

void ABlasterCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
    if (BlasterPlayerState)
    {
        // Set the ASC on the Server. Clients do this in OnRep_PlayerState()
        AbilitySystemComponent = Cast<UBlasterASC>(BlasterPlayerState->GetAbilitySystemComponent());
        
        BlasterPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(BlasterPlayerState, this);

        // Set the AttributeSetBase for convenience attribute functions
        AttributeSetBase = BlasterPlayerState->GetAttributeSetBase();

        // If we handle players disconnecting and rejoining in the future, 
		// we'll have to change this so that possession from rejoining doesn't reset attributes.
		// For now assume possession = spawn/respawn.
		InitializeAttributes();

        AddStartupEffects();

        AddInitialAbilities();

		// Set Health/Mana/Stamina to their max. This is only necessary for *Respawn*.
        SetHealth(GetMaxHealth());
        SetMana(GetMaxMana());
        SetStamina(GetMaxStamina());

        // HUD
		BlasterPlayerController = BlasterPlayerController == nullptr ? 
			Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
		if (BlasterPlayerController)
		{
			BlasterPlayerController->CreateHUD();
		}

        InitializeFloatingStatusBar();
    }
}

void ABlasterCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
    if (BlasterPlayerState)
    {
        // Set the ASC for clients. Server does this in PossessedBy.
        AbilitySystemComponent = Cast<UBlasterASC>(BlasterPlayerState->GetAbilitySystemComponent());

        // Init ASC Actor Info for clients. Server will init its ASC when it possesses a new Actor.
        BlasterPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(BlasterPlayerState, this);

        // Bind player input to the AbilitySystemComponent. 
		// Also called in SetupPlayerInputComponent because of a potential race condition.
        BindASCInput();

        // Set the AttributeSetBase for convenience attribute functions
        AttributeSetBase = BlasterPlayerState->GetAttributeSetBase();

        // If we handle players disconnecting and rejoining in the future, we'll have to change this so that posession from rejoining doesn't reset attributes.
		// For now assume possession = spawn/respawn.
        InitializeAttributes();

		// Set Health/Mana/Stamina to their max. This is only necessary for *Respawn*.
		SetHealth(GetMaxHealth());
		SetMana(GetMaxMana());
		SetStamina(GetMaxStamina());

        // HUD
		BlasterPlayerController = BlasterPlayerController == nullptr ? 
			Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
		if (BlasterPlayerController)
		{
			BlasterPlayerController->CreateHUD();
		}

		// Simulated on proxies don't have their PlayerStates yet when BeginPlay is called so we call it again here
		InitializeFloatingStatusBar();
    }
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ABlasterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ABlasterCharacter::GrenadeButtonPressed);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);

	BindASCInput();
}

#pragma endregion

#pragma region GAS

void ABlasterCharacter::BindASCInput()
{
    if (!ASCInputBound && AbilitySystemComponent.IsValid() && IsValid(InputComponent))
    {
        AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, 
            FGameplayAbilityInputBinds(
                FString("ConfirmTarget"),
                FString("CancelTarget"),
                FString("EBlasterGAInputID"),
                static_cast<int32>(EBlasterGAInputID::Confirm),
                static_cast<int32>(EBlasterGAInputID::Cancel)
            )
        );
    }

    ASCInputBound = true;
}

UAbilitySystemComponent* ABlasterCharacter::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent.Get();
}

int32 ABlasterCharacter::GetAbilityLevel(EBlasterGAInputID AbilityID) const
{
    return 1;
}

void ABlasterCharacter::RemoveCharacterAbilities()
{
    if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent.IsValid() || 
        !AbilitySystemComponent->bCharacterAbilitiesGiven)
    {
        return;
    }

    // Remove any abilities added from a previous call. This checks to make sure the ability is in the startup 'CharacterAbilities' array.
	TArray<FGameplayAbilitySpecHandle> AbilitiesToRemove;
    for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
    {
        if ((Spec.SourceObject == this) && InitialAbilities.Contains(Spec.Ability->GetClass()))
        {
            AbilitiesToRemove.Add(Spec.Handle);
        }
    }

    // Do in two passes so the removal happens after we have the full list
    for (int32 i = 0; i < AbilitiesToRemove.Num(); i++)
	{
		AbilitySystemComponent->ClearAbility(AbilitiesToRemove[i]);
	}

	AbilitySystemComponent->bCharacterAbilitiesGiven = false;
}

void ABlasterCharacter::AddInitialAbilities()
{
    if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent.IsValid() || 
        AbilitySystemComponent->bCharacterAbilitiesGiven)
    {
        return;
    }

    // Grant abilities, but only on the server	
    for (TSubclassOf<UBlasterGA>& StartupAbility : InitialAbilities)
    {
        AbilitySystemComponent->GiveAbility(
            FGameplayAbilitySpec(
                StartupAbility, 
                GetAbilityLevel(StartupAbility.GetDefaultObject()->AbilityID),
                static_cast<int32>(StartupAbility.GetDefaultObject()->AbilityInputID),
                this
            )
        );
    }

    AbilitySystemComponent->bCharacterAbilitiesGiven = true;
}

void ABlasterCharacter::InitializeAttributes()
{
    if (!AbilitySystemComponent.IsValid())
	{
		return;
	}

	if (!InitialAttributes)
    {
        UE_LOG(LogTemp, Warning, TEXT("No InitialAttributes set for %s. Please set in Blueprint."), *GetName());
        return;
    }

    // Can run on Server and Client
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

    FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(
        InitialAttributes, 1, EffectContext);

    if (NewHandle.IsValid())
    {
        FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->
            ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), AbilitySystemComponent.Get());
    }
}

void ABlasterCharacter::AddStartupEffects()
{
    if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent.IsValid() || 
        AbilitySystemComponent->bStartupEffectsApplied)
    {
        return;
    }

    FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

    // Apply startup gameplay effects to actor
    for (TSubclassOf<UGameplayEffect>& StartupEffect : StartupEffects)
    {
        FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(
            StartupEffect, 1, EffectContext);

        if (NewHandle.IsValid())
        {
            FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->
                ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), AbilitySystemComponent.Get());
        }
    }

    AbilitySystemComponent->bStartupEffectsApplied = true;
}

#pragma endregion

#pragma region Move

void ABlasterCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		// 以控制器的旋转方向作为角色的移动方向
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		// 通过旋转矩阵获取 X 轴的单位向量
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		// 通过旋转矩阵获取 Y 轴的单位向量
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		// InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 10.f);

		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABlasterCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	// UE_LOG(LogTemp, Warning, TEXT("ProxyYaw: %f"), ProxyYaw);

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

}

void ABlasterCharacter::Jump()
{
	if (Combat && Combat->bHoldingTheFlag) return;
	if (bIsCrouched)
	{
		UnCrouch();

	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag) return;
	if (bIsCrouched)
	{
		
		UnCrouch();
	}
	else
	{
		// 如同 Jump，Crouch 也是在 Character 中就有的功能，并且本身也已经 replicated
		// Crouch 会自动改变碰撞体的大小以及角色的移动速度，十分方便
		Crouch();
	}
}

#pragma endregion

#pragma region Combat

FVector ABlasterCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}

#pragma endregion

#pragma region EquipWeapon

void ABlasterCharacter::EquipButtonPressed()
{
	if (Combat)
	{
		// 调用 Server RPC，因此装备武器的操作只会在服务器上执行
		// 再由服务器同步给客户端
		// ServerEquipButtonPressed();
		if (Combat->CombatState == ECombatState::ECS_Unoccupied) {
			ServerEquipButtonPressed();
		}

		bool bSwap = Combat->ShouldSwapWeapons() && 
			!HasAuthority() && 
			Combat->CombatState == ECombatState::ECS_Unoccupied && 
			OverlappingWeapon == nullptr;

		if (bSwap)	
		{
			PlaySwapMontage();
			Combat->CombatState = ECombatState::ECS_SwappingWeapons;
			bFinishedSwapping = false;
		}
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else if (Combat->ShouldSwapWeapons())
		{
			Combat->SwapWeapons();
		}
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	
	OverlappingWeapon = Weapon;

	// for determining whether an actor is being controlled by the local player or not
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

bool ABlasterCharacter::IsWeaponEquipped() const
{
	// 必须设置了 EuqippedWeapon 为 replicated 才能在客户端也同步信息
	return (Combat && Combat->EquippedWeapon);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon() const
{
	if (Combat) return Combat->EquippedWeapon;
	return nullptr;
}

#pragma endregion

#pragma region SwapWeapon

void ABlasterCharacter::PlaySwapMontage()
{
	AnimInstance = AnimInstance == nullptr ? GetMesh()->GetAnimInstance() : AnimInstance;
	if (AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
}

#pragma endregion

#pragma region Fire

void ABlasterCharacter::FireButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag) return;
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if (Combat && Combat->bHoldingTheFlag) return;
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	AnimInstance = AnimInstance == nullptr ? GetMesh()->GetAnimInstance() : AnimInstance;
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("FireAim") : FName("FireHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

#pragma endregion

#pragma region Aim

bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

void ABlasterCharacter::AimButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag) return;
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (Combat && Combat->bHoldingTheFlag) return;
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	return Velocity.Size();
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	// FString mode = GetNetMode() == NM_Client ? TEXT("Client") : TEXT("Server");

	if (Speed == 0.f && !bIsInAir) // standing still, not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		// FRotator CurrentAimRotation = FRotator(0.f, GetControlRotation().Yaw, 0.f);

		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(
			CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;

		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);

		// GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(
		// 	TEXT("Mode: %s, AO_Yaw: %f"), *mode, AO_Yaw));
		// GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(
		// 	TEXT("Mode: %s, CurrentAimRotation: %s"), *mode, *CurrentAimRotation.ToString()));
		// GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(
		// 	TEXT("Mode: %s, StartingAimRotation: %s"), *mode, *StartingAimRotation.ToString()));
	}

	if (Speed > 0.f || bIsInAir) // running, or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		// GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, TEXT("Set AO_Yaw to 0"));
	}

	CalculateAO_Pitch();

	// GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(
	// 	TEXT("Mode: %s, AO_Pitch: %f"), *mode, AO_Pitch));
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

#pragma endregion

#pragma region Reload

void ABlasterCharacter::ReloadButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag) return;
	if (Combat)
	{
		Combat->Reload();
	}
}

void ABlasterCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	AnimInstance = AnimInstance == nullptr ? GetMesh()->GetAnimInstance() : AnimInstance;
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("Sniper");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("Grenade");
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

#pragma endregion

#pragma region ThrowGrenade

void ABlasterCharacter::GrenadeButtonPressed()
{
	if (Combat)
	{
		Combat->ThrowGrenade();
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage()
{
	AnimInstance = AnimInstance == nullptr ? GetMesh()->GetAnimInstance() : AnimInstance;
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

#pragma endregion

#pragma region Health

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (HP < LastHealth)
	{
		PlayHitReactMontage();  // 受击动画同步到客户端
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? 
		Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(HP, MaxHP);
	}
}

#pragma endregion

#pragma region Shield

// =========================================== Shield =========================================== //

void ABlasterCharacter::UpdateHUDShield()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

#pragma endregion

#pragma region UI

void ABlasterCharacter::InitializeFloatingStatusBar()
{
	// Only create once
	if (FloatingStatusBar || !AbilitySystemComponent.IsValid())
	{
		return;
	}

	// Setup UI for Locally Owned Players only, not AI or the server's copy of the PlayerControllers
	ABlasterPlayerController* PC = Cast<ABlasterPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PC && PC->IsLocalPlayerController())
	{
		if (FloatingStatusBarClass)
		{
			FloatingStatusBar = CreateWidget<UFloatStatusBarWidget>(PC, FloatingStatusBarClass);
			if (FloatingStatusBar && OverheadWidget)
			{
				OverheadWidget->SetWidget(FloatingStatusBar);

				// Setup the floating status bar
				FloatingStatusBar->SetHealthPercentage(GetHealth() / GetMaxHealth());
				// FloatingStatusBar->SetShieldPercentage(GetShield() / GetMaxShield());
			}
		}
	}
}

void ABlasterCharacter::ShowDamageNumber_Implementation(float DamageAmount)
{
	UDamageTextWidgetCompotent* DamageTextWidgetComponent = 
		NewObject<UDamageTextWidgetCompotent>(this, DamageTextWidgetClass);

	if (DamageTextWidgetComponent)
	{
		DamageTextWidgetComponent->RegisterComponent();
		DamageTextWidgetComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		DamageTextWidgetComponent->SetDamageText(DamageAmount);
	}
}

#pragma endregion

#pragma region GetDamage

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, 
	const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->
		GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	if (bInvincible || bElimmed || BlasterGameMode == nullptr) return;

	Damage = BlasterGameMode->CalculateDamage(InstigatorController, Controller, Damage);

	float DamageToHealth = Damage;
	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
			Shield = 0.f;
		}
	}

	HP = FMath::Clamp(HP - DamageToHealth, 0.f, MaxHP);

	UpdateHUDHealth();
	UpdateHUDShield();

	PlayHitReactMontage();  // 服务器端播放受击动画

	if (HP == 0.f)
	{
		// GetAuthGameMode() : Returns the current Game Mode instance cast to the template type.
		// This can only return a valid pointer on the server and may be null if the cast fails. 
		// Will always return null on a client.
		// ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();

		if (BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
}

void ABlasterCharacter::PlayHitReactMontage_Implementation()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	
	AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(FName("FromFront"));
	}
}

#pragma endregion

#pragma region Elim

void ABlasterCharacter::Elim(bool bPlayerLeftGame)
{
	// 掉落武器
	if (Combat)
	{
		// // 销毁初始武器
		// if (Combat->EquippedWeapon->bDestroyWeapon)
		// {
		// 	Combat->EquippedWeapon->Destroy();
		// }
		// else
		// {
		// 	Combat->EquippedWeapon->Dropped();
		// }
		if (Combat->EquippedWeapon) Combat->EquippedWeapon->Dropped();
		if (Combat->SecondaryWeapon) Combat->SecondaryWeapon->Dropped();
		if (Combat->TheFlag) Combat->TheFlag->Dropped();
		
	}
	// 由于 Hit 只发生在服务器上，因此 Elim 也只会发生在服务器上
	MulticastElim(bPlayerLeftGame);
}

void ABlasterCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	// 淘汰时将 HUD 的武器弹药设置为 0
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}

	bElimmed = true;
	PlayElimMontage();
	
	// Start Dissolve Effect
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	// Disable character movement
	GetCharacterMovement()->DisableMovement();          // 禁用 WASD
	GetCharacterMovement()->StopMovementImmediately();  // 禁用 旋转
	if (BlasterPlayerController)
	{
		DisableInput(BlasterPlayerController);          // 禁用
	}
	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Spawn elim bot
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
		);
	}

	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}

	bool bHideSniperScope = IsLocallyControlled() && 
		Combat && 
		Combat->bAiming && 
		Combat->EquippedWeapon && 
		Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}

	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}

	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ABlasterCharacter::ElimTimerFinished,
		ElimDelay
	);
}

void ABlasterCharacter::ElimTimerFinished()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->
		GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	if (BlasterGameMode && !bLeftGame)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

void ABlasterCharacter::ServerLeaveGame_Implementation()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->
		GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if (BlasterGameMode && BlasterPlayerState)
	{
		BlasterGameMode->PlayerLeftGame(BlasterPlayerState);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	AnimInstance = AnimInstance == nullptr ? GetMesh()->GetAnimInstance() : AnimInstance;
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	// 摧毁 Elim Bot 特效，利用 Destroyed 会在服务器和客户端都调用的特性
	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->
		GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;

	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

#pragma endregion

#pragma region HitBoxes

void ABlasterCharacter::SetHitBoxes()
{
	/** 
	* Hit boxes for server-side rewind
	*/

	head1 = CreateDefaultSubobject<USphereComponent>(TEXT("head"));
	head1->SetupAttachment(GetMesh(), FName("head"));
	// head1->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("head"), head1);	

	spine_03 = CreateDefaultSubobject<USphereComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	// spine_03->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	pelvis = CreateDefaultSubobject<USphereComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	// pelvis->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

#pragma endregion

#pragma region Getters and Setters

float ABlasterCharacter::GetHealth() const
{
    if (AttributeSetBase.IsValid())
    {
        return AttributeSetBase->GetHealth();
    }

    return 0.0f;
}

float ABlasterCharacter::GetMaxHealth() const
{
    if (AttributeSetBase.IsValid())
    {
        return AttributeSetBase->GetMaxHealth();
    }

    return 0.0f;
}

float ABlasterCharacter::GetMana() const
{
    if (AttributeSetBase.IsValid())
    {
        return AttributeSetBase->GetMana();
    }

    return 0.0f;
}

float ABlasterCharacter::GetMaxMana() const
{
    if (AttributeSetBase.IsValid())
    {
        return AttributeSetBase->GetMaxMana();
    }

    return 0.0f;
}

float ABlasterCharacter::GetStamina() const
{
    if (AttributeSetBase.IsValid())
    {
        return AttributeSetBase->GetStamina();
    }

    return 0.0f;
}

float ABlasterCharacter::GetMaxStamina() const
{
    if (AttributeSetBase.IsValid())
    {
        return AttributeSetBase->GetMaxStamina();
    }

    return 0.0f;
}

float ABlasterCharacter::GetAttackPower() const
{
    if (AttributeSetBase.IsValid())
    {
        return AttributeSetBase->GetAttackPower();
    }

    return 0.0f;
}

float ABlasterCharacter::GetAttackSpeed() const
{
    if (AttributeSetBase.IsValid())
    {
        return AttributeSetBase->GetAttackSpeed();
    }

    return 0.0f;
}

float ABlasterCharacter::GetMoveSpeed() const
{
    if (AttributeSetBase.IsValid())
    {
        return AttributeSetBase->GetMoveSpeed();
    }

    return 0.0f;
}

float ABlasterCharacter::GetMoveSpeedBaseValue() const
{
    if (AttributeSetBase.IsValid())
    {
        return AttributeSetBase->GetMoveSpeedAttribute().
            GetGameplayAttributeData(AttributeSetBase.Get())->GetBaseValue();
    }

    return 0.0f;
}

float ABlasterCharacter::GetJumpSpeed() const
{
    if (AttributeSetBase.IsValid())
    {
        return AttributeSetBase->GetJumpSpeed();
    }

    return 0.0f;
}

float ABlasterCharacter::GetJumpSpeedBaseValue() const
{
    if (AttributeSetBase.IsValid())
    {
        return AttributeSetBase->GetJumpSpeedAttribute().
            GetGameplayAttributeData(AttributeSetBase.Get())->GetBaseValue();
    }

    return 0.0f;
}

void ABlasterCharacter::SetHealth(float Health)
{
    if (AttributeSetBase.IsValid())
    {
        AttributeSetBase->SetHealth(Health);
    }
}

void ABlasterCharacter::SetMana(float Mana)
{
    if (AttributeSetBase.IsValid())
    {
        AttributeSetBase->SetMana(Mana);
    }
}

void ABlasterCharacter::SetStamina(float Stamina)
{
    if (AttributeSetBase.IsValid())
    {
        AttributeSetBase->SetStamina(Stamina);
    }
}

#pragma endregion

#pragma region Helper

/// @brief 用于隐藏角色及武器，当摄像机与角色距离过近时，只对本地玩家有效
void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		// 隐藏角色
		GetMesh()->SetVisibility(false);
		// 隐藏武器
		if (Combat)
		{
			if (Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh()) {
				// 在 CombatComponent 中已经设置了武器的拥有者为拾取的玩家
				Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;  // 设置为拥有者不可见
			}	
			if (Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh()) {
				Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;
			}
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat)
		{
			if (Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
			{
				Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
			}
			if (Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
			{
				Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
			}
		}
	}
}

bool ABlasterCharacter::IsLocallyReloading()
{
	if (Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}

#pragma endregion

#pragma region Crown

void ABlasterCharacter::MulticastGainedTheLead_Implementation()
{
	if (CrownSystem == nullptr) return;
	if (CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetCapsuleComponent(),
			FName(),
			GetActorLocation() + FVector(0.f, 0.f, 110.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
	if (CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void ABlasterCharacter::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

#pragma endregion

#pragma region Team

// TODO: Team 功能也可以用 Tag 来实现。

ETeam ABlasterCharacter::GetTeam()
{
	BlasterPlayerState = BlasterPlayerState == nullptr ? 
		GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if (BlasterPlayerState == nullptr) return ETeam::ET_NoTeam;
	return BlasterPlayerState->GetTeam();
}

void ABlasterCharacter::SetTeamColor(ETeam Team)
{
	if (GetMesh() == nullptr || OriginalMaterial == nullptr) return;
	switch (Team)
	{
	case ETeam::ET_NoTeam:
		GetMesh()->SetMaterial(0, OriginalMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;
	case ETeam::ET_BlueTeam:
		GetMesh()->SetMaterial(0, BlueMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;
	case ETeam::ET_RedTeam:
		GetMesh()->SetMaterial(0, RedMaterial);
		DissolveMaterialInstance = RedDissolveMatInst;
		break;
	}
}

void ABlasterCharacter::OnPlayerStateInitialized()
{
	BlasterPlayerState->AddToScore(0.f);
	BlasterPlayerState->AddToDefeats(0);
	SetTeamColor(BlasterPlayerState->GetTeam());
	SetSpawnPoint();
}

void ABlasterCharacter::SetSpawnPoint()
{
	if (HasAuthority() && BlasterPlayerState->GetTeam() != ETeam::ET_NoTeam)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, ATeamPlayerStart::StaticClass(), PlayerStarts);
		TArray<ATeamPlayerStart*> TeamPlayerStarts;
		for (auto Start : PlayerStarts)
		{
			ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start);
			if (TeamStart && TeamStart->Team == BlasterPlayerState->GetTeam())
			{
				TeamPlayerStarts.Add(TeamStart);
			}
		}
		if (TeamPlayerStarts.Num() > 0)
		{
			ATeamPlayerStart* ChosenPlayerStart = TeamPlayerStarts[FMath::RandRange(0, TeamPlayerStarts.Num() - 1)];
			SetActorLocationAndRotation(
				ChosenPlayerStart->GetActorLocation(),
				ChosenPlayerStart->GetActorRotation()
			);
		}
	}	
}

#pragma endregion

#pragma region Flag

bool ABlasterCharacter::IsHoldingTheFlag() const
{
	if (Combat == nullptr) return false;
	return Combat->bHoldingTheFlag;
}

void ABlasterCharacter::SetHoldingTheFlag(bool bHolding)
{
	if (Combat == nullptr) return;
	Combat->bHoldingTheFlag = bHolding;
}

#pragma endregion
