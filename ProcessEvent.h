#pragma once
#include "Engine.h"

void (*ProcessEventOG)(void*,void*,void*);
void ProcessEventHook(UObject* Obj, UFunction* Func, void* Params)
{
	if (Func->GetName() == "ServerSpawnDeco") {
		ProcessEventOG(Obj, Func, Params);
		auto TT = (AFortTrapTool*)Obj;

		auto Def = TT->ItemDefinition;
		auto Pawn = (AFortPlayerPawn*)TT->GetOwner();
		auto PC = (AFortPlayerController*)Pawn->GetOwner();

		if (!PC->bInfiniteAmmo) {
			Remove(PC, Def);
		}
		log_debug("UF: 0x%llx\n", __int64(Func));
		return;
	}
	else if (Func->GetName() == "ServerCreateBuildingAndSpawnDeco") {
		auto TT = (AFortTrapTool*)Obj;

		auto Def = TT->ItemDefinition;
		auto Pawn = (AFortPlayerPawn*)TT->GetOwner();
		auto PC = (AFortPlayerControllerAthena*)Pawn->GetOwner();

		//UObject* BuildingClass = nullptr;
		FVector TrapLocation;
		FRotator TrapRotation{};
		FVector BuildLocation;
		FRotator BuildRotation{};
		ABuildingSMActor* NewBuilding = nullptr;

		//if (BuildingRotationOffset != 0) // skunked
		{
			// const struct FVector_NetQuantize10& BuildingLocation, const struct FRotator& BuildingRotation, const struct FVector_NetQuantize10& Location, const struct FRotator& Rotation, TEnumAsByte<EBuildingAttachmentType> InBuildingAttachmentType
			struct parms { FVector BuildingLocation; FRotator BuildingRotation; FVector Location; FRotator Rotation; };

			parms *Param = (parms*)Params;

			TrapLocation = Param->Location;
			TrapRotation = Param->Rotation;
			BuildLocation = Param->BuildingLocation;
			BuildRotation = Param->BuildingRotation;
		}

		struct ServerSpawnDeco_Params { FVector Location; FRotator Rotation; ABuildingSMActor* AttachedActor; };

		// Spawn the building
		//AFortBroadcastRemoteClientInfo* Info = *(AFortBroadcastRemoteClientInfo**)(__int64(PC) + 0x3030);
		//UClass* BuildingClass = Info->RemoteBuildableClass.Get();
		// this is a bad idea in reality, but since the only trap in the game is a launch pad, and that needs a floor, this works fine.
		UClass* BuildingClass = APBWA_S1_Floor_C::StaticClass();
		char a7;
		TArray<AActor*> BuildingsToRemove;
		if (!CantBuild(GetWorld(), BuildingClass, BuildLocation, BuildRotation, false, &BuildingsToRemove, &a7))
		{
			if (!PC->bBuildFree) {
				auto ResDef = GetFortKismet()->K2_GetResourceItemDefinition(((ABuildingSMActor*)BuildingClass->DefaultObject)->ResourceType);
				Remove(PC, ResDef, 10);
			}

			NewBuilding = SpawnActor<ABuildingSMActor>(BuildingClass, BuildLocation, BuildRotation, PC);
			uint8 Team = *(uint8*)(__int64(PC->PlayerState) + 0xE60);

			NewBuilding->bPlayerPlaced = true;
			NewBuilding->InitializeKismetSpawnedBuildingActor(NewBuilding, PC, true);
			NewBuilding->TeamIndex = Team;
			NewBuilding->Team = EFortTeam(Team);

			for (int32 /*size_t*/ i = 0; i < BuildingsToRemove.Num(); i++)
			{
				BuildingsToRemove[i]->K2_DestroyActor();
			}
			BuildingsToRemove.FreeArray();
		}



		ServerSpawnDeco_Params ServerSpawnDeco_params = { TrapLocation, TrapRotation, NewBuilding };

		//auto SSD = StaticLoadObject<UFunction>("/Script/FortniteGame.FortDecoTool.ServerSpawnDeco");
		auto SSD = AFortDecoTool::StaticClass()->DefaultObject->Class->GetFunction("FortDecoTool", "ServerSpawnDeco");
		ProcessEvent(Obj, SSD, &ServerSpawnDeco_params); // Spawn the trap
	}
	else if (Func->GetName() == "OnAlertLevelChanged") {
		auto PC = (ABP_PhoebePlayerController_C*)Obj;
		struct parms { EAlertLevel Old; EAlertLevel New; };

		parms* Param = (parms*)Params;

		OnAlertLevelChanged(PC, Param->Old, Param->New);
	}
	if (!Obj->GetName().contains("Vehicle") && (Func->GetName().contains("Overlap") || Func->GetName().contains("CreateBuilding") || Func->GetName().contains("ServerSpawn") || Func->GetName().contains("OnAlertLevel"))/* || Func->GetName().contains("Hit")*/)
	{
		log_debug("FuncName: %s\n", Func->GetName().c_str());
		log_debug("ObjectName: %s\n", Obj->GetName().c_str());
		log_debug("Func: 0x%llx\n", __int64(*(void**)(__int64(Func) + 0xF0)) - __int64(GetModuleHandleA(0)));
		log_debug("DefaultObject: 0x%llx\n", __int64((*(void***)Obj->Class->DefaultObject)) - __int64(GetModuleHandleA(0)));
	}

	return ProcessEventOG(Obj, Func, Params);
}