// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "../BlasterTypes/ProjectileType.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;
	virtual void Reload() override;

protected:
	virtual void BeginPlay() override;

	// UPROPERTY(EditAnywhere, Category = "Weapon|Scatter")
	// bool bUseScatter = false;

	UPROPERTY(EditAnywhere, Category = "Weapon|Scatter")
	float ScatterAngle = 10.f;

	UPROPERTY(EditAnywhere, Category = "Weapon|MultiShot")
	uint32 NumberOfPellets = 1;

private:
	// Replicate Projectile
	UPROPERTY(EditAnywhere, Category = "Weapon|Projectile")
	TSubclassOf<class AProjectile> ProjectileClass;

	// Non-replicated Projectile
	UPROPERTY(EditAnywhere, Category = "Weapon|Projectile")
	TSubclassOf<AProjectile> ServerSideRewindProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Weapon|Projectile")
	EProjectileType InitProjectileType = EProjectileType::EPT_Normal;

	UPROPERTY(VisibleAnywhere, Category = "Weapon|Projectile")
	EProjectileType ProjectileType;

	void SpawnProjectiles(TSubclassOf<AProjectile>& ProjectileToSpawn, FVector3d& SpawnLocation, FRotator& SpawnRotation, FActorSpawnParameters& SpawnParams);

	UPROPERTY(EditAnywhere, Category = "Weapon|Animation")
	class UAnimMontage* WeaponFireMontage;

	UPROPERTY(EditAnywhere, Category = "Weapon|Animation")
	class UAnimMontage* WeaponReloadMontage;

	void PlayFireMontage();

	UPROPERTY()
	class UAnimInstance* WeaponAnimInstance;

	UPROPERTY()
	FName SectionName = FName("Normal");

public:
	EProjectileType GetProjectileType() const { return ProjectileType; }
	void SetProjectileType(EProjectileType NewType) { ProjectileType = NewType; }

};
