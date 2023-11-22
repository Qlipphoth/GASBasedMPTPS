#pragma once

UENUM(BlueprintType)
enum class EBlasterGAInputID : uint8
{
    // 0 None
	None			UMETA(DisplayName = "None"),
    // 1 Confirm
	Confirm			UMETA(DisplayName = "Confirm"),
	// 2 Cancel
	Cancel			UMETA(DisplayName = "Cancel"),
    // 3 Sprint
    Sprint			UMETA(DisplayName = "Sprint"),
    // 4 Ability1
    Ability1		UMETA(DisplayName = "Ability1"),
    // 5 Ability2
    Ability2		UMETA(DisplayName = "Ability2"),
    // 6 Ability3
    Ability3		UMETA(DisplayName = "Ability3"),
    // 7 Ability4
    Ability4		UMETA(DisplayName = "Ability4")
};