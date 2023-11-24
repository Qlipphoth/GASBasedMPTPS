// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Components/TimelineComponent.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/BlasterTypes/Team.h"
#include "Blaster/BlasterTypes/InputID.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "Blaster/BlasterTypes/ProjectileType.h"
#include "BlasterCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

UCLASS()
class BLASTER_API ABlasterCharacter : 
	public ACharacter, public IInteractWithCrosshairsInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void PossessedBy(AController* NewController) override;

	// Switch on AbilityID to return individual ability levels. 
	// Hardcoded to 1 for every ability in this project.
	UFUNCTION(BlueprintCallable, Category = "GAS|Character")
	virtual int32 GetAbilityLevel(EBlasterGAInputID AbilityID) const;

	// Removes all CharacterAbilities. Can only be called by the Server. 
	// Removing on the Server will remove from Client too.
	virtual void RemoveCharacterAbilities();

	// Implement IAbilitySystemInterface
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	void         PlayFireMontage(bool bAiming);
	void 	   	 PlayReloadMontage();	
	void		 PlayElimMontage();
	
	void         PlayThrowGrenadeMontage();
	void         GrenadeButtonPressed();

	UFUNCTION(NetMulticast, Unreliable)
	void PlayHitReactMontage();
	void PlayHitReactMontage_Implementation();

	UFUNCTION(NetMulticast, Unreliable)
	void ShowDamageNumber(float DamageAmount);
	void ShowDamageNumber_Implementation(float DamageAmount);


	virtual void OnRep_ReplicatedMovement() override;
	
	void Elim(bool bPlayerLeftGame);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);

	virtual void Destroyed() override;

	UFUNCTION(BlueprintImplementableEvent)  // 在蓝图中实现
	void ShowSniperScopeWidget(bool bShowScope);

	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();

	UPROPERTY()
	// TMap<FName, class UBoxComponent*> HitCollisionBoxes;
	TMap<FName, class USphereComponent*> HitCollisionBoxes;

protected:

	//******************************** GAS References ********************************//

	TWeakObjectPtr<class UBlasterASC> AbilitySystemComponent;
	TWeakObjectPtr<class UBlasterAttributeSetBase> AttributeSetBase;

	//********************************************************************************//

	virtual void BeginPlay() override;

	// Client only
	virtual void OnRep_PlayerState() override;

	// Called from both SetupPlayerInputComponent and OnRep_PlayerState because of a potential race condition 
	// where the PlayerController might call ClientRestart which calls SetupPlayerInputComponent before the 
	// PlayerState is repped to the client so the PlayerState would be null in SetupPlayerInputComponent.
	// Conversely, the PlayerState might be repped before the PlayerController calls ClientRestart 
	// so the Actor's InputComponent would be null in OnRep_PlayerState.
	void BindASCInput();

	// Default abilities for this Character. These will be removed on Character death and 
	// regiven if Character respawns.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|Abilities")
	TArray<TSubclassOf<class UBlasterGA>> InitialAbilities;

	// Default attributes for a character for initializing on spawn/respawn.
	// This is an instant GE that overrides the values for attributes that get reset on spawn/respawn.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|Attributes")
	TSubclassOf<class UGameplayEffect> InitialAttributes;

	// These effects are only applied one time on startup
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|Effects")
	TArray<TSubclassOf<class UGameplayEffect>> StartupEffects;

	// Grant abilities on the Server. The Ability Specs will be replicated to the owning client.
	virtual void AddInitialAbilities();

	// Initialize the Character's attributes. Must run on Server but we run it on Client too
	// so that we don't have to wait. The Server's replication to the Client won't matter since
	// the values should be the same.
	virtual void InitializeAttributes();

	virtual void AddStartupEffects();

	/**
	* Setters for Attributes. Only use these in special cases like Respawning, otherwise use a GE to change Attributes.
	* These change the Attribute's Base Value.
	*/

	virtual void SetHealth(float Health);
	virtual void SetMana(float Mana);
	virtual void SetStamina(float Stamina);

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void ReloadButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn();

	void FireButtonPressed();
	void FireButtonReleased();

	virtual void Jump() override;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, 
		class AController* InstigatorController, AActor* DamageCauser);

	// Poll for any relelvant classes and initialize our HUD
	void PollInit();


	// Invicible 不需要同步，因为打击判定是在服务器上进行的，通过 Controller 设置该参数
	// 会让 Server 上的 Controller->Pawn 也变成无敌，因此逻辑不会有问题。
	bool bInvincible = false;

	// 用于造成伤害的 GE
	FGameplayEffectSpecHandle DamageEffectSpecHandle;

	//============================= UI =============================//

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI")
	TSubclassOf<class UFloatStatusBarWidget> FloatingStatusBarClass;

	UPROPERTY()
	class UFloatStatusBarWidget* FloatingStatusBar;

	UFUNCTION()
	void InitializeFloatingStatusBar();

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UDamageTextWidgetCompotent> DamageTextWidgetClass;

private:

	//*********************************** Important References *********************************//

	bool ASCInputBound = false;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;

	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;

	UPROPERTY()
	class UAnimInstance* AnimInstance;

	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

	UPROPERTY()
	class ABlasterGameState* BlasterGameState;

	//*****************************************************************************************//

	/** 
	* Blaster components
	*/

	UPROPERTY(VisibleAnywhere, Category = "Components|Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Components|Camera")
	class UCameraComponent* FollowCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Components|Widget")
	class UWidgetComponent* OverheadWidget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Components|Combat")
	class UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere, Category = "Components|Combat")
	class UBuffComponent* Buff;

	UPROPERTY(VisibleAnywhere, Category = "Components|Combat")
	class ULagCompensationComponent* LagCompensation;


	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	// UPROPERTY(Replicated)
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	/** 
	* Animation montages
	*/

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGrenadeMontage;

	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	/**
	* Player health
	*/

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHP = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float HP = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	/** 
	* Player shield
	*/

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")
	float Shield = 0.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 2.f;

	void ElimTimerFinished();

	/**
	* Dissolve effect
	*/

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	// Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	// Material instance set on the Blueprint, used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	/**
	* Elim bot
	*/

	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotSound;


	/** 
	* Grenade
	*/

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

protected:  // Hit boxes used for server-side rewind

	void SetHitBoxes();

	UPROPERTY(EditAnywhere)
	class USphereComponent* head1;

	UPROPERTY(EditAnywhere)
	class USphereComponent* spine_01;

	UPROPERTY(EditAnywhere)
	class USphereComponent* spine1_02;

	UPROPERTY(EditAnywhere)
	class USphereComponent* spine_03;

	UPROPERTY(EditAnywhere)
	class USphereComponent* pelvis;

public:	// Getter & Setter
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped() const;
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AWeapon* GetEquippedWeapon() const;
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	
	FORCEINLINE float GetHP() const { return HP; }
	FORCEINLINE void SetHP(float Amount) { HP = Amount; }
	FORCEINLINE float GetMaxHP() const { return MaxHP; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FGameplayEffectSpecHandle GetDamageEffectSpecHandle() const { return DamageEffectSpecHandle; }
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetDamageEffectSpecHandle(FGameplayEffectSpecHandle Handle) { DamageEffectSpecHandle = Handle; }

	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }

	FORCEINLINE bool IsInvincible() const { return bInvincible; }
	void SetInvincible(bool Invincible) { bInvincible = Invincible; }

	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }
	
	bool IsLocallyReloading();

	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }
	FORCEINLINE bool IsHoldingTheFlag() const;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE class UFloatStatusBarWidget* GetFloatingStatusBar() const { return FloatingStatusBar; }

	void SetFloatingStatusBarVisibility(bool Visible);

public:
	/**
	* Getters for attributes from GDAttributeSetBase
	**/

	UFUNCTION(BlueprintCallable, Category = "GAS|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Attributes")
	float GetMaxHealth() const;

	bool IsAlive() const { return GetHealth() > 0.f; }

	UFUNCTION(BlueprintCallable, Category = "GAS|Attributes")
	float GetMana() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Attributes")
	float GetMaxMana() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Attributes")
	float GetStamina() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Attributes")
	float GetMaxStamina() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Attributes")
	float GetAttackPower() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Attributes")
	float GetAttackSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Attributes")
	float GetDamageType() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Attributes")
	float GetMoveSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Attributes")
	float GetMoveSpeedBaseValue() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Attributes")
	float GetJumpSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Attributes")
	float GetJumpSpeedBaseValue() const;

// ========================= swap weapon ========================= // 
private:
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* SwapMontage;

public:
	void PlaySwapMontage();
	bool bFinishedSwapping = false;

// ========================= leave game ========================= //
public:	
	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	FOnLeftGame OnLeftGame;

private:
	bool bLeftGame = false;

// ============================ Crown =========================== //
private:
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* CrownSystem;

	UPROPERTY()
	class UNiagaraComponent* CrownComponent;

public:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

// ============================ Team =========================== //

public:
	void SetTeamColor(ETeam Team);
	ETeam GetTeam();
	void SetHoldingTheFlag(bool bHolding);

private:
	/** 
	* Team colors
	*/

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* RedDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* RedMaterial;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* BlueDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* BlueMaterial;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* OriginalMaterial;

protected:
	void SetSpawnPoint();
	void OnPlayerStateInitialized();

};
