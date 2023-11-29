// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Blaster/BlasterGAS/BlasterASC.h"
#include "AbilitySystemComponent.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();
	StartDeActivateTimer();

	// FPredictProjectilePathParams PathParams;
	// PathParams.bTraceWithChannel = true;
	// PathParams.bTraceWithCollision = true;
	// PathParams.DrawDebugTime = 5.f;
	// PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	// PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	// PathParams.MaxSimTime = 4.f;
	// PathParams.ProjectileRadius = 5.f;
	// PathParams.SimFrequency = 30.f;
	// PathParams.StartLocation = GetActorLocation();
	// PathParams.TraceChannel = ECollisionChannel::ECC_Visibility;
	// PathParams.ActorsToIgnore.Add(this);

	// FPredictProjectilePathResult PathResult;

	// UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);

	ABlasterCharacter* OwnerCharacter = Cast<ABlasterCharacter>(GetOwner());
	ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(OtherActor);

	if (OwnerCharacter)
	{
		ABlasterPlayerController* OwnerController = 
			Cast<ABlasterPlayerController>(OwnerCharacter->Controller);
		if (OwnerController)
		{
			// 如果未开启服务器回滚则不以客户端的伤害为准，转而由客户端发送 RPC
			// !bUseServerSideRewind || OwnerCharacter->IsLocallyControlled() 可以保证服务器控制的角色能够正常开火
			if (OwnerCharacter->HasAuthority() && 
				(!bUseServerSideRewind || OwnerCharacter->IsLocallyControlled()))
			{
				const float DamageToCause = Hit.BoneName.ToString() == 
					FString("head") ? HeadShotDamage : Damage;

				// Pass the damage to the Damage Execution Calculation through a SetByCaller value on the GameplayEffectSpec
				if (DamageEffectSpecHandle != nullptr)
				{
					DamageEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(
						FGameplayTag::RequestGameplayTag(FName("Data.Damage")), DamageToCause);
					DamageEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(
						FGameplayTag::RequestGameplayTag(FName("Data.DamageType")), static_cast<float>(ProjectileType));
				}

				if (HitCharacter)
				{
					UAbilitySystemComponent* HitASC = HitCharacter->GetAbilitySystemComponent();
					if (HitASC)
					{
						HitASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
						
						// 额外效果，如灼烧、中毒等
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

			// 客户端（LocallyControlled）开启了回滚则以客户端发送的回滚请求得到的伤害为准
			// 否则客户端不会造成伤害
			if (!OwnerCharacter->HasAuthority() && bUseServerSideRewind)
			{ 
				if (bUseServerSideRewind && 
					OwnerCharacter->GetLagCompensation() && 
					OwnerCharacter->IsLocallyControlled() && 
					HitCharacter)
				{
					OwnerCharacter->GetLagCompensation()->ProjectileServerScoreRequest(
						HitCharacter,
						TraceStart,
						InitialVelocity,
						OwnerController->GetServerTime() - OwnerController->SingleTripTime
					);
				}
			}
		}
	}

	if (bShouldExplode)
	{
		SpawnHitImpact();
		DeactivateProjectile();
		bShouldExplode = false;
	}
	StartDestroyTimer();
}

void AProjectileBullet::StartDeActivateTimer()
{
	GetWorldTimerManager().SetTimer(
		DeActivateTimerHandle,
		this,
		&AProjectileBullet::DeActivateTimerFinished,
		LifeTime
	);
}

void AProjectileBullet::DeActivateTimerFinished()
{
	if (bShouldExplode)
	{
		SpawnHitImpact();
		DeactivateProjectile();
		bShouldExplode = false;
	}

	StartDestroyTimer();
}


#if WITH_EDITOR

/// @brief 后处理编辑器属性变化，改变 InitialSpeed 时，同步修改 ProjectileMovementComponent 的一些属性
/// @param Event 
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}

#endif