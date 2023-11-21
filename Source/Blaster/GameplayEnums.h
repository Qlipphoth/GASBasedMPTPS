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

UEnum(BlueprintType)
enum class EBlasterGAInputID : uint8
{
    // 0 None
	EID_None			UMETA(DisplayName = "None"),
    // 1 Confirm
	EID_Confirm			UMETA(DisplayName = "Confirm"),
	// 2 Cancel
	EID_Cancel			UMETA(DisplayName = "Cancel"),
    // 3 Sprint
    EID_Sprint			UMETA(DisplayName = "Sprint"),
    // 4 Ability1
    EID_Ability1		UMETA(DisplayName = "Ability1"),
    // 5 Ability2
    EID_Ability2		UMETA(DisplayName = "Ability2"),
    // 6 Ability3
    EID_Ability3		UMETA(DisplayName = "Ability3"),
    // 7 Ability4
    EID_Ability4		UMETA(DisplayName = "Ability4")
}