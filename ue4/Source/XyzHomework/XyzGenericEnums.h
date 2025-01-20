#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.generated.h"

UENUM(BlueprintType)
enum class EWallRunSide : uint8
{
	None = 0,
	Left,
	Right,
	Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECustomMovementMode : uint8
{
	CMOVE_None = 0 UMETA(DisplayName = "None"),
	CMOVE_Mantling UMETA(DisplayName = "Mantling"),
	CMOVE_Ladder UMETA(DisplayName = "Ladder"),
	CMOVE_Zipline UMETA(DisplayName = "Zipline"),
	CMOVE_WallRun UMETA(DisplayName = "Wall run"),
	CMOVE_Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDetachFromLadderMethod : uint8
{
	Fall = 0,
	ReachingTheTop,
	ReachingTheBottom,
	JumpOff,
	Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDetachFromZiplineMethod : uint8
{
	Fall = 0,
	ReachingTheEnd,
	Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDetachFromRunnableWallMethod : uint8
{
	Fall = 0,
	JumpOff,
	Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EEquipmentItemType : uint8
{
	None = 0,
	Pistol,
	Rifle,
	Shotgun,
	SniperRifle,
	Grenade,
	Knife,
	Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EEquipmentItemSlot : uint8
{
	None = 0,
	SideArm,
	PrimaryWeapon,
	SecondaryWeapon,
	PrimaryItem,
	SecondaryItem,
	MeleeWeapon,
	Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
	Single = 0,
	FullAuto,
	Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EWeaponAmmoType : uint8
{
	None = 0,
	Pistol,
	Rifle,
	Shotgun,
	SniperRifle,
	Grenade,
	RifleGrenade,
	Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EWeaponReloadType : uint8
{
	ByClip = 0,
	ByBullet,
	Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EHitRegistrationType : uint8
{
	HitScan = 0,
	Projectile,
	Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EReticleType : uint8
{
	None = 0,
	Default,
	SniperRifle,
	Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EMeleeAttackType : uint8
{
	None = 0,
	PrimaryAttack,
	SecondaryAttack,
	Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ETurretMode : uint8
{
	Dead = 0,
	Searching,
	Tracking,
	Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ETeam : uint8
{
	Enemy = 0,
	Player,
	Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EPatrolMode : uint8
{
	None = 0,
	Circle,
	PingPong,
	Max UMETA(Hidden)
};
