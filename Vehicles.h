#pragma once
#include "Engine.h"
#include "PlayerController.h"

void SpawnVehicles()
{
	if (bLateGame || bArsenal)
		return;

	TArray<AActor*> Spawners;
	GetStatics()->GetAllActorsOfClass(GetWorld(), AFortAthenaVehicleSpawner::StaticClass(), &Spawners);
	static bool spawnedSkyeHeli = false;

	for (int32 /*size_t*/ i = 0; i < Spawners.Num(); i++)
	{

		AFortAthenaVehicleSpawner* Spawner = (AFortAthenaVehicleSpawner*)Spawners[i];
		log_debug("Vehicle: %s\n", Spawner->GetName().c_str());
		auto Name = Spawner->GetName();
		if (Name.starts_with("Athena_Meatball_L_Spawner") || Name.starts_with("Athena_Infiltration_Meatball_L_Spawner") || Name.starts_with("Apollo_Hoagie_Spawner")) {
			log_info("Spawned vehicle: %s\n", Spawner->GetName().c_str());
			SpawnActor<AFortAthenaVehicle>(Spawner->GetVehicleClass(), Spawner->K2_GetActorLocation());
			if (!spawnedSkyeHeli && Name.starts_with("Apollo_Hoagie_Spawner")) {
				SpawnActor<AFortAthenaVehicle>(Spawner->GetVehicleClass(), {
					113665,
					-91120,
					-2985
				});
				spawnedSkyeHeli = true;
			}
		}
		else {
			log_info("Not spawning %s\n", Spawner->GetName().c_str());
		}
	}

	Spawners.FreeArray();
}

void (*ServerMoveOG)(AFortPhysicsPawn* Pawn, FReplicatedPhysicsPawnState InState);
void ServerMove(AFortPhysicsPawn* Pawn, FReplicatedPhysicsPawnState InState)
{
	UPrimitiveComponent* Mesh = (UPrimitiveComponent*)Pawn->RootComponent;

	InState.Rotation.X -= 2.5f;
	InState.Rotation.Y /= 0.3f;
	InState.Rotation.Z -= -2.0f;
	InState.Rotation.W /= -1.2f;

	FTransform Transform{};
	Transform.Translation = InState.Translation;
	Transform.Rotation = InState.Rotation;
	Transform.Scale3D = FVector{ 1, 1, 1 };

	Mesh->K2_SetWorldTransform(Transform, false, nullptr, true);
	Mesh->bComponentToWorldUpdated = true;
	Mesh->SetPhysicsLinearVelocity(InState.LinearVelocity, 0, FName());
	Mesh->SetPhysicsAngularVelocity(InState.AngularVelocity, 0, FName());
}


bool RemoveItem(AFortInventory *inv, const FGuid& ItemGuid, bool* bShouldUpdate, int Count, bool bForceRemoval = false, bool bIgnoreVariables = false)
{
	if (bShouldUpdate)
		*bShouldUpdate = false;

	auto ItemInstance = FindItemInstance(inv, ItemGuid);
	auto ReplicatedEntry = FindReplicatedEntry(inv, ItemGuid);

	if (!ItemInstance || !ReplicatedEntry)
		return false;

	auto ItemDefinition = Cast<UFortWorldItemDefinition>(ReplicatedEntry->ItemDefinition);

	if (!ItemDefinition)
		return false;

	int OldCount = Count;

	if (Count < 0) // idk why i have this
	{
		Count = 0;
		bForceRemoval = true;
	}

	auto& ItemInstances = inv->Inventory.ItemInstances;
	static auto ReplicatedEntriesOffset = FindOffsetStruct("/Script/FortniteGame.FortItemList", "ReplicatedEntries");
	TArray<FFortItemEntry> ReplicatedEntries = *(TArray<FFortItemEntry>*)(__int64(&inv->Inventory) + ReplicatedEntriesOffset);

	auto NewCount = ReplicatedEntry->Count - Count;

	bool bOverrideChangeStackSize = false;

	if (!bIgnoreVariables && ItemDefinition->bPersistInInventoryWhenFinalStackEmpty)
	{
		bool bIsFinalStack = true;

		for (int i = 0; i < ItemInstances.Num(); i++)
		{
			auto ItemInstance = ItemInstances.Data[i];

			if (ItemInstance->ItemEntry.ItemDefinition == ItemDefinition && ItemInstance->ItemEntry.ItemGuid != ItemGuid)
			{
				bIsFinalStack = false;
				break;
			}
		}

		if (bIsFinalStack)
		{
			NewCount = NewCount < 0 ? 0 : NewCount; // min(NewCount, 0) or something i forgot
			bOverrideChangeStackSize = true;
		}
	}

	if (OldCount != -1 && (NewCount > 0 || bOverrideChangeStackSize))
	{
		ItemInstance->ItemEntry.Count = NewCount;
		ReplicatedEntry->Count = NewCount;

		inv->Inventory.MarkItemDirty(ItemInstance->ItemEntry);
		inv->Inventory.MarkItemDirty(*ReplicatedEntry);

		return true;
	}

	if (NewCount < 0) // Hm
		return false;

	auto FortPlayerController = Cast<AFortPlayerController>(inv->GetOwner());

	bool bWasGadget = false;

	for (int i = 0; i < ItemInstances.Num(); i++)
	{
		if (ItemInstances[i]->ItemEntry.ItemGuid == ItemGuid)
		{
			static auto GenericAttributeValuesOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntry", "GenericAttributeValues", false);

			if (GenericAttributeValuesOffset != -1)
			{
				ItemInstances[i]->ItemEntry.GenericAttributeValues.FreeArray();
			}

			ItemInstances[i]->ItemEntry.StateValues.FreeArray();
			ItemInstances.RemoveSingle(i);
			break;
		}
	}

	for (int i = 0; i < ReplicatedEntries.Num(); i++)
	{
		if (ReplicatedEntries[i].ItemGuid == ItemGuid)
		{
			static auto GenericAttributeValuesOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntry", "GenericAttributeValues", false);

			if (GenericAttributeValuesOffset != -1)
			{
				ReplicatedEntries[i].GenericAttributeValues.FreeArray();
			}

			ReplicatedEntries[i].StateValues.FreeArray();
			ReplicatedEntries.RemoveSingle(i);
			break;
		}
	}

	// todo remove from weaponlist

	if (bShouldUpdate)
		*bShouldUpdate = true;

	return true;
}

UFortWeaponItemDefinition* GetVehicleWeaponForSeat(AFortAthenaVehicle *vehicle, int SeatIdx)
{
	//static auto GetSeatWeaponComponentFn = StaticFindObject<UFunction>("/Script/FortniteGame.FortAthenaVehicle.GetSeatWeaponComponent");

	UFortWeaponItemDefinition* VehicleWeaponDefinition = nullptr;

	log_debug("SeatIndex: %d\n", SeatIdx);

	UFortVehicleSeatWeaponComponent* WeaponComponent = nullptr;

	/*if (GetSeatWeaponComponentFn)
	{
		struct { int SeatIndex; UObject* ReturnValue; } AFortAthenaVehicle_GetSeatWeaponComponent_Params{};

		vehicle->ProcessEvent(GetSeatWeaponComponentFn, &AFortAthenaVehicle_GetSeatWeaponComponent_Params);

		WeaponComponent = AFortAthenaVehicle_GetSeatWeaponComponent_Params.ReturnValue;*/
		WeaponComponent = vehicle->GetSeatWeaponComponent(0);

		if (!WeaponComponent) {
			log_warn("NO WEAPON COMPONENT!\n");
			return VehicleWeaponDefinition;
		}

		auto& WeaponSeatDefinitions = WeaponComponent->WeaponSeatDefinitions;

		for (int i = 0; i < WeaponSeatDefinitions.Num(); i++)
		{
			auto WeaponSeat = WeaponSeatDefinitions[i];

			if (WeaponSeat.SeatIndex != SeatIdx) {
				log_debug("%d is not %d\n", WeaponSeat.SeatIndex, SeatIdx);
				continue;
			}


			log_debug("found it! %d %d\n", WeaponSeat.SeatIndex, SeatIdx);
			VehicleWeaponDefinition = WeaponSeat.VehicleWeapon;

			break;
		}
	//}
	/*else
	{
		static auto FerretWeaponItemDefinition = StaticFindObject<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/Ferret_Weapon.Ferret_Weapon");
		static auto OctopusWeaponItemDefinition = StaticFindObject<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/Vehicles/WID_Octopus_Weapon.WID_Octopus_Weapon");
		static auto InCannonWeaponItemDefinition = StaticFindObject<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/Vehicles/ShipCannon_Weapon_InCannon.ShipCannon_Weapon_InCannon");
		static auto CannonWeaponItemDefinition = StaticFindObject<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/Vehicles/ShipCannon_Weapon.ShipCannon_Weapon");
		static auto TurretWeaponItemDefinition = StaticFindObject<UFortWeaponItemDefinition>("/Game/Athena/Items/Traps/MountedTurret/MountedTurret_Weapon.MountedTurret_Weapon");

		auto ReceivingActorName = vehicle->GetName();

		if (SeatIdx == 0)
		{
			if (ReceivingActorName.contains("Ferret")) // plane
			{
				VehicleWeaponDefinition = FerretWeaponItemDefinition;
			}
		}

		if (ReceivingActorName.contains("Octopus")) // baller
		{
			VehicleWeaponDefinition = OctopusWeaponItemDefinition;
		}

		else if (ReceivingActorName.contains("Cannon")) // cannon
		{
			VehicleWeaponDefinition = SeatIdx == 1 ? InCannonWeaponItemDefinition : CannonWeaponItemDefinition;
		}

		else if (ReceivingActorName.contains("MountedTurret"))
		{
			VehicleWeaponDefinition = TurretWeaponItemDefinition;
		}
	}*/

	return VehicleWeaponDefinition;
}

void (*ServerRequestSeatChangeOG)(AFortPlayerControllerAthena* PC, int TargetSeatIndex);
void ServerRequestSeatChange(AFortPlayerControllerAthena* PC, int TargetSeatIndex) {
	log_debug("Called!\n");
	/*auto Pawn = PC->MyFortPawn;
	
	if (!Pawn)
		return ServerRequestSeatChangeOG(PC, TargetSeatIndex);

	auto Vehicle = Pawn->GetVehicle();

	if (!Vehicle)
		return ServerRequestSeatChangeOG(PC, TargetSeatIndex);

	int lastSeats[] = {
		3,
		0,
		1,
		2
	};

	auto SeatIndex2 = Vehicle->FindSeatIndex(Pawn);
	auto SeatIndex = lastSeats[TargetSeatIndex];
	log_debug("%d %d\n", SeatIndex2, SeatIndex);
	auto OldVehicleWeaponDefinition = GetVehicleWeaponForSeat(Vehicle, SeatIndex);
	
	log_debug("OldVehicleWeaponDefinition: %s\n", OldVehicleWeaponDefinition ? OldVehicleWeaponDefinition->GetFullName().c_str() : "BadRead");

	if (!OldVehicleWeaponDefinition)
		return ServerRequestSeatChangeOG(PC, TargetSeatIndex);

	auto WorldInventory = PC->WorldInventory;

	if (!WorldInventory)
		return ServerRequestSeatChangeOG(PC, TargetSeatIndex);

	auto OldVehicleWeaponInstance = FindItemInstance(WorldInventory, OldVehicleWeaponDefinition);

	if (OldVehicleWeaponInstance)
	{
		bool bShouldUpdate = false;
		RemoveItem(WorldInventory, OldVehicleWeaponInstance->ItemEntry.ItemGuid, &bShouldUpdate, OldVehicleWeaponInstance->ItemEntry.Count, true);
		log_debug("removed\n");

		if (bShouldUpdate) {
			WorldInventory->HandleInventoryLocalUpdate();

			WorldInventory->Inventory.MarkArrayDirty();
			log_debug("updated\n");
		}
	}

	UFortWeaponItemDefinition* RequestingVehicleWeaponDefinition = GetVehicleWeaponForSeat(Vehicle, TargetSeatIndex);
	if (!RequestingVehicleWeaponDefinition)
	{
		auto PickaxeInstance = GetPickaxeInstance(WorldInventory);
		log_debug("pickaxe\n");


		if (!PickaxeInstance)
			return ServerRequestSeatChangeOG(PC, TargetSeatIndex);

		//ServerExecuteInventoryItem(PC, PickaxeInstance->ItemEntry.ItemGuid); // Bad, we should equip the last weapon.
		Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition *) PickaxeInstance->ItemEntry.ItemDefinition, PickaxeInstance->ItemEntry.ItemGuid);
		log_debug("Equipped!\n");
		return ServerRequestSeatChangeOG(PC, TargetSeatIndex);
	}

	auto NewAndModifiedInstances = InvAddItem(WorldInventory, RequestingVehicleWeaponDefinition, nullptr);
	UFortWorldItem *RequestedVehicleInstance = (UFortWorldItem *) NewAndModifiedInstances.first[0];

	if (!RequestedVehicleInstance)
		return ServerRequestSeatChangeOG(PC, TargetSeatIndex);

	WorldInventory->HandleInventoryLocalUpdate();

	WorldInventory->Inventory.MarkArrayDirty();

	log_debug("equip htis idiot\n");
	auto RequestedVehicleWeapon = Pawn->EquipWeaponDefinition(RequestingVehicleWeaponDefinition, RequestedVehicleInstance->ItemEntry.ItemGuid);
	log_debug("ploosh is now mad\n");*/

	return ServerRequestSeatChangeOG(PC, TargetSeatIndex);
};