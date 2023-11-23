// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "AbilitySystemComponent.h"

// TODO: 创建 Rocket 自己的 MovementComponent，实现自定义的移动逻辑

AProjectileRocket::AProjectileRocket()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);  // 无碰撞

	DamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DamageSphere"));
	DamageSphere->SetupAttachment(RootComponent);
	DamageSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	DamageSphere->SetSphereRadius(DamageRadius);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	// 由于仅在服务器启用碰撞检测，这里需要在客户端也开启碰撞才能触发 OnHit
	// if (!HasAuthority())
	// {
	// 	CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	// }

	// SpawnTrailSystem();

	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,
			1.f,
			0.f,
			LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
		);
	}

	StartDeActivateTimer();
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (HasAuthority())
	{
		ExplodeDamage();
	}

	SpawnHitImpact();
	DeactivateProjectile();
	StopLoopingSound();
	bShouldExplode = false;
	
	StartDestroyTimer();
}

void AProjectileRocket::ExplodeDamage()
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
			}

			UAbilitySystemComponent* HitASC = HitCharacter->GetAbilitySystemComponent();
			if (HitASC)
			{
				HitASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
			}
		}
	}
}


void AProjectileRocket::StartDeActivateTimer()
{
	GetWorldTimerManager().SetTimer(
		DeActivateTimerHandle,
		this, 
		&AProjectileRocket::DeActivateTimerFinished, 
		LifeTime
	);
}

void AProjectileRocket::DeActivateTimerFinished()
{
	if (bShouldExplode)
	{
		if (HasAuthority())
		{
			ExplodeDamage();
		}
		
		SpawnHitImpact();
		DeactivateProjectile();
		StopLoopingSound();
	}

	StartDestroyTimer();
}

void AProjectileRocket::StopLoopingSound()
{
	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}
}