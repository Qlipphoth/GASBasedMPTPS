// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"
#include "../GameplayEnums.h"

void AProjectileWeapon::BeginPlay()
{
	Super::BeginPlay();

	ProjectileType = InitProjectileType;
}

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	// Weapon 被设置为了 Replicate，所以只有服务器端才能发射子弹
	// 但 Projectile 也被设置为了 Replicate，所以服务器端生成的 Projectile 会被复制到客户端
	// if (!HasAuthority()) return;

	PlayFireMontage();

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	
	if (MuzzleFlashSocket && World)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector3d MuzzleLocation = SocketTransform.GetLocation();
		// From muzzle flash socket to hit location from TraceUnderCrosshairs
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;

		AProjectile* SpawnedProjectile = nullptr;
		if (bUseServerSideRewind)
		{
			// server
			if (InstigatorPawn->HasAuthority()) 
			{
				// 服务器端自己控制的角色，无需 SSR，发射 Replicated Projectile 同步到客户端即可
				if (InstigatorPawn->IsLocallyControlled()) 
				{
					SpawnProjectiles(ProjectileClass, MuzzleLocation, TargetRotation, SpawnParams);
					// SpawnedProjectile->bUseServerSideRewind = false;
				}
				// 服务器端的其他角色，需要生成不会复制到客户端的本地 Projectile，需要使用 SSR
				else 
				{
					SpawnProjectiles(ServerSideRewindProjectileClass, MuzzleLocation, TargetRotation, SpawnParams);
					// 由于服务器端的伤害计算条件是 
					// OwnerCharacter->HasAuthority() && (!bUseServerSideRewind || OwnerCharacter->IsLocallyControlled())
					// 如果这里设置为 bUseServerSideRewind = false 会导致客户端发射的子弹多造成一次伤害
					// 客户端本地调用 Rewind 造成伤害 + 服务器端的直接伤害
					// 因此这里需要设置为 bUseServerSideRewind = true
					// SpawnedProjectile->bUseServerSideRewind = true;
				}
			}
			// client, using SSR
			else 
			{
				// 客户端自己控制的角色，需要使用 SSR，并在本地生成 Projectile
				if (InstigatorPawn->IsLocallyControlled()) 
				{
					SpawnProjectiles(ServerSideRewindProjectileClass, MuzzleLocation, TargetRotation, SpawnParams);
					
					// 这里设置 Damage 实际上并不能起到作用，计算伤害使用 Server 端的数据
					// SpawnedProjectile->Damage = Damage;
					// SpawnedProjectile->bUseServerSideRewind = true;
				}
				// 客户端的其他角色，不需要 SSR，生成本地的 Projectile 即可
				else 
				{
					// TODO: 这里使用两种子弹是不是都可以
					SpawnProjectiles(ServerSideRewindProjectileClass, MuzzleLocation, TargetRotation, SpawnParams);
					// 客户端其他角色的 bUseServerSideRewind 设置也无关紧要，因为 Rewind 的伤害判定要经过
					// IsLocallyControlled() 的判断，否则不会造成伤害
					// SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
		}
		// 不使用 SSR 的情况均转到 Server 生成 Replicated Projectile 再同步到客户端
		else 
		{
			if (InstigatorPawn->HasAuthority())
			{
				SpawnProjectiles(ProjectileClass, MuzzleLocation, TargetRotation, SpawnParams);
			}
		}
	}
}


void AProjectileWeapon::SpawnProjectiles(TSubclassOf<AProjectile>& ProjectileToSpawn, FVector& SpawnLocation, FRotator& SpawnRotation, FActorSpawnParameters& SpawnParams)
{	
	for (uint32 i = 0; i < NumberOfPellets; ++i)
	{
		FRotator FinalRotation = bUseScatter ? SpawnRotation + FRotator(
			FMath::RandRange(-ScatterAngle, ScatterAngle),
			FMath::RandRange(-ScatterAngle, ScatterAngle),
			0.f) : SpawnRotation;

		FTransform FinalTransform(FinalRotation, SpawnLocation);

		// AProjectile* SpawnedProjectile = GetWorld()->SpawnActor<AProjectile>(
		// 	ProjectileToSpawn, SpawnLocation, FinalRotation, SpawnParams);
		
		// 延迟生成 Projectile 至参数设置完成
		AProjectile* SpawnedProjectile = GetWorld()->SpawnActorDeferred<AProjectile>(
			ProjectileToSpawn, 
			FinalTransform,
			SpawnParams.Owner, SpawnParams.Instigator, 
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

		SpawnedProjectile->Damage = Damage;
		SpawnedProjectile->HeadShotDamage = HeadShotDamage;

		// 将 Projectile 的 bUseServerSideRewind 设置为 Weapon 的 bUseServerSideRewind
		SpawnedProjectile->bUseServerSideRewind = bUseServerSideRewind;

		// 设置用于 PathPrediction 的参数
		SpawnedProjectile->TraceStart = SpawnLocation;
		SpawnedProjectile->InitialVelocity = 
			SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;

		SpawnedProjectile->ProjectileType = ProjectileType;

		// 设置完参数后手动生成
		SpawnedProjectile->FinishSpawning(FinalTransform);
	}
}

void AProjectileWeapon::PlayFireMontage()
{
	WeaponAnimInstance = WeaponAnimInstance == nullptr ? GetWeaponMesh()->GetAnimInstance() : WeaponAnimInstance; 
	if (WeaponAnimInstance && WeaponFireMontage)
	{
		WeaponAnimInstance->Montage_Play(WeaponFireMontage);
		switch (ProjectileType)
		{
		case EProjectileType::EPT_Normal:
			SectionName = FName("Normal");
			break;
		case EProjectileType::EPT_Flame:
			SectionName = FName("Flame");
			break;
		case EProjectileType::EPT_Flash:
			SectionName = FName("Flash");
			break;
		case EProjectileType::EPT_Poison:
			SectionName = FName("Poison");
			break;
		}
		WeaponAnimInstance->Montage_JumpToSection(SectionName, WeaponFireMontage);
	}
}

void AProjectileWeapon::Reload()
{
	Super::Reload();

	WeaponAnimInstance = WeaponAnimInstance == nullptr ? GetWeaponMesh()->GetAnimInstance() : WeaponAnimInstance; 
	if (WeaponAnimInstance && WeaponReloadMontage)
	{
		WeaponAnimInstance->Montage_Play(WeaponReloadMontage);
	}
}