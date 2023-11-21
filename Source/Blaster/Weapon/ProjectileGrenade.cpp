// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"

AProjectileGrenade::AProjectileGrenade()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true;
}

void AProjectileGrenade::BeginPlay()
{
	// 注意这里使用的不是 Super::BeginPlay()，因为 OnHit 的行为不同
	Super::BeginPlay();

	// 运动时忽略角色
	CollisionBox->IgnoreActorWhenMoving(GetOwner(), true);
	
	// TrailSystemTimer -> 完成后设置子弹不可见 -> 启动 DestroyTimer -> 完成后销毁子弹
	StartTrailSystemTimer();

	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BounceSound,
			GetActorLocation()
		);
	}
}

void AProjectileGrenade::StartTrailSystemTimer()
{
	GetWorldTimerManager().SetTimer(
		DestroyTimer, 
		this, 
		&AProjectileGrenade::TrailSystemTimerFinished, 
		DestroyTime
	);
}

void AProjectileGrenade::TrailSystemTimerFinished()
{
	ExplodeDamage();
	SpawnHitImpact();

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

	StartDestroyTimer();
}

void AProjectileGrenade::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
		TrailSystemTimerHandle,
		this, 
		&AProjectileGrenade::DestroyTimerFinished, 
		TrailSystemLifeTime
	);
}

void AProjectileGrenade::DestroyTimerFinished()
{
	Destroy();
}

void AProjectileGrenade::Destroyed()
{

}
