// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"
#include "Announcement.h"
#include "ElimAnnouncement.h"
#include "Components/HorizontalBox.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/Texture2D.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ABlasterHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ABlasterHUD::AddCharacterOverlay()
{
	OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;
	if (OwningPlayer && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(OwningPlayer, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::AddAnnouncement()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && AnnouncementClass)
	{
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		Announcement->AddToViewport();
	}
}

void ABlasterHUD::DrawHUD()
{
    Super::DrawHUD();

    FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
        float SpreadScaled = HUDPackage.CrosshairSpread * CrosshairSpreadMax;

		if (HUDPackage.CrosshairsCenter)
		{
			DrawCrosshair(HUDPackage.CrosshairsCenter, 
				ViewportCenter, FVector2D(0.f, 0.f), HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			DrawCrosshair(HUDPackage.CrosshairsLeft, 
				ViewportCenter, FVector2D(-SpreadScaled, 0.f), HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			DrawCrosshair(HUDPackage.CrosshairsRight, 
				ViewportCenter, FVector2D(SpreadScaled, 0.f), HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsTop)
		{
			DrawCrosshair(HUDPackage.CrosshairsTop, 
				ViewportCenter, FVector2D(0.f, -SpreadScaled), HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			DrawCrosshair(HUDPackage.CrosshairsBottom, 
				ViewportCenter, FVector2D(0.f, SpreadScaled), HUDPackage.CrosshairsColor);
		}
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, 
	FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairsColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
    // 调整位置至正中央
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairsColor
	);
}

void ABlasterHUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
	OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;
	if (OwningPlayer && ElimAnnouncementClass)
	{
		UElimAnnouncement* ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwningPlayer, ElimAnnouncementClass);
		if (ElimAnnouncementWidget)
		{
			ElimAnnouncementWidget->SetElimAnnouncementText(Attacker, Victim);
			ElimAnnouncementWidget->AddToViewport();
		}

		// TODO: 动画效果

		for (UElimAnnouncement* Msg : ElimMessages)
		{
			if (Msg && Msg->AnnouncementBox)
			{
				UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
				if (CanvasSlot)
				{
					FVector2D Position = CanvasSlot->GetPosition();
					FVector2D NewPosition(
						CanvasSlot->GetPosition().X,
						Position.Y - CanvasSlot->GetSize().Y
					);
					CanvasSlot->SetPosition(NewPosition);
				}
			}
		}

		ElimMessages.Add(ElimAnnouncementWidget);

		FTimerHandle ElimMsgTimer;
		FTimerDelegate ElimMsgDelegate;
		ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncementWidget);
		GetWorldTimerManager().SetTimer(
			ElimMsgTimer,
			ElimMsgDelegate,
			ElimAnnouncementTime,
			false
		);
	}
}

void ABlasterHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{
	if (MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();
	}
}

void ABlasterHUD::SetHUDHealth(float Health, float MaxHealth)
{
	if (CharacterOverlay)
	{
		CharacterOverlay->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterHUD::SetHUDShield(float Shield, float MaxShield)
{
	if (CharacterOverlay)
	{
		CharacterOverlay->SetHUDShield(Shield, MaxShield);
	}
}

void ABlasterHUD::SetHUDScore(float Score)
{
	if (CharacterOverlay)
	{
		CharacterOverlay->SetHUDScore(Score);
	}
}

void ABlasterHUD::SetHUDDefeats(int32 Defeats)
{
	if (CharacterOverlay)
	{
		CharacterOverlay->SetHUDDefeats(Defeats);
	}
}

void ABlasterHUD::SetHUDWeaponAmmo(int32 Ammo)
{
	if (CharacterOverlay)
	{
		CharacterOverlay->SetHUDWeaponAmmo(Ammo);
	}
}

void ABlasterHUD::SetHUDCarriedAmmo(int32 Ammo)
{
	if (CharacterOverlay)
	{
		CharacterOverlay->SetHUDCarriedAmmo(Ammo);
	}
}

void ABlasterHUD::SetHUDGrenades(int32 Grenades)
{
	if (CharacterOverlay)
	{
		CharacterOverlay->SetHUDGrenades(Grenades);
	}
}

void ABlasterHUD::SetBuffBar(FGameplayTag BuffTag, int32 StackNum)
{
	if (CharacterOverlay)
	{
		CharacterOverlay->SetBuffBar(BuffTag, StackNum);
	}
}

void ABlasterHUD::OnSkillSet(UBlasterSkill* Skill, int32 Index)
{
	if (CharacterOverlay == nullptr)
	{
		AddCharacterOverlay();
	}
	if (CharacterOverlay)
	{
		// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("HUD::OnSkillSet"));
		CharacterOverlay->OnSkillSet(Skill, Index);
	}
}

void ABlasterHUD::OnSkillUnset(int32 Index)
{
	if (CharacterOverlay)
	{
		CharacterOverlay->OnSkillUnset(Index);
	}
}