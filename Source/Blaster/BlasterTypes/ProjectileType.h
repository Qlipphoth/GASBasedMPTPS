#pragma once

UENUM(BlueprintType)
enum class EProjectileType : uint8
{
	EPT_Normal UMETA(DisplayName = "Normal"),
    EPT_Flame  UMETA(DisplayName = "Flame"),
    EPT_Flash  UMETA(DisplayName = "Flash"),
    EPT_Poison UMETA(DisplayName = "Poison"),

    EPT_MAX    UMETA(DisplayName = "DefaultMAX")
};