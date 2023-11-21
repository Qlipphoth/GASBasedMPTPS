// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Blaster.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);  // 角色 mesh

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	// ProjectileMovementComponent->InitialSpeed = 15000.f;
	// ProjectileMovementComponent->MaxSpeed = 15000.f;
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// if (HasAuthority())
	// {
	// 	// 仅在服务器启用碰撞检测
	// 	CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	// }
	
	CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	CollisionBox->IgnoreActorWhenMoving(GetOwner(), true);
	
	SetVFX();
	SpawnTrailSystem();
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	// Destroy();
}

void AProjectile::SetVFX()
{
	switch (ProjectileType)
	{
	case EProjectileType::EPT_Normal:
		ImpactNiagara = NormalVFX.Impact;
		TrailNiagara = NormalVFX.Trail;
		break;
	case EProjectileType::EPT_Flame:
		ImpactNiagara = FlameVFX.Impact;
		TrailNiagara = FlameVFX.Trail;
		break;
	case EProjectileType::EPT_Flash:
		ImpactNiagara = FlashVFX.Impact;
		TrailNiagara = FlashVFX.Trail;
		break;
	case EProjectileType::EPT_Poison:
		ImpactNiagara = PoisonVFX.Impact;
		TrailNiagara = PoisonVFX.Trail;
		break;
	}
}

void AProjectile::SpawnTrailSystem()
{
	if (TrailNiagara)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailNiagara,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
}

void AProjectile::SpawnHitImpact()
{
	if (ImpactNiagara)
	{
		ImpactSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ImpactNiagara,
			GetActorLocation(),
			GetActorRotation()
		);
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, GetActorLocation());
	}
}

void AProjectile::ExplodeDamage()
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // World context object
				Damage, // BaseDamage
				10.f, // MinimumDamage
				GetActorLocation(), // Origin
				DamageInnerRadius, // DamageInnerRadius
				DamageOuterRadius, // DamageOuterRadius
				1.f, // DamageFalloff
				UDamageType::StaticClass(), // DamageTypeClass
				TArray<AActor*>(), // IgnoreActors
				this, // DamageCauser
				FiringController // InstigatorController
			);
		}
	}
}

void AProjectile::DeactivateProjectile()
{
	if (ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
	{
		// TrailSystemComponent->GetSystemInstance()->Deactivate();
		TrailSystemComponent->GetSystemInstanceController()->Deactivate();
	}
}

void AProjectile::StartDeActivateTimer()
{

}

void AProjectile::DeActivateTimerFinished()
{

}

void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
		DestroyTimerHandle,
		this,
		&AProjectile::DestroyTimerFinished,
		DestroyDelayTime
	);
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}

/// @brief 会在显式调用 Destroy() 时自动回调，由于 Projectile 被设置为了 replicate，因此在服务器和客户端都会调用
void AProjectile::Destroyed()
{
	Super::Destroyed();
	// SpawnHitImpact();
}

