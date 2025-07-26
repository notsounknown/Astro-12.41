#pragma once
#include "Engine.h"
#include <vector>
#include <map>
#include <algorithm>
#include <random>

#ifndef VPS
//#define TESTING_LOOTPOOL
#endif

struct ItemRow
{
	UFortItemDefinition* Def;
	int Level = -1;
	int DropCount;
	float Weight;
	int LoadedAmmo = 0;
};

map<string, vector<ItemRow>> LootForTiers{};

int GetAmmoForDef(UFortWeaponItemDefinition* Def)
{
	if (!Def)
	{
		log_warn("Invalid Def in GetAmmoForDef\n");
		return 0;
	}

	UDataTable* Table = ((UFortWeaponItemDefinition*)Def)->WeaponStatHandle.DataTable;
	FName RowName = ((UFortWeaponItemDefinition*)Def)->WeaponStatHandle.RowName;

	MTMap<FName, void*>& RowMap = Table->RowMap;

	for (int32 /*size_t*/ i = 0; i < RowMap.Pairs.Elements.Data.Num(); i++)
	{
		auto& Pair = RowMap.Pairs.Elements.Data[i].ElementData.Value;
		FName rowName = Pair.First;
		if (!RowName.ComparisonIndex || !Pair.Second)
			continue;

		if (RowName.ComparisonIndex == rowName.ComparisonIndex)
		{
			return ((FFortRangedWeaponStats*)Pair.Second)->ClipSize;
		}
	}

	log_warn("Invalid Def in GetAmmoForDef 2\n");
	return 0;
}

void InitLooting()
{
	//AR
	ItemRow Row1{};
#ifndef TESTING_LOOTPOOL
#ifndef ONE_SHOT
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Auto_Athena_C_Ore_T02.WID_Assault_Auto_Athena_C_Ore_T02");
	Row1.Level = 0;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.15f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Auto_Athena_UC_Ore_T03.WID_Assault_Auto_Athena_UC_Ore_T03");
	Row1.Level = 1;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.30f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	Row1.Weight = 0.15f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Auto_Athena_R_Ore_T03.WID_Assault_Auto_Athena_R_Ore_T03");
	Row1.Level = 2;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.10f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	Row1.Weight = 0.35f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_VR_Ore_T03.WID_Assault_AutoHigh_Athena_VR_Ore_T03");
	Row1.Level = 3;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.05f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_SR_Ore_T03.WID_Assault_AutoHigh_Athena_SR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.01f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);
	//End AR

	//Burst Rifle
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_SemiAuto_Athena_C_Ore_T02.WID_Assault_SemiAuto_Athena_C_Ore_T02");
	Row1.Level = 0;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.15f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_SemiAuto_Athena_UC_Ore_T03.WID_Assault_SemiAuto_Athena_UC_Ore_T03");
	Row1.Level = 1;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.30f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	Row1.Weight = 0.15f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_SemiAuto_Athena_R_Ore_T03.WID_Assault_SemiAuto_Athena_R_Ore_T03");
	Row1.Level = 2;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.10f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	Row1.Weight = 0.35f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_SemiAuto_Athena_VR_Ore_T03.WID_Assault_SemiAuto_Athena_VR_Ore_T03");
	Row1.Level = 3;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.05f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_SemiAuto_Athena_SR_Ore_T03.WID_Assault_SemiAuto_Athena_SR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.01f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);
	//End Burst Rifle

	//AK
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Heavy_Athena_C_Ore_T03.WID_Assault_Heavy_Athena_C_Ore_T03");
	Row1.Level = 0;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.12f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Heavy_Athena_UC_Ore_T03.WID_Assault_Heavy_Athena_UC_Ore_T03");
	Row1.Level = 1;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.28f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	Row1.Weight = 0.13f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Heavy_Athena_R_Ore_T03.WID_Assault_Heavy_Athena_R_Ore_T03");
	Row1.Level = 2;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.07f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	Row1.Weight = 0.34f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Heavy_Athena_VR_Ore_T03.WID_Assault_Heavy_Athena_VR_Ore_T03");
	Row1.Level = 3;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.05f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Heavy_Athena_SR_Ore_T03.WID_Assault_Heavy_Athena_SR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.01f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);
	//End AK

	//Minigun
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_LMG_Athena_VR_Ore_T03.WID_Assault_LMG_Athena_VR_Ore_T03");
	Row1.Level = 3;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.04f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_LMG_Athena_SR_Ore_T03.WID_Assault_LMG_Athena_SR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.015f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1); 
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	//End minigun

	//Pump
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_Common.WID_Shotgun_Standard_Athena_Common");
	Row1.Level = 0;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.12f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_C_Ore_T03.WID_Shotgun_Standard_Athena_C_Ore_T03"); // actually uncommon
	Row1.Level = 1;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.28f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	Row1.Weight = 0.13f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_UC_Ore_T03.WID_Shotgun_Standard_Athena_UC_Ore_T03"); // actually rare
	Row1.Level = 2;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.07f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	Row1.Weight = 0.19f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_VR_Ore_T03.WID_Shotgun_Standard_Athena_VR_Ore_T03");
	Row1.Level = 3;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.05f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_SR_Ore_T03.WID_Shotgun_Standard_Athena_SR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.01f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);
	//End Pump

	//TAC
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_SemiAuto_Athena_UC_Ore_T03.WID_Shotgun_SemiAuto_Athena_UC_Ore_T03");
	Row1.Level = 0;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.12f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_SemiAuto_Athena_R_Ore_T03.WID_Shotgun_SemiAuto_Athena_R_Ore_T03");
	Row1.Level = 1;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.28f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	Row1.Weight = 0.13f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_SemiAuto_Athena_VR_Ore_T03.WID_Shotgun_SemiAuto_Athena_VR_Ore_T03");
	Row1.Level = 2;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.07f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	Row1.Weight = 0.28f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_HighSemiAuto_Athena_VR_Ore_T03.WID_Shotgun_HighSemiAuto_Athena_VR_Ore_T03");
	Row1.Level = 3;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.07f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_HighSemiAuto_Athena_SR_Ore_T03.WID_Shotgun_HighSemiAuto_Athena_SR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.04f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);
	//End TAC

	//Start supprcessed smg
	/*Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavySuppressed_Athena_C_Ore_T02.WID_Pistol_AutoHeavySuppressed_Athena_C_Ore_T02");
	Row1.Level = 0;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.10f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);*/

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavySuppressed_Athena_UC_Ore_T03.WID_Pistol_AutoHeavySuppressed_Athena_UC_Ore_T03");
	Row1.Level = 1;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.15f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavySuppressed_Athena_R_Ore_T03.WID_Pistol_AutoHeavySuppressed_Athena_R_Ore_T03");
	Row1.Level = 2;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.07f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	Row1.Weight = 0.25f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	//End Suppressed smg

	// pistol
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_SemiAuto_Athena_C_Ore_T02.WID_Pistol_SemiAuto_Athena_C_Ore_T02");
	Row1.Level = 0;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.21f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_SemiAuto_Athena_UC_Ore_T03.WID_Pistol_SemiAuto_Athena_UC_Ore_T03");
	Row1.Level = 1;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.19f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_SemiAuto_Athena_R_Ore_T03.WID_Pistol_SemiAuto_Athena_R_Ore_T03");
	Row1.Level = 2;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.15f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_Standard_Athena_VR.WID_Pistol_Standard_Athena_VR");
	Row1.Level = 1;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.10f;
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_Standard_Athena_SR.WID_Pistol_Standard_Athena_SR");
	Row1.Level = 2;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.05f;
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);
	// end pistol

	//Suppressed AR
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Suppressed_Athena_R_Ore_T03.WID_Assault_Suppressed_Athena_R_Ore_T03");
	Row1.Level = 2;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.15f;
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Suppressed_Athena_VR_Ore_T03.WID_Assault_Suppressed_Athena_VR_Ore_T03");
	Row1.Level = 3;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.11f;
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Suppressed_Athena_SR_Ore_T03.WID_Assault_Suppressed_Athena_SR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.07f;
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	//End suppressed AR

	//Rapid fire SMG
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_RapidFireSMG_Athena_UC_Ore_T03.WID_Pistol_RapidFireSMG_Athena_UC_Ore_T03");
	Row1.Level = 1;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.15f;
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	//LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_RapidFireSMG_Athena_R_Ore_T03.WID_Pistol_RapidFireSMG_Athena_R_Ore_T03");
	Row1.Level = 2;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.07f;
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	//LootForTiers["Loot_AthenaVending"].push_back(Row1);
	//End rapid fire

	//SMG
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavyPDW_Athena_C_Ore_T03.WID_Pistol_AutoHeavyPDW_Athena_C_Ore_T03");
	Row1.Level = 0;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.31f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavyPDW_Athena_UC_Ore_T03.WID_Pistol_AutoHeavyPDW_Athena_UC_Ore_T03");
	Row1.Level = 1;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.17f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	Row1.Weight = 0.13f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavyPDW_Athena_R_Ore_T03.WID_Pistol_AutoHeavyPDW_Athena_R_Ore_T03");
	Row1.Level = 2;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.07f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	Row1.Weight = 0.30f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	/*Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavyPDW_Athena_VR_Ore_T03.WID_Pistol_AutoHeavyPDW_Athena_VR_Ore_T03");
	Row1.Level = 3;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.05f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavyPDW_Athena_SR_Ore_T03.WID_Pistol_AutoHeavyPDW_Athena_SR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.01f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);*/
	//End SMG

	//Bolt Action Sniper
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_BoltAction_Scope_Athena_R_Ore_T03.WID_Sniper_BoltAction_Scope_Athena_R_Ore_T03");
	Row1.Level = 2;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.15f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_BoltAction_Scope_Athena_VR_Ore_T03.WID_Sniper_BoltAction_Scope_Athena_VR_Ore_T03");
	Row1.Level = 3;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.1f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_BoltAction_Scope_Athena_SR_Ore_T03.WID_Sniper_BoltAction_Scope_Athena_SR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.05f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);
	//End Bolt Action Sniper

	//Heavy Sniper
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_VR_Ore_T03.WID_Sniper_Heavy_Athena_VR_Ore_T03");
	Row1.Level = 3;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.07f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_SR_Ore_T03.WID_Sniper_Heavy_Athena_SR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.04f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	//End Heavy Sniper



	//RPG
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Launcher_Rocket_Athena_VR_Ore_T03.WID_Launcher_Rocket_Athena_VR_Ore_T03");
	Row1.Level = 3;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.06f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

    Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Launcher_Rocket_Athena_SR_Ore_T03.WID_Launcher_Rocket_Athena_SR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.03f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);
	//End RPG

	//Prox mine
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/Prototype/PurpleMouse/WID_Athena_PurpleMouse.WID_Athena_PurpleMouse");
	Row1.DropCount = 2;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.13f;
	LootForTiers["Airdrop"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	//End prox mine

	//Consumables
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/Grenade/Athena_Grenade.Athena_Grenade");
	Row1.DropCount = 3;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.25f;
	//LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);

	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	//LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/Shields/Athena_Shields.Athena_Shields");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.20f;
	//LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);
	LootForTiers["Loot_AthenaLlama"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/ShieldSmall/Athena_ShieldSmall.Athena_ShieldSmall");
	Row1.DropCount = 3;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.27f;
	//LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);
	LootForTiers["Loot_AthenaLlama"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/Bandage/Athena_Bandage.Athena_Bandage");
	Row1.DropCount = 5;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.21f;
	//LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);
	LootForTiers["Loot_AthenaLlama"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/Medkit/Athena_Medkit.Athena_Medkit");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.35f;
	//LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);
	LootForTiers["Loot_AthenaLlama"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/DangerGrape/WID_Athena_DangerGrape.WID_Athena_DangerGrape");
	Row1.DropCount = 3;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.07f;
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/AppleSun/WID_Athena_AppleSun.WID_Athena_AppleSun");
	Row1.DropCount = 3;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.10f;
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/C4/Athena_C4.Athena_C4");
	Row1.DropCount = 3;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.05f;
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Gameplay/Lotus/Mustache/AGID_Lotus_Mustache.AGID_Lotus_Mustache");
	Row1.DropCount = 3;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.075f;
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Traps/TID_Floor_Player_Launch_Pad_Athena.TID_Floor_Player_Launch_Pad_Athena");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.06f;
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Gameplay/SneakySnowmanV2/AGID_CoolCarpet.AGID_CoolCarpet");
	Row1.DropCount = 5;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.045f;
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	//End Consumables

	//Ammo
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy");
	Row1.DropCount = /*((UFortAmmoItemDefinition*)Row1.Def)->DropCount*/ 6;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.25f;
	LootForTiers["Loot_AthenaAmmoLarge"].push_back(Row1);
	Row1.Weight = 0.15f;
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight");
	Row1.DropCount = ((UFortAmmoItemDefinition*)Row1.Def)->DropCount;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.25f;
	LootForTiers["Loot_AthenaAmmoLarge"].push_back(Row1);
	Row1.Weight = 0.15f;
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium");
	Row1.DropCount = ((UFortAmmoItemDefinition*)Row1.Def)->DropCount;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.25f;
	LootForTiers["Loot_AthenaAmmoLarge"].push_back(Row1);
	Row1.Weight = 0.15f;
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataShells.AthenaAmmoDataShells");
	Row1.DropCount = ((UFortAmmoItemDefinition*)Row1.Def)->DropCount;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.25f;
	LootForTiers["Loot_AthenaAmmoLarge"].push_back(Row1);
	Row1.Weight = 0.15f;
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AmmoDataRockets.AmmoDataRockets");
	Row1.DropCount = /*((UFortAmmoItemDefinition*)Row1.Def)->DropCount*/ 2;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.10f;
	LootForTiers["Loot_AthenaAmmoLarge"].push_back(Row1);

	//End Ammo

	//Faction Chests
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_VR_Ore_T03.WID_Sniper_Heavy_Athena_VR_Ore_T03");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.20f;
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_SR_Ore_T03.WID_Sniper_Heavy_Athena_SR_Ore_T03");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.15f;
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Heavy_Athena_VR_Ore_T03.WID_Assault_Heavy_Athena_VR_Ore_T03");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.30f;
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Heavy_Athena_R_Ore_T03.WID_Assault_Heavy_Athena_R_Ore_T03");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.65f;
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_VR_Ore_T03.WID_Shotgun_Standard_Athena_VR_Ore_T03");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.40f;
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_SR_Ore_T03.WID_Shotgun_Standard_Athena_SR_Ore_T03");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.10f;
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	//End Faction Chests

	//RodBoxes
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/FloppingRabbit/WID_Athena_FloppingRabbit.WID_Athena_FloppingRabbit");
	Row1.DropCount = 1;
	Row1.Weight = 0.85f;
	LootForTiers["Loot_ApolloRodBox"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/HappyGhost/WID_Athena_HappyGhost.WID_Athena_HappyGhost");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = 10;
	Row1.Weight = 0.15f;
	LootForTiers["Loot_ApolloRodBox"].push_back(Row1);
	//End RodBoxes

	//Airdrops
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
	Row1.DropCount = 30;
	Row1.Weight = 0.50f;
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
	Row1.DropCount = 30;
	Row1.Weight = 0.50f;
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/MetalItemData.MetalItemData");
	Row1.DropCount = 30;
	Row1.Weight = 0.50f;
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_BoltAction_Scope_Athena_VR_Ore_T03.WID_Sniper_BoltAction_Scope_Athena_VR_Ore_T03");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.35f;
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_VR_Ore_T03.WID_Sniper_Heavy_Athena_VR_Ore_T03");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.30f;
	LootForTiers["Airdrop"].push_back(Row1);

	/*Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavyPDW_Athena_VR_Ore_T03.WID_Pistol_AutoHeavyPDW_Athena_VR_Ore_T03");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.35f;
	LootForTiers["Airdrop"].push_back(Row1);*/

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_HighSemiAuto_Athena_VR_Ore_T03.WID_Shotgun_HighSemiAuto_Athena_VR_Ore_T03");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.40f;
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_VR_Ore_T03.WID_Shotgun_Standard_Athena_VR_Ore_T03");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.30f;
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Heavy_Athena_VR_Ore_T03.WID_Assault_Heavy_Athena_VR_Ore_T03");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.32f;
	LootForTiers["Airdrop"].push_back(Row1);
	//End Airdrops

	//Refrigerator thing
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/Flopper/Small/WID_Athena_FlopperSmall.WID_Athena_FlopperSmall");
	Row1.DropCount = 3;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.25f;
	LootForTiers["Loot_AthenaIceBox"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/Flopper/Effective/WID_Athena_Flopper_Effective.WID_Athena_Flopper_Effective");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.30f;
	LootForTiers["Loot_AthenaIceBox"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/Flopper/WID_Athena_Flopper.WID_Athena_Flopper");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.20f;
	LootForTiers["Loot_AthenaIceBox"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/Bucket/Nice/WID_Athena_Bucket_Nice.WID_Athena_Bucket_Nice");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.001f;
	LootForTiers["Loot_AthenaIceBox"].push_back(Row1);
	//End Refrigerator thing

	//Kingsman
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Mantis/Items/UncleBrolly/WID_UncleBrolly.WID_UncleBrolly");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.05f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
#else
	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_BoltAction_Scope_Athena_R_Ore_T03.WID_Sniper_BoltAction_Scope_Athena_R_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.20f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_BoltAction_Scope_Athena_VR_Ore_T03.WID_Sniper_BoltAction_Scope_Athena_VR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.10f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_BoltAction_Scope_Athena_SR_Ore_T03.WID_Sniper_BoltAction_Scope_Athena_SR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.05f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	/*Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Suppressed_Scope_Athena_R_Ore_T03.WID_Sniper_Suppressed_Scope_Athena_R_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.18f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Suppressed_Scope_Athena_VR_Ore_T03.WID_Sniper_Suppressed_Scope_Athena_VR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.09f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Suppressed_Scope_Athena_SR_Ore_T03.WID_Sniper_Suppressed_Scope_Athena_SR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.04f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);*/

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy");
	Row1.DropCount = /*((UFortAmmoItemDefinition*)Row1.Def)->DropCount*/ 6;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.15f;
	LootForTiers["Loot_AthenaAmmoLarge"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Traps/TID_Floor_Player_Launch_Pad_Athena.TID_Floor_Player_Launch_Pad_Athena");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.06f;
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_VR_Ore_T03.WID_Sniper_Heavy_Athena_VR_Ore_T03");
	Row1.Level = 3;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.07f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_SR_Ore_T03.WID_Sniper_Heavy_Athena_SR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.04f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_SR_Ore_T03.WID_Sniper_Heavy_Athena_SR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.04f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Hook_Gun_VR_Ore_T03.WID_Hook_Gun_VR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.04f;
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	/*Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Crossbow_Athena_R_Ore_T03.WID_Sniper_Crossbow_Athena_R_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.20f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Crossbow_Athena_VR_Ore_T03.WID_Sniper_Crossbow_Athena_VR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.10f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Special_FiendHunter_Athena_VR_Ore_T03.WID_Special_FiendHunter_Athena_VR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.13f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);*/

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_NoScope_Athena_R_Ore_T03.WID_Sniper_NoScope_Athena_R_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.20f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_NoScope_Athena_VR_Ore_T03.WID_Sniper_NoScope_Athena_VR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.10f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Standard_Scope_Athena_VR_Ore_T03.WID_Sniper_Standard_Scope_Athena_VR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.20f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Standard_Scope_Athena_SR_Ore_T03.WID_Sniper_Standard_Scope_Athena_SR_Ore_T03");
	Row1.Level = 4;
	Row1.DropCount = 1;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.10f;
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/Bandage/Athena_Bandage.Athena_Bandage");
	Row1.DropCount = 5;
	Row1.LoadedAmmo = GetAmmoForDef((UFortWeaponItemDefinition*)Row1.Def);
	Row1.Weight = 0.21f;
	//LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);
	LootForTiers["Loot_AthenaLlama"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/AppleSun/WID_Athena_AppleSun.WID_Athena_AppleSun");
	Row1.DropCount = 3;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.10f;
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/ShockwaveGrenade/Athena_ShockGrenade.Athena_ShockGrenade");
	Row1.DropCount = 3;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.15f;
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/Flopper/Small/WID_Athena_FlopperSmall.WID_Athena_FlopperSmall");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.25f;
	LootForTiers["Loot_AthenaIceBox"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/Bucket/Nice/WID_Athena_Bucket_Nice.WID_Athena_Bucket_Nice");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.01f;
	LootForTiers["Loot_AthenaIceBox"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/Bucket/WID_Athena_Bucket_Old.WID_Athena_Bucket_Old");
	Row1.DropCount = 1;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 0.1f;
	LootForTiers["Loot_AthenaIceBox"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);
#endif
#else
	// current test: BB pickup
	#define ITEM_TO_TEST "/Game/Athena/Items/Gameplay/Lotus/Mustache/AGID_Lotus_Mustache.AGID_Lotus_Mustache"
	#define ITEM2_TO_TEST "/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_VR_Ore_T03.WID_Sniper_Heavy_Athena_VR_Ore_T03"

	Row1.Def = StaticLoadObject<UFortItemDefinition>(ITEM_TO_TEST);
	log_debug("Def: %p\n", Row1.Def);
	Row1.DropCount = 1;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 1.0f;
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);

	Row1.Def = StaticLoadObject<UFortItemDefinition>(ITEM2_TO_TEST);
	log_debug("Def: %p\n", Row1.Def);
	Row1.DropCount = 1;
	Row1.LoadedAmmo = 0;
	Row1.Weight = 1.0f;
	LootForTiers["Loot_AthenaFloorLoot"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure"].push_back(Row1);
	LootForTiers["Airdrop"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Blue"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_Red"].push_back(Row1);
	LootForTiers["Loot_AthenaTreasure_White"].push_back(Row1);
	LootForTiers["Loot_AthenaVending"].push_back(Row1);
	LootForTiers["Loot_AthenaFloorLoot_Warmup"].push_back(Row1);
#endif
}

template<typename T>
bool VectorContains(T Item, vector<T>& Vector)
{
	for (auto& Def : Vector)
	{
		if (Def == Item)
			return true;
	}

	return false;
}

ItemRow* GetRandomItem(string LootTier, EFortItemType ItemType = EFortItemType::EFortItemType_MAX, int Level = -1, bool EnableDefs = false)
{
	static vector<UFortItemDefinition*> LastDefs{};

	if (!LootForTiers.contains(LootTier))
		return nullptr;

	auto& Vector = LootForTiers[LootTier];

	if (Vector.size() <= 0)
		return nullptr;
	static auto rng = default_random_engine((unsigned int)time(0));
	shuffle(Vector.begin(), Vector.end(), rng);

	ItemRow* Ret = &Vector[GetMath()->RandomInteger64InRange(0, Vector.size() - 1)];

	if (!Ret->Def) {
		return GetRandomItem(LootTier, ItemType, Level, EnableDefs);
	}

	if (EnableDefs)
	{
		bool contains = VectorContains<UFortItemDefinition*>(Ret->Def, LastDefs);

		if (contains)
			return GetRandomItem(LootTier, ItemType, Level, EnableDefs);
	}
#ifndef TESTING_LOOTPOOL
	if (ItemType == EFortItemType::WeaponRanged && (Ret->Def->GetName().starts_with("Athena_") || Ret->Def->GetName() == "WID_Athena_PurpleMouse" || Ret->Def->GetName() == "WID_Hook_Gun_VR_Ore_T03")) // consumables are not guns!
	{
		return GetRandomItem(LootTier, ItemType, Level, EnableDefs);
	}
	else if (ItemType == EFortItemType::Consumable && (Ret->Def->GetName().starts_with("Athena_") || Ret->Def->GetName() == "WID_Athena_PurpleMouse" || Ret->Def->GetName() == "WID_Hook_Gun_VR_Ore_T03")) // consumables are not guns!
	{
		//log_info("Chose item: %s\n", Ret->Def->GetFullName().c_str());

		return Ret;
	}

	if (ItemType != EFortItemType::EFortItemType_MAX && Ret->Def->GetItemType() != ItemType)
		return GetRandomItem(LootTier, ItemType, Level, EnableDefs);
	if (!GetMath()->RandomBoolWithWeight(Ret->Weight))
		return GetRandomItem(LootTier, ItemType, Level, EnableDefs);
	if (Level != -1 && Ret->Level != -1 && Ret->Level != Level)
		return GetRandomItem(LootTier, ItemType, Level, EnableDefs);

#endif

	if (EnableDefs)
	{
		LastDefs.push_back(Ret->Def);

		if (LastDefs.size() > 3)
			LastDefs.erase(LastDefs.begin());
	}

	log_info("Chose item: %s\n", Ret->Def->GetFullName().c_str());

	return Ret;
}

char __fastcall SpawnLoot(ABuildingContainer* Object)
{
	if (!bEnableLooting)
		return 0;

	log_debug("Spawning loot for %s, TG: %s\n", Object->GetName().c_str(), Object->SearchLootTierGroup.ToString().c_str());

	string TierGroupStr = Object->SearchLootTierGroup.ToString();

	if (!LootForTiers.contains(TierGroupStr))
	{
		log_warn("Invalid TierGroup %s\n", TierGroupStr.c_str());
		return 0;
	}

	log_debug("Size: %zd\n", LootForTiers[TierGroupStr].size());

	EFortPickupSpawnSource Source = EFortPickupSpawnSource::Unset;
	EFortPickupSourceTypeFlag Flag = EFortPickupSourceTypeFlag::Container;
	bool IsChest = false;
	bool IsFactionChest = false;

	if (TierGroupStr == "Loot_AthenaTreasure")
	{
		Source = EFortPickupSpawnSource::Chest;
		IsChest = true;
	}
	else if (TierGroupStr == "Loot_AthenaAmmoLarge")
	{
		Source = EFortPickupSpawnSource::AmmoBox;
	}
	else if (TierGroupStr.contains("FloorLoot"))
	{
		Flag = EFortPickupSourceTypeFlag::FloorLoot;
	}

	IsFactionChest = TierGroupStr == "Loot_AthenaTreasure_Blue" || TierGroupStr == "Loot_AthenaTreasure_Red" || TierGroupStr == "Loot_AthenaTreasure_White";

	if (TierGroupStr == "Loot_AthenaIceBox")
	{
		Object->LootSpawnLocation_Athena.Y += 45;
	}

	FVector Loc = Object->K2_GetActorLocation() + (Object->GetActorForwardVector() * Object->LootSpawnLocation_Athena.X) + (Object->GetActorRightVector() * Object->LootSpawnLocation_Athena.Y) + (Object->GetActorUpVector() * Object->LootSpawnLocation_Athena.Z);

	ItemRow* Item = GetRandomItem(TierGroupStr, ((IsChest || IsFactionChest) ? EFortItemType::WeaponRanged : EFortItemType::EFortItemType_MAX));
	while (!Item) {
		Item = GetRandomItem(TierGroupStr, ((IsChest || IsFactionChest) ? EFortItemType::WeaponRanged : EFortItemType::EFortItemType_MAX));
	}
	if (!Item || !Item->Def)
		return 0;

	SpawnPickup(Loc, Item->Def, Item->DropCount, Item->LoadedAmmo, Flag, Source);


	if (TierGroupStr == "Loot_AthenaAmmoLarge")
	{
		ItemRow* Item = GetRandomItem(TierGroupStr, EFortItemType::EFortItemType_MAX);
		while (!Item) {
			Item = GetRandomItem(TierGroupStr, EFortItemType::EFortItemType_MAX);
		}
		if (!Item || !Item->Def)
			return 0;

		SpawnPickup(Loc, Item->Def, Item->DropCount, Item->LoadedAmmo, Flag, Source);
	}

	if (TierGroupStr == "Loot_AthenaIceBox")
	{
		//for (size_t i = 0; i < 2; i++)
		//{
		// we only want 2 items
		ItemRow* Item = GetRandomItem(TierGroupStr, EFortItemType::EFortItemType_MAX);
		while (!Item) {
			Item = GetRandomItem(TierGroupStr, EFortItemType::EFortItemType_MAX);
		}
		if (!Item || !Item->Def)
			return 0;
		SpawnPickup(Loc, Item->Def, Item->DropCount, Item->LoadedAmmo, Flag, Source);
		//}
	}

	UFortAmmoItemDefinition* AmmoDef = (UFortAmmoItemDefinition*)((UFortWeaponRangedItemDefinition*)Item->Def)->GetAmmoWorldItemDefinition_BP();
	if (AmmoDef && Item->Def != AmmoDef && AmmoDef->DropCount > 0 && AmmoDef->GetName() != "AmmoInfiniteCrossbow_Athena")
	{
		SpawnPickup(Loc, AmmoDef, AmmoDef->DropCount == 4 ? 6 : (AmmoDef->DropCount == 10 ? 2 : AmmoDef->DropCount), 0, Flag, Source);
	}

	if (IsChest || IsFactionChest)
	{
		ItemRow* ConsItem = GetRandomItem(TierGroupStr, EFortItemType::Consumable);
		while (!ConsItem) {
			ConsItem = GetRandomItem(TierGroupStr, EFortItemType::Consumable);
		}
		if (!ConsItem)
			return 0;
		SpawnPickup(Loc, ConsItem->Def, ConsItem->DropCount, ConsItem->LoadedAmmo, Flag, Source);

		static auto Wood = StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
		static auto Metal = StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/MetalItemData.MetalItemData");
		static auto Stone = StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
		UFortItemDefinition* Def = nullptr;
		if (rand() % 40 > 20)
		{
			Def = rand() % 20 > 10 ? Wood : Stone;
		}
		else
		{
			Def = Metal;
		}
		SpawnPickup(Loc, Def, 30, 0, Flag, Source);

	}

	if (IsFactionChest)
	{
		/*for (size_t i = 0; i < 2 /*3* /; i++)
		{*/
			ItemRow* Item = GetRandomItem(TierGroupStr, EFortItemType::WeaponRanged);
			while (!Item) {
				Item = GetRandomItem(TierGroupStr, EFortItemType::WeaponRanged);
			}
			if (!Item || !Item->Def)
				return 0;

			int Ammo2 = 0;
			if (Item->Def->GetName().starts_with("WID_Assault_LMG_Athena_")) {
				Ammo2 = 45;
			}

			SpawnPickup(Loc, Item->Def, Item->DropCount, Item->LoadedAmmo, Flag, Source);
			UFortAmmoItemDefinition* AmmoDef = (UFortAmmoItemDefinition*)((UFortWeaponRangedItemDefinition*)Item->Def)->GetAmmoWorldItemDefinition_BP();
			if (AmmoDef && Item->Def != AmmoDef && (AmmoDef->DropCount > 0 || Ammo2 > 0) && AmmoDef->GetName() != "AmmoInfiniteCrossbow_Athena")
			{
				SpawnPickup(Loc, AmmoDef, Ammo2 == 0 ? AmmoDef->DropCount == 4 ? 6 : (AmmoDef->DropCount == 10 ? 2 : AmmoDef->DropCount) : Ammo2, 0, Flag, Source);
			}
		//}
	}

	Object->bAlreadySearched = true;
	Object->SearchBounceData.SearchAnimationCount++;
	Object->BounceContainer();
	Object->OnRep_bAlreadySearched();

	return 1;
}

void SpawnFloorLoot()
{
	TArray<AActor*> FloorLootSpawners;
	UClass* SpawnerClass = StaticLoadObject<UClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_Warmup.Tiered_Athena_FloorLoot_Warmup_C");
	GetStatics()->GetAllActorsOfClass(GetWorld(), SpawnerClass, &FloorLootSpawners);

	for (int32 /*size_t*/ i = 0; i < FloorLootSpawners.Num(); i++)
	{
		FloorLootSpawners[i]->K2_DestroyActor();
	}

	FloorLootSpawners.FreeArray();

	SpawnerClass = StaticLoadObject<UClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_01.Tiered_Athena_FloorLoot_01_C");
	GetStatics()->GetAllActorsOfClass(GetWorld(), SpawnerClass, &FloorLootSpawners);

	for (int32 /*size_t*/ i = 0; i < FloorLootSpawners.Num(); i++)
	{
		FloorLootSpawners[i]->K2_DestroyActor();
	}

	FloorLootSpawners.FreeArray();
}