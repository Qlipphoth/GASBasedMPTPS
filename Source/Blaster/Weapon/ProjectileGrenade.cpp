// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"

// TODO: 创建 Grenade 自己的 MovementComponent，实现自定义的移动逻辑

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
	AActor::BeginPlay();

	// 运动时忽略角色
	CollisionBox->IgnoreActorWhenMoving(GetOwner(), true);

	// 因此必须手动 Spawn Trail System
	SpawnTrailSystem();
	
	StartDestroyTimer();
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

void AProjectileGrenade::Destroyed()
{
	ExplodeDamage();
	Super::Destroyed();
}
