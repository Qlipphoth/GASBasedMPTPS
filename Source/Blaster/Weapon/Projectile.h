// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	/** 
	* Used with server-side rewind
	*/

	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;

	// 精确到小数点后两位
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000;

	// Only set this for Grenades and Rockets
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	// Doesn't matter for Grenades and Rockets
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

	UPROPERTY(EditAnywhere, Category = "LifeSpan")
	float DestroyTime = 3.f;

protected:
	virtual void BeginPlay() override;
	void InitVFXComponents();
	
	virtual void StartDestroyTimer();
	virtual void DestroyTimerFinished();
	
	void SpawnTrailSystem();
	void SpawnHitImpact();
	void ExplodeDamage();

	UFUNCTION()
	virtual void OnHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

protected:
	UPROPERTY(EditAnywhere, Category = "VFXs|Impact")
	class UNiagaraSystem* ImpactNiagara;

	UPROPERTY(EditAnywhere, Category = "VFXs|Impact")
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, Category = "VFXs|Trail")
	class UNiagaraSystem* TrailSystem;

	UPROPERTY(EditAnywhere, Category = "VFXs|Trail")
	class UParticleSystem* Tracer;

	UPROPERTY(EditAnywhere, Category = "SFXs")
	class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere, Category = "Damage|Range")
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "Damage|Range")
	float DamageOuterRadius = 500.f;

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

	FTimerHandle DestroyTimer;
	
};
