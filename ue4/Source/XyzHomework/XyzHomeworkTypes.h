#pragma once

#define ECC_Climbing ECC_GameTraceChannel1
#define ECC_InteractiveVolume ECC_GameTraceChannel2
#define ECC_WallRunning ECC_GameTraceChannel3
#define ECC_Bullet ECC_GameTraceChannel5
#define ECC_Melee ECC_GameTraceChannel9

const FName CollisionProfilePawn = FName("Pawn");
const FName CollisionProfilePawnInteractiveVolume = FName("PawnInteractiveVolume");
const FName CollisionProfilePawnWallRunnable = FName("PawnWallRunnable");
const FName CollisionProfileRagdoll = FName("Ragdoll");

const FName DebugCategoryCharacterAttributes = FName("CharacterAttributes");
const FName DebugCategoryAIAttributes = FName("AIAttributes");
const FName DebugCategoryRangedWeapon = FName("RangedWeapon");
const FName DebugCategoryMeleeWeapon = FName("MeleeWeapon");

const FName AICharacterBTCurrentTargetName = FName("CurrentTarget");
const FName AICharacterBTNextLocationName = FName("NextLocation");
const FName AICharacterBTCanSeeTargetName = FName("CanSeeTarget");
const FName AICharacterBTDistanceToTargetName = FName("DistanceToTarget");