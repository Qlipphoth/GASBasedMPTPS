// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;

protected:
	// FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OntHit);

	UPROPERTY(EditAnywhere, Category = "Trace|VFXs")
	class UNiagaraSystem* ImpactNiagara;

	UPROPERTY(EditAnywhere, Category = "Trace|VFXs")
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, Category = "Trace|SFXs")
	USoundCue* HitSound;

	// UPROPERTY(EditAnywhere)
	// float Damage = 20.f;

private:
	UPROPERTY(EditAnywhere, Category = "Trace|VFXs")
	class UNiagaraSystem* BeamNiagara;

	UPROPERTY(EditAnywhere, Category = "Trace|VFXs")
	UParticleSystem* BeamParticles;

	// TODO: Complete In Montage

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;

	/**
	* Trace end with scatter
	*/

	// UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	// float DistanceToSphere = 800.f;

	// UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	// float SphereRadius = 75.f;

	// UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	// bool bUseScatter = false;
};
