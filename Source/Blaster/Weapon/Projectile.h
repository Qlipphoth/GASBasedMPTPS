// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blaster/BlasterTypes/ProjectileType.h"
#include "GameplayEffect.h"
#include "Curves/CurveFloat.h"
#include "Projectile.generated.h"

USTRUCT(BlueprintType)
struct FProjectileVFX
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* Impact;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* Trail;
};


UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	/** 
	* Used with server-side rewind
	*/

	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;

	// 精确到小数点后两位
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(Replicated, EditAnywhere, Category = "Properties|ProjectileType")
	EProjectileType ProjectileType = EProjectileType::EPT_Normal;

	UPROPERTY(EditAnywhere, Category = "Properties|Movement")
	float InitialSpeed = 15000;

	// Only set this for Grenades and Rockets
	UPROPERTY(EditAnywhere, Category = "Properties|Damage")
	float Damage = 20.f;

	// Doesn't matter for Grenades and Rockets
	UPROPERTY(EditAnywhere, Category = "Properties|Damage")
	float HeadShotDamage = 40.f;

	// Only set this for Grenades and Rockets
	UPROPERTY(EditAnywhere, Category = "Properties|Damage|Explosion")
	float DamageRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = "Properties|Damage|Explosion")
	UCurveFloat* DamageFalloffCurve;

	UPROPERTY(EditAnywhere, Category = "Properties|Damage")
	FGameplayEffectSpecHandle DamageEffectSpecHandle;

	UPROPERTY(EditAnywhere, Category = "Properties|Damage")
	TArray<FGameplayEffectSpecHandle> ExtraEffectSpecHandle;

	UPROPERTY(EditAnywhere, Category = "Properties|Lifetime")
	float LifeTime = 3.f;

	UPROPERTY(EditAnywhere, Category = "Properties|Lifetime")
	float DestroyDelayTime = 1.f;

protected:
	virtual void BeginPlay() override;

	void SpawnTrailSystem();
	void SpawnHitImpact();
	
	virtual void ExplodeDamage();

	UFUNCTION()
	virtual void OnHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

protected:
	UPROPERTY(EditAnywhere, Category = "VFXs")
	FProjectileVFX NormalVFX;

	UPROPERTY(EditAnywhere, Category = "VFXs")
	FProjectileVFX FlameVFX;

	UPROPERTY(EditAnywhere, Category = "VFXs")
	FProjectileVFX FlashVFX;

	UPROPERTY(EditAnywhere, Category = "VFXs")
	FProjectileVFX PoisonVFX;

	UPROPERTY(EditAnywhere, Category = "SFXs")
	class USoundCue* ImpactSound;

protected:
	UPROPERTY()
	class UNiagaraComponent* ImpactSystemComponent;

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere)
	class USphereComponent* DamageSphere;

	void DeactivateProjectile();

	UPROPERTY()
	bool bShouldExplode = true;  // 用于防止多次爆炸

	FTimerHandle DeActivateTimerHandle;
	virtual void StartDeActivateTimer();

	UFUNCTION(BlueprintCallable)
	virtual void DeActivateTimerFinished();

	FTimerHandle DestroyTimerHandle;
	virtual void StartDestroyTimer();
	virtual void DestroyTimerFinished();

private:
	class UNiagaraSystem* ImpactNiagara;
	class UNiagaraSystem* TrailNiagara;

	void SetVFX();

public:
	UFUNCTION(BlueprintCallable)
	class UProjectileMovementComponent* GetProjectileMovementComponent() const { return ProjectileMovementComponent; }
};
