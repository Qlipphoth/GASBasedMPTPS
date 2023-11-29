// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"

AProjectileGrenade::AProjectileGrenade()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true;

	DamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DamageSphere"));
	DamageSphere->SetupAttachment(RootComponent);
	DamageSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	DamageSphere->SetSphereRadius(DamageRadius);
}

void AProjectileGrenade::BeginPlay()
{
	// 注意这里使用的不是 Super::BeginPlay()，因为 OnHit 的行为不同
	Super::BeginPlay();

	// 运动时忽略角色
	CollisionBox->IgnoreActorWhenMoving(GetOwner(), true);
	
	// TrailSystemTimer -> 完成后设置子弹不可见 -> 启动 DestroyTimer -> 完成后销毁子弹
	StartDeActivateTimer();

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

void AProjectileGrenade::ExplodeDamage()
{
	// 遍历 DamageSphere 内的所有 BlasterCharacter，造成伤害
	TArray<AActor*> OverlappingActors;
	DamageSphere->GetOverlappingActors(OverlappingActors, ABlasterCharacter::StaticClass());
	for (AActor* Actor : OverlappingActors)
	{
		ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(Actor);
		if (HitCharacter)
		{
			float NormalizedDistance = FVector::Distance(HitCharacter->GetActorLocation(), GetActorLocation()) / DamageRadius;
			float DamageToCause = DamageFalloffCurve->GetFloatValue(NormalizedDistance) * Damage;

			// Pass the damage to the Damage Execution Calculation through a SetByCaller value on the GameplayEffectSpec
			// 注：ExplodeDamge 不考虑 HeadShot
			if (DamageEffectSpecHandle != nullptr)
			{
				DamageEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(
					FGameplayTag::RequestGameplayTag(FName("Data.Damage")), DamageToCause);
				DamageEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(
						FGameplayTag::RequestGameplayTag(FName("Data.DamageType")), static_cast<float>(ProjectileType));
			}

			UAbilitySystemComponent* HitASC = HitCharacter->GetAbilitySystemComponent();
			if (HitASC)
			{
				HitASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());

				for (auto Handle : ExtraEffectSpecHandle)
				{
					if (Handle != nullptr)
					{
						// // 将 Instigator 设为被攻击者自己，这么做不符合逻辑但是比较方便
						// Handle.Data.Get()->GetContext().AddInstigator(HitASC->GetOwnerActor(), HitASC->GetAvatarActor());
						HitASC->ApplyGameplayEffectSpecToSelf(*Handle.Data.Get());
					}
				}
			}
		}
	}
}

void AProjectileGrenade::StartDeActivateTimer()
{
	GetWorldTimerManager().SetTimer(
		DeActivateTimerHandle,
		this, 
		&AProjectileGrenade::DeActivateTimerFinished,
		LifeTime
	);
}

void AProjectileGrenade::DeActivateTimerFinished()
{
	if (bShouldExplode)
	{
		if (HasAuthority())
		{
			ExplodeDamage();
		}

		SpawnHitImpact();
		DeactivateProjectile();
		bShouldExplode = false;
	}

	StartDestroyTimer();
}
