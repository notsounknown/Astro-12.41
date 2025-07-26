#pragma once
#include "Abilities.h"
#include "Inventory.h"
#include "Looting.h"
#include "misc.h"

void GiveModifier(UFortGameplayModifierItemDefinition* Modifier, AFortPlayerControllerAthena* PC)
{
	static auto AbilityOffset = GetOffset(Modifier, "PersistentAbilitySets");
	static auto EffectsOffset = GetOffset(Modifier, "PersistentGameplayEffects");

	TArray<FFortAbilitySetDeliveryInfo>& Abilities = *(TArray<FFortAbilitySetDeliveryInfo>*)(__int64(Modifier) + AbilityOffset);
	TArray<FFortGameplayEffectDeliveryInfo>& Effects = *(TArray<FFortGameplayEffectDeliveryInfo>*)(__int64(Modifier) + EffectsOffset);

	for (int32 /*size_t*/ i = 0; i < Abilities.Num(); i++)
	{
		auto& idk = Abilities[i];
		if (idk.DeliveryRequirements.bApplyToPlayerPawns)
		{
			for (int32 /*size_t*/ j = 0; j < idk.AbilitySets.Num(); j++)
			{
				UFortAbilitySet* Set = StaticLoadObject<UFortAbilitySet>(Conv_NameToString(idk.AbilitySets[j].ObjectID.AssetPathName).ToString());
				log_debug("Set: %s\n", Set->GetName().c_str());
				GiveAbilitySet(PC, Set);
			}
		}
	}

	for (int32 /*size_t*/ i = 0; i < Effects.Num(); i++)
	{
		auto& idk = Effects[i];
		if (idk.DeliveryRequirements.bApplyToPlayerPawns)
		{
			for (int32 /*size_t*/ j = 0; j < idk.GameplayEffects.Num(); j++)
			{
				UClass* Effect = StaticLoadObject<UClass>(Conv_NameToString(idk.GameplayEffects[j].GameplayEffect.ObjectID.AssetPathName).ToString());
				log_debug("Effect: %s\n", Effect->GetName().c_str());
				PC->MyFortPawn->AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(Effect, idk.GameplayEffects[j].Level, FGameplayEffectContextHandle());
			}
		}
	}
}

void (*ServerReadyToStartMatchOG)(AFortPlayerControllerAthena* PC);
void ServerReadyToStartMatch(AFortPlayerControllerAthena* PC)
{
	if (PC->IsA(AFortLiveBroadcastController::StaticClass()))
		return ServerReadyToStartMatchOG(PC); // Ploosh: bad? either this is spectators or replay, idk.


	/*string AccountID = GetAccountID(PC).ToString();
	bool Registered = PlooshFNAPI::CheckUser(AccountID);
	bool Reboot = PlooshFNAPI::CheckReboot(AccountID);

	if (Reboot) {
		auto Name = PC->PlayerState->GetPlayerName().ToString();

		log_info("User %s (%s) is a reboot account!\n", Name.c_str(), AccountID.c_str());
#ifdef VPS
		PC->ClientReturnToMainMenu(L"You are not allowed to use reboot accounts on Astro. Please go to https://discord.gg/rpFMF3aHjG to register.");
#endif
	}


	if (!Registered) {
		auto Name = PC->PlayerState->GetPlayerName().ToString();

		log_info("User %s (%s) is not registered!\n", Name.c_str(), AccountID.c_str());
		#ifdef VPS
		PC->ClientReturnToMainMenu(L"Your account is not registered with Project Astro. Please go to https://discord.gg/rpFMF3aHjG to register.");
		#endif
	}

	if (PlooshFNAPI::IsBanned(AccountID))
	{
		//GameSession_KickPlayer(GetGameMode()->GameSession, PC);
		#ifndef VPS
		PC->ClientReturnToMainMenu(L"You are banned from PlooshFN. Please go to https://discord.gg/StWVWn6a3B to appeal.");
		#else
		PC->ClientReturnToMainMenu(L"You are banned from Project Astro. Please go to https://discord.gg/rpFMF3aHjG to appeal.");
		#endif
		return;
	}*/

	// epic
	if (bStaffGameOnly) {
		auto Name = PC->PlayerState->GetPlayerName().ToString(); // i hope this works

		if (!VectorContains<string>(Name, AdminNames))
			PC->ClientReturnToMainMenu(L"This server has whitelists turned on. Please contact Ploosh to become whitelisted.");

		/*auto IP = PC->PlayerState->SavedNetworkAddress.ToString();
		if (!VectorContains<string>(IP, AdminIps))
			PC->ClientReturnToMainMenu(L"This server has whitelists turned on. Please contact Ploosh to become whitelisted.");*/
	}


	string name = PC->PlayerState->GetPlayerName().ToString();

	if (GetGameState()->GamePhase >= EAthenaGamePhase::Aircraft)
	{
		//GameSession_KickPlayer(Cast<AFortGameSession>(GetGameMode()->GameSession), PC);
		#ifndef PLAYGROUND
		if (!VectorContains<string>(PC->PlayerState->GetPlayerName().ToString(), AdminNames)) PC->ClientReturnToMainMenu(L"This server has already started!");
		#endif
		return;
	}

	ReviveCounts[PC] = 0;

	if (bDropZone)
	{
		AFortAthenaMutator_SpyRumble* Mutator = (AFortAthenaMutator_SpyRumble*)GetGameState()->GetMutatorByClass(GetGameMode(), AFortAthenaMutator_SpyRumble::StaticClass());
		AFortAthenaMutator_PerkSystemMutator* Mutator2 = (AFortAthenaMutator_PerkSystemMutator*)GetGameState()->GetMutatorByClass(GetGameMode(), AFortAthenaMutator_PerkSystemMutator::StaticClass());

		log_debug("Mutator 1: %p\n", Mutator);
		log_debug("Mutator 2: %p\n", Mutator2);

		UFortControllerComponent_PerkSystem* Comp = (UFortControllerComponent_PerkSystem*)PC->GetComponentByClass(UFortControllerComponent_PerkSystem::StaticClass());

		Comp->MutatorData = Mutator2->MutatorData;
		Comp->CachedFactionTag.TagName = Conv_StringToName(TEXT("Athena.Faction.Ego"));
		log_info("PerkSelection num %d\n", Comp->PerkSelection.Num());
		Comp->RerollCount = 1;

		auto First = StaticLoadObject<UFortSpyTechItemDefinition>("/Game/Athena/Items/SpyTech/ItemDef/STID_PumpShotgun.STID_PumpShotgun");
		auto Second = StaticLoadObject<UFortSpyTechItemDefinition>("/Game/Athena/Items/SpyTech/ItemDef/STID_JetPack.STID_JetPack");

		Comp->SpyTechArray.Add(Comp->DefaultPerks[0]);
		Comp->SpyTechArray.Add(First);
		Comp->SpyTechArray.Add(Second);

		for (int32 /*size_t*/ i = 0; i < Comp->PerkSelection.Num(); i++)
		{
			Comp->PerkSelection[i].Items.FreeArray();
			Comp->PerkSelection[i].Items.Add(Comp->SpyTechArray[i]);
			Comp->PerkSelection[i].Items.Add(Comp->SpyTechArray[i]);
		}

		Comp->RoundStartCache.bDataReady = true;
		log_debug("FuturePerks %d\n", Comp->RoundStartCache.FuturePerks.Num());
		log_debug("SpyTechArray %d\n", Comp->SpyTechArray.Num());
		log_debug("AllSelectedPerks %d\n", Comp->AllSelectedPerks.Num());
		log_debug("DefaultPerks %d\n", Comp->DefaultPerks.Num());

		Comp->OnRep_PerkSelection();
		Comp->OnRep_SpyTechArray();
		Comp->OnRep_RerollCount();
	}

	AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;

	//if (!bNoTeams) {
	*(uint8*)(__int64(PlayerState) + 0xFDC) = *(uint8*)(__int64(PlayerState) + 0xE60) - FirstTeam;
	PlayerState->OnRep_SquadId();
	PlayerState->OnRep_TeamIndex(0);
	PlayerState->OnRep_PlayerTeam();
	PlayerState->OnRep_PlayerTeamPrivate();

	FGameMemberInfo Member{ -1,-1,-1 };
	Member.TeamIndex = *(uint8*)(__int64(PlayerState) + 0xE60);
	Member.SquadId = *(uint8*)(__int64(PlayerState) + 0xFDC);
	Member.MemberUniqueId = *(FUniqueNetIdRepl*)(__int64(PlayerState) + 0x248);

	GetGameState()->GameMemberInfoArray.Members.Add(Member);
	GetGameState()->GameMemberInfoArray.MarkItemDirty(Member);
	//}

	/*if (PlayerBots.size() > 0) {
		PlayerBots[PlayerBots.size() - 1]->Pawn->K2_DestroyActor();
		PlayerBots[PlayerBots.size() - 1]->PC->K2_DestroyActor();
		delete PlayerBots[PlayerBots.size() - 1];
		PlayerBots.pop_back();
	}*/

	#ifdef PLAYGROUND
	PC->bInfiniteAmmo = true;
	PC->bBuildFree = true;
	#endif

	return ServerReadyToStartMatchOG(PC);
}

static vector<APlayerState*> PlayersThatJoined{};
void (*ServerLoadingScreenDroppedOG)(AFortPlayerControllerAthena* PC);
void ServerLoadingScreenDropped(AFortPlayerControllerAthena* PC)
{
	if (PC->IsA(AFortLiveBroadcastController::StaticClass()))
		return;

	for (auto PlayerState : PlayersThatJoined)
	{
		if (PlayerState == PC->PlayerState)
		{
			//return ServerLoadingScreenDroppedOG(PC);
		}
	}
	PlayersThatJoined.push_back(PC->PlayerState);

	if (GetGameState()->GamePhase >= EAthenaGamePhase::Aircraft && !VectorContains<string>(PC->PlayerState->GetPlayerName().ToString(), AdminNames))
	{
		#ifndef PLAYGROUND
		GameSession_KickPlayer(Cast<AFortGameSession>(GetGameMode()->GameSession), PC);
		#endif
		return;
	}

	log_info("%s just joined!\n", PC->PlayerState->GetPlayerName().ToString().c_str());

	for (int32 /*size_t*/ i = 0; i < GetGameMode()->StartingItems.Num(); i++)
	{
		if (GetGameMode()->StartingItems[i].Count <= 0)
			continue;
		GiveItem(PC, GetGameMode()->StartingItems[i].Item, GetGameMode()->StartingItems[i].Count);
	}

	if (bArsenal)
	{
		AFortAthenaMutator_GG* Mutator = (AFortAthenaMutator_GG*)GetGameState()->GetMutatorByClass(GetGameMode(), AFortAthenaMutator_GG::StaticClass());

		GiveItem(PC, Mutator->WeaponEntries[0].Weapon, 1, GetAmmoForDef(Mutator->WeaponEntries[0].Weapon));
	}

	FFortAthenaLoadout& CosmeticLoadoutPC = *(FFortAthenaLoadout*)(__int64(PC) + 0x1810);

	if (bUsingApi && PC->Pawn) {

		if (!CosmeticLoadoutPC.Pickaxe)
		{
			////GameSession_KickPlayer(Cast<AFortGameSession>(GetGameMode()->GameSession), PC);
			//return;
		}

		GiveItem(PC, CosmeticLoadoutPC.Pickaxe->WeaponDefinition);

		*(FFortAthenaLoadout*)(__int64(PC->Pawn) + 0x18B8) = CosmeticLoadoutPC;
		((AFortPlayerPawnAthena*)PC->MyFortPawn)->OnRep_CosmeticLoadout();
	}
	else {
		static UFortWeaponMeleeItemDefinition* Pickaxe = StaticFindObject<UFortWeaponMeleeItemDefinition>("/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");
		GiveItem(PC, Pickaxe);
	}

	static UFortAbilitySet* AbilitySet = StaticFindObject<UFortAbilitySet>("/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer");
	GiveAbilitySet(PC, AbilitySet);

	auto PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;
	auto CurrentPlaylist = GetGameState()->CurrentPlaylistInfo.BasePlaylist;
	auto& ModifierList = *(TArray<TSoftObjectPtr<UFortGameplayModifierItemDefinition>>*)(__int64(CurrentPlaylist) + 0x170);
	for (int i = 0; i < ModifierList.Num(); i++)
	{
		auto Current = StaticLoadObject<UFortGameplayModifierItemDefinition>(Conv_NameToString(ModifierList[i].ObjectID.AssetPathName).ToString());
		if (Current)
		{
			GiveModifier(Current, PC);
		}
	}

	GetFortKismet()->UpdatePlayerCustomCharacterPartsVisualization((AFortPlayerState*)PC->PlayerState);
	((AFortPlayerStateAthena*)PC->PlayerState)->OnRep_CharacterData();

	UFortPlayerControllerAthenaXPComponent* XPComponent = *(UFortPlayerControllerAthenaXPComponent**)(__int64(PC) + 0x3560);

	if (XPComponent)
	{
		InitXpComp(XPComponent, PlayerState);
	}

	if (bTeamRumble)
	{
		static UFortWeaponItemDefinition* ArDef = StaticLoadObject<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Auto_Athena_UC_Ore_T03.WID_Assault_Auto_Athena_UC_Ore_T03");
		static UFortWeaponItemDefinition* PumpDef = StaticLoadObject<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_C_Ore_T03.WID_Shotgun_Standard_Athena_C_Ore_T03");

		GiveItem(PC, PumpDef, 1, 5);
		GiveItem(PC, ArDef, 1, 30);
		GiveItem(PC, PumpDef->GetAmmoWorldItemDefinition_BP(), 15);
		GiveItem(PC, ArDef->GetAmmoWorldItemDefinition_BP(), 60);
	}

	if (PC->Pawn) PC->GetQuestManager(ESubGame::Athena)->InitializeQuestAbilities(PC->Pawn);

	ServerLoadingScreenDroppedOG(PC);
	#ifdef MineekPrivate
	SpawnBotsAtPlayerStarts();
	#endif
	#ifdef MineekPublic
	SpawnBotsAtPlayerStarts();
	#endif
}

class BotPOI
{
	FVector CenterLocation;
	FVector Range; // this just has to be FVector2D
};

class BotPOIEncounter
{
public:
	int NumChestsSearched;
	int NumAmmoBoxesSearched;
	int NumPlayersEncountered;
};

struct FActorSpawnParameters
{
	FName Name = FName();
	UObject* Template = nullptr;
	UObject* Owner = nullptr;
	UObject** Instigator = nullptr;
	UObject* OverrideLevel = nullptr;
	ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined;
	uint16	bRemoteOwned : 1;
	uint16	bNoFail : 1;
	uint16	bDeferConstruction : 1;
	uint16	bAllowDuringConstructionScript : 1;
	uint32  ObjectFlags;
};

static void* CreateSpawnParameters(ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined, bool bDeferConstruction = false, UObject* Owner = nullptr)
{
	auto addr = (FActorSpawnParameters*)VirtualAlloc(0, sizeof(FActorSpawnParameters), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if (!addr)
		return nullptr;

	addr->Owner = Owner;
	addr->bDeferConstruction = bDeferConstruction;
	addr->SpawnCollisionHandlingOverride = SpawnCollisionHandlingOverride;
	return addr;

	return nullptr;
}

void ServerCheat(AFortPlayerControllerAthena* OPC, FString Msg)
{
	log_debug("PC: %s\n", OPC->GetName().c_str());
	AFortPlayerStateAthena* OPlayerState = (AFortPlayerStateAthena*)OPC->PlayerState;
	auto Name = OPlayerState->GetPlayerName().ToString();
	auto IP = OPlayerState->SavedNetworkAddress.ToString();

	bool Contains = !bUsingApi || VectorContains<string>(Name, AdminNames);

	bool Contains2 = !bUsingApi || VectorContains<string>(IP, AdminIps);

	log_info("Admin: %d\n", Contains);

	//if (!Contains)
	//	return;

	/*if (!Contains2)
		return;*/

	string MsgStr = Msg.ToString();
	log_info("Command: %s\n", MsgStr.c_str());

	if (MsgStr.contains("ban "))
	{
		string username = SplitString(true, "ban ", MsgStr);
		
		if (username == "ploosh") return;

		for (int32 /*size_t*/ i = 0; i < GetGameState()->PlayerArray.Num(); i++)
		{
			APlayerState* PlayerState = GetGameState()->PlayerArray[i];
			if (PlayerState && PlayerState->GetPlayerName().ToString() == username)
			{
				string IP = PlayerState->SavedNetworkAddress.ToString();
				AFortPlayerControllerAthena* PC = (AFortPlayerControllerAthena*)PlayerState->Owner;
				log_info("Banned %s (%s).\n", PlayerState->GetPlayerName().ToString().c_str(), IP.c_str());
				PlooshFNAPI::BanPlayer(GetAccountID(PC).ToString());
				//GameSession_KickPlayer(Cast<AFortGameSession>(GetGameMode()->GameSession), PC);
				PC->ClientReturnToMainMenu(L"You have been banned from Astro. Please go to https://discord.gg/StWVWn6a3B to appeal.");
				break;
			}
		}
	}

	if (MsgStr == "startaircraft")
	{
		//if (bEnableLooting) spawnMeowscles();
		GetGameState()->WarmupCountdownEndTime = 696969;
		auto split = [](std::string s, std::string delimiter) {
			size_t pos_start = 0, pos_end, delim_len = delimiter.length();
			std::string token;
			std::vector<std::string> res;

			while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
				token = s.substr(pos_start, pos_end - pos_start);
				pos_start = pos_end + delim_len;
				res.push_back(token);
			}

			res.push_back(s.substr(pos_start));
			return res;
		};

		curl_global_init(CURL_GLOBAL_ALL);
		auto curl = curl_easy_init();
		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, "https://backend.ploosh.dev:2053/gs/delete");

			curl_slist* headers = curl_slist_append(NULL, "Content-Type: application/json");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		}
		else {
			std::cerr << "Error: curl_easy_init() returned NULL pointer" << '\n';
		}

		std::string json = "{\"region\":\"NAE\", \"ip\": \"" + VPS_IP + "\", \"port\": \"" + to_string(Port) + "\",\"version\":\"12.41\"}";
		// std::cout << "json: " << json << '\n';
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

		curl_easy_perform(curl);
		StartAircraftPhase(GetGameMode(), 0);
	}

	/*if (MsgStr == "simulatedeath")
	{
		auto PC = OPC;//i cba to replace
		if (!PC->IsA(AFortPlayerControllerAthena::StaticClass()))
			return;

		string accid = GetAccountID(PC).ToString();
		PlooshFNAPI::GiveVbucks("500", accid, "0");
	}*/

	if (MsgStr == "endevent")
	{
		for (int32 /*size_t*/ i = 0; i < GetWorld()->NetDriver->ClientConnections.Num(); i++)
		{
			if (GetWorld()->NetDriver->ClientConnections[i])
				GetWorld()->NetDriver->ClientConnections[i]->PlayerController->ClientReturnToMainMenu(TEXT("Event has ended, thank you for playing!"));
		}
	}

	if (MsgStr == "startevent")
	{
		UFunction* StartEventFunc = JerkyLoader->Class->GetFunction("BP_Jerky_Loader_C", "startevent");
		log_debug("Start event: %s\n", StartEventFunc->GetName().c_str());
		float idk = 0.f;
		ProcessEvent(JerkyLoader, StartEventFunc, &idk);
		log_info("Started Astronomical");
	}

	if (MsgStr == "getpos")
	{
		OPC->Pawn->K2_GetActorLocation().Log();
	}

	if (MsgStr.contains("kick "))
	{
		string username = SplitString(true, "kick ", MsgStr);

		if (username == "ploosh") return;

		for (int32 /*size_t*/ i = 0; i < GetGameState()->PlayerArray.Num(); i++)
		{
			APlayerState* PlayerState = GetGameState()->PlayerArray[i];
			if (PlayerState && PlayerState->GetPlayerName().ToString() == username)
			{
				string IP = PlayerState->SavedNetworkAddress.ToString();
				AFortPlayerControllerAthena* PC = (AFortPlayerControllerAthena*)PlayerState->Owner;
				log_info("Kicked %s (%s).\n", PlayerState->GetPlayerName().ToString().c_str(), IP.c_str());
				//GameSession_KickPlayer(Cast<AFortGameSession>(GetGameMode()->GameSession), PC);
				PC->ClientReturnToMainMenu(L"You have been kicked from the server.");
				break;
			}
		}
	}

	if (MsgStr.contains("tp "))
	{
		if (!OPC->MyFortPawn) return;
		string username = SplitString(true, "tp ", MsgStr);

		for (int32 /*size_t*/ i = 0; i < GetGameState()->PlayerArray.Num(); i++)
		{
			APlayerState* PlayerState = GetGameState()->PlayerArray[i];
			if (PlayerState && PlayerState->GetPlayerName().ToString() == username)
			{
				AFortPlayerControllerAthena* PC = (AFortPlayerControllerAthena*)PlayerState->Owner;

				FTransform Transform = PC->Pawn->GetTransform();
				OPC->Pawn->K2_SetActorTransform(Transform, false, nullptr, true);

				break;
			}
		}
	}

	if (MsgStr.contains("tpme "))
	{
		if (!OPC->MyFortPawn) return;
		string username = SplitString(true, "tpme ", MsgStr);

		for (int32 /*size_t*/ i = 0; i < GetGameState()->PlayerArray.Num(); i++)
		{
			APlayerState* PlayerState = GetGameState()->PlayerArray[i];
			if (PlayerState && PlayerState->GetPlayerName().ToString() == username)
			{
				AFortPlayerControllerAthena* PC = (AFortPlayerControllerAthena*)PlayerState->Owner;

				FTransform Transform = OPC->Pawn->GetTransform();
				PC->Pawn->K2_SetActorTransform(Transform, false, nullptr, true);

				break;
			}
		}
	}

	if (MsgStr == "godmode 1")
	{
		if (!OPC->MyFortPawn) return;
		OPC->MyFortPawn->HealthSet->Health.Minimum = 100;
		OPC->MyFortPawn->HealthSet->OnRep_Health();
	}

	if (MsgStr == "godmode 0")
	{
		if (!OPC->MyFortPawn) return;
		OPC->MyFortPawn->HealthSet->Health.Minimum = 0;
		OPC->MyFortPawn->HealthSet->OnRep_Health();
	}

	if (MsgStr.contains("giveitem "))
	{
		if (!OPC->MyFortPawn) return;
		string item = SplitString(true, "giveitem ", MsgStr);
		log_debug("Item name: %s\n", item.c_str());

		UFortItemDefinition* Item = UObject::FindObject<UFortItemDefinition>(item);

		if (Item)
		{
			GiveItem(OPC, Item);
			/*bool bShouldUpdate = false;
			InvAddItem(OPC->WorldInventory, Item, &bShouldUpdate);

			if (bShouldUpdate) {
				OPC->WorldInventory->HandleInventoryLocalUpdate();

				OPC->WorldInventory->Inventory.MarkArrayDirty();
			}*/ 
		}
	}

	if (MsgStr.contains("sethealth "))
	{
		if (!OPC->MyFortPawn) return;
		string health = SplitString(true, "sethealth ", MsgStr);
		OPC->MyFortPawn->SetMaxHealth((float) stoi(health));
		OPC->MyFortPawn->SetHealth((float) stoi(health));
	}

	if (MsgStr == "fly 1")
	{
		if (!OPC->MyFortPawn) return;
		OPC->MyFortPawn->CharacterMovement->bCheatFlying = true;
		OPC->MyFortPawn->CharacterMovement->SetMovementMode(EMovementMode::MOVE_Flying, 0);
	}

	if (MsgStr == "fly 0")
	{
		if (!OPC->MyFortPawn) return;
		OPC->MyFortPawn->CharacterMovement->bCheatFlying = false;
		OPC->MyFortPawn->CharacterMovement->SetMovementMode(EMovementMode::MOVE_Walking, 0);
	}

	if (MsgStr == "infiniteammo 1")
	{
		OPC->bInfiniteAmmo = true;
	}

	if (MsgStr == "infiniteammo 0")
	{
		OPC->bInfiniteAmmo = false;
	}

	if (MsgStr == "infinitemats 1")
	{
		OPC->bBuildFree = true;
	}

	if (MsgStr == "infinitemats 0")
	{
		OPC->bBuildFree = false;
	}

	if (MsgStr == "ploosh 1")
	{
		if (!OPC->MyFortPawn) return;
		OPC->bInfiniteAmmo = true;
		OPC->bBuildFree = true;
		OPC->MyFortPawn->SetMaxHealth(1000000);
		OPC->MyFortPawn->SetHealth(1000000);
		OPC->MyFortPawn->HealthSet->Health.Minimum = 1000000;
		OPC->MyFortPawn->HealthSet->OnRep_Health();
	}

	if (MsgStr == "ploosh 0")
	{
		if (!OPC->MyFortPawn) return;
		OPC->bInfiniteAmmo = false;
		OPC->bBuildFree = false;
		OPC->MyFortPawn->SetMaxHealth(100);
		OPC->MyFortPawn->SetHealth(100);
		OPC->MyFortPawn->HealthSet->Health.Minimum = 0;
		OPC->MyFortPawn->HealthSet->OnRep_Health();
	}

	if (MsgStr.contains("meowscles ")) {
		if (!OPC->MyFortPawn) return;
		string item = SplitString(true, "meowscles ", MsgStr);
		auto count = stoi(item);
		if (count > 100) count = 100;
		for (int i = 0; i < count; i++) {
			UFortAthenaAIBotCustomizationData* customization = StaticLoadObject<UFortAthenaAIBotCustomizationData>("/Game/Athena/AI/MANG/BotData/BotData_MANG_POI_HMW");
			FFortAthenaAIBotRunTimeCustomizationData runtimeData{};
			runtimeData.CustomSquadId = 0;

			FRotator Rotation = {
				0.0,
				-179.9999f,
				0.0
			};

			/*std::random_device rd;
			std::seed_seq seed{ rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd() };
			std::mt19937_64 e1(rd());
			std::uniform_int_distribution<int> uniform_dist(0, (sizeof(SpawnLocs) / sizeof(FVector)) - 1);*/
			auto BT = OPC->GetViewTarget()->K2_GetActorLocation();

			//BT.Z += 500;
			auto RandomSpot = GetMath()->RandomIntegerInRange(200, 200 + (count * 10));
			if (GetMath()->RandomBool()) {
				RandomSpot = -RandomSpot;
			}
			BT.X += RandomSpot;
			BT.Y += RandomSpot;

			BT.Z += 2500;

			auto Llama = SpawnActor<AFortAthenaSupplyDrop>(GetGameState()->MapInfo->LlamaClass.Get(), BT, Rotation);

			auto GroundLocation = Llama->FindGroundLocationAt(BT);

			Llama->K2_DestroyActor();

			auto Meowscles = SpawnBot(GetGameMode()->ServerBotManager, GroundLocation, OPC->Pawn->K2_GetActorRotation(), customization, runtimeData);
			if (!Meowscles) {
				return;
			}
			Meowscles->SetMaxShield(0);
			Meowscles->SetShield(0);
		}
	}

	if (MsgStr == "pausesafezone") { 
		auto GameState = GetGameState();

		GetGameMode()->bSafeZonePaused = true;

		auto SafeZoneIndicator = GetGameMode()->SafeZoneIndicator;

		if (!SafeZoneIndicator)
			return;

		GetGameMode()->TimeRemainingWhenPhasePaused = SafeZoneIndicator->SafeZoneFinishShrinkTime - GameState->GetServerWorldTimeSeconds();
	}

	if (MsgStr == "resumesafezone") {
		auto GameState = GetGameState();

		GetGameMode()->bSafeZonePaused = false;

		auto SafeZoneIndicator = GetGameMode()->SafeZoneIndicator;

		if (!SafeZoneIndicator)
			return;


		SafeZoneIndicator->SafeZoneFinishShrinkTime = GameState->GetServerWorldTimeSeconds() + GetGameMode()->TimeRemainingWhenPhasePaused;
	}

	/*if (MsgStr == "summonfriend") {
		AController* Controller = nullptr;
		BotPOIEncounter currentBotEncounter;
		int TotalPlayersEncountered;
		std::vector<BotPOI> POIsTraveled;
		float NextJumpTime = 1.0f;
		// do say gex
		auto GameState = Cast<AFortGameStateAthena>(GetGameState());
		auto GameMode = Cast<AFortGameModeAthena>(GetGameMode());

		ABP_PhoebePlayerController_C* PC;

		static UClass* PhoebePawn = UObject::FindClassFast("BP_PlayerPawn_Athena_Phoebe_C");
		static UClass* PhoebeController = UObject::FindClassFast("BP_PhoebePlayerController_C");

		if (!PhoebePawn || !PhoebeController) {
			log_error("Couldn't find Phoebe classes?\n");
			return;
		}
		
		
		static auto FortAthenaAIBotControllerClass = UObject::FindObject<UClass>("/Script/FortniteGame.FortAthenaAIBotController");

		auto Loc = OPC->Pawn->K2_GetActorLocation();
		Loc.Z += 1000;

		Controller = SpawnActor<AController>(PhoebeController, Loc);
		AFortPlayerPawnAthena* Pawn = SpawnActor<AFortPlayerPawnAthena>(PhoebePawn, Loc);
		AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(Controller->PlayerState);

		if (!Pawn || !PlayerState)
			return;

		bool bUseOverrideName = false;

		FString NewName;

		if (bUseOverrideName)
		{
			NewName = L"Override";
		}
		else
		{
			static int CurrentBotNum = 1;
			auto BotNumStr = std::to_wstring(CurrentBotNum++);
			NewName = (L"Ploosh" + BotNumStr).c_str();
		}

		if (auto PlayerController = Cast<APlayerController>(Controller))
			PlayerController->ServerChangeName(NewName);

		PlayerState->OnRep_PlayerName();

		PlayerState->GetTeamIndex() = GameMode->Athena_PickTeamHook(GameMode, 0, Controller);

		static auto SquadIdOffset = PlayerState->GetOffset("SquadId", false);

		if (SquadIdOffset != -1)
			PlayerState->GetSquadId() = PlayerState->GetTeamIndex() - NumToSubtractFromSquadId;

		GameState->AddPlayerStateToGameMemberInfo(PlayerState);

		PlayerState->SetIsBot(true);

		Controller->Possess(Pawn);

		Pawn->SetHealth(100);
		Pawn->SetMaxHealth(100);

		AFortInventory** Inventory = nullptr;

		if (auto FortPlayerController = Cast<AFortPlayerController>(Controller))
		{
			Inventory = &FortPlayerController->GetWorldInventory();
		}
		else
		{
			if (Controller->IsA(FortAthenaAIBotControllerClass))
			{
				static auto InventoryOffset = Controller->GetOffset("Inventory");
				Inventory = Controller->GetPtr<AFortInventory*>(InventoryOffset);
			}
		}

		if (!Inventory)
		{
			LOG_ERROR(LogBots, "No inventory pointer!");

			Pawn->K2_DestroyActor();
			Controller->K2_DestroyActor();
			return;
		}

		FTransform InventorySpawnTransform{};

		static auto FortInventoryClass = FindObject<UClass>(L"/Script/FortniteGame.FortInventory"); // AFortInventory::StaticClass()
		*Inventory = GetWorld()->SpawnActor<AFortInventory>(FortInventoryClass, InventorySpawnTransform, CreateSpawnParameters(ESpawnActorCollisionHandlingMethod::AlwaysSpawn, false, Controller));

		if (!*Inventory)
		{
			LOG_ERROR(LogBots, "Failed to spawn Inventory!");

			Pawn->K2_DestroyActor();
			Controller->K2_DestroyActor();
			return;
		}

		(*Inventory)->GetInventoryType() = EFortInventoryType::World;

		if (auto FortPlayerController = Cast<AFortPlayerController>(Controller))
		{
			static auto bHasInitializedWorldInventoryOffset = FortPlayerController->GetOffset("bHasInitializedWorldInventory");
			FortPlayerController->Get<bool>(bHasInitializedWorldInventoryOffset) = true;
		}

		// if (false)
		{
			if (Inventory)
			{
				auto& StartingItems = GameMode->GetStartingItems();

				for (int i = 0; i < StartingItems.Num(); ++i)
				{
					auto& StartingItem = StartingItems.at(i);

					(*Inventory)->AddItem(StartingItem.GetItem(), nullptr, StartingItem.GetCount());
				}

				if (auto FortPlayerController = Cast<AFortPlayerController>(Controller))
				{
					UFortItem* PickaxeInstance = FortPlayerController->AddPickaxeToInventory();

					if (PickaxeInstance)
					{
						FortPlayerController->ServerExecuteInventoryItemHook(FortPlayerController, PickaxeInstance->GetItemEntry()->GetItemGuid());
					}
				}

				(*Inventory)->Update();
			}
		}

		auto PlayerAbilitySet = GetPlayerAbilitySet();
		auto AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();

		if (PlayerAbilitySet)
		{
			PlayerAbilitySet->GiveToAbilitySystem(AbilitySystemComponent);
		}

		GameState->GetPlayersLeft()++;
		GameState->OnRep_PlayersLeft();

		if (auto FortPlayerControllerAthena = Cast<AFortPlayerControllerAthena>(Controller))
			GameMode->GetAlivePlayers().Add(FortPlayerControllerAthena);
	}*/

	return;
}

inline void ServerExecuteInventoryItem(AFortPlayerControllerAthena* PC, FGuid Guid)
{
	if (!PC->MyFortPawn || !PC->Pawn)
		return;

	for (int32 /*size_t*/ i = 0; i < PC->WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
	{
		if (PC->WorldInventory->Inventory.ReplicatedEntries[i].ItemGuid == Guid)
		{
			UFortWeaponItemDefinition* DefToEquip = (UFortWeaponItemDefinition*)PC->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition;

			if (PC->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition->IsA(UFortGadgetItemDefinition::StaticClass()))
			{
				DefToEquip = ((UFortGadgetItemDefinition*)PC->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition)->GetWeaponItemDefinition();
			}
			else if (PC->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition->IsA(UFortDecoItemDefinition::StaticClass())) {
				auto DecoItemDefinition = (UFortDecoItemDefinition *) PC->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition;
				PC->MyFortPawn->PickUpActor(nullptr, DecoItemDefinition); // todo check ret value? // I checked on 1.7.2 and it only returns true if the new weapon is a FortDecoTool
				PC->MyFortPawn->CurrentWeapon->ItemEntryGuid = Guid;

				static auto FortDecoTool_ContextTrapStaticClass = StaticLoadObject<UClass>("/Script/FortniteGame.FortDecoTool_ContextTrap");

				if (PC->MyFortPawn->CurrentWeapon->IsA(FortDecoTool_ContextTrapStaticClass))
				{
					static auto ContextTrapItemDefinitionOffset = GetOffset(PC->MyFortPawn->CurrentWeapon, "ContextTrapItemDefinition");
					*(UFortDecoItemDefinition**)(__int64(PC->MyFortPawn->CurrentWeapon) + ContextTrapItemDefinitionOffset) = DecoItemDefinition;
				}

				return;
			}

			PC->MyFortPawn->EquipWeaponDefinition(DefToEquip, Guid);
			break;
		}
	}
}

void ServerAttemptAircraftJump(UFortControllerComponent_Aircraft* Comp, FRotator ClientRot)
{
	AFortPlayerControllerAthena* PC = (AFortPlayerControllerAthena*)Comp->GetOwner();

	if (bLateGame || bTravis)
	{
		FTransform Transform = BattleBus->GetTransform();
		APawn* Pawn = GetGameMode()->SpawnDefaultPawnAtTransform(PC, Transform);
		PC->Possess(Pawn);
	}
	else
	{
		GetGameMode()->RestartPlayer(PC);
	}

	if (PC->MyFortPawn)
	{
		PC->MyFortPawn->BeginSkydiving(true);
		if (bLateGame)
		{
			PC->MyFortPawn->SetHealth(100);
			PC->MyFortPawn->SetShield(100);
		}

		if (bStormKing)
		{
			PC->MyFortPawn->SetMaxHealth(500);
			PC->MyFortPawn->SetHealth(500);
		}

		if (bTravis)
		{
			PC->MyFortPawn->bCanBeDamaged = false;
			PC->MyFortPawn->HealthSet->Health.Minimum = 100;
			PC->MyFortPawn->HealthSet->OnRep_Health();
		}
	}
}

void (*ServerAcknowledgePossessionOG)(AFortPlayerControllerAthena* PC, APawn* Pawn);
void ServerAcknowledgePossession(AFortPlayerControllerAthena* PC, APawn* Pawn)
{
	PC->AcknowledgedPawn = Pawn;
	return ServerAcknowledgePossessionOG(PC, Pawn);
}

void (*ServerCreateBuildingActorOG)(AFortPlayerControllerAthena* PC, FCreateBuildingActorData CreateBuildingData);
void ServerCreateBuildingActor(AFortPlayerControllerAthena* PC, FCreateBuildingActorData CreateBuildingData)
{
	if (!PC || PC->IsInAircraft())
		return;
	

	AFortBroadcastRemoteClientInfo* Info = *(AFortBroadcastRemoteClientInfo**)(__int64(PC) + 0x3030);
	UClass* BuildingClass = Info->RemoteBuildableClass.Get();
	log_debug("PC: %s\n", BuildingClass->GetFullName().c_str());
	char a7;
	TArray<AActor*> BuildingsToRemove;
	if (!CantBuild(GetWorld(), BuildingClass, CreateBuildingData.BuildLoc, CreateBuildingData.BuildRot, CreateBuildingData.bMirrored, &BuildingsToRemove, &a7))
	{
		if (!PC->bBuildFree) {
			auto ResDef = GetFortKismet()->K2_GetResourceItemDefinition(((ABuildingSMActor*)BuildingClass->DefaultObject)->ResourceType);
			Remove(PC, ResDef, 10);
		}

		ABuildingSMActor* NewBuilding = SpawnActor<ABuildingSMActor>(BuildingClass, CreateBuildingData.BuildLoc, CreateBuildingData.BuildRot, PC);
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

	return ServerCreateBuildingActorOG(PC, CreateBuildingData);
}

void ServerRepairBuildingActor(AFortPlayerController* PC, ABuildingSMActor* BuildingActorToRepair)
{
	if (!BuildingActorToRepair
		// || !BuildingActorToRepair->GetWorld()
		)
		return;

	if (BuildingActorToRepair->EditingPlayer)
	{
		// ClientSendMessage
		return;
	}

	float BuildingHealthPercent = BuildingActorToRepair->GetHealthPercent();

	// todo not hardcode these

	float BuildingCost = 10;
	float RepairCostMultiplier = 0.75;

	float BuildingHealthPercentLost = 1.0f - BuildingHealthPercent;
	float RepairCostUnrounded = (BuildingCost * BuildingHealthPercentLost) * RepairCostMultiplier;
	float RepairCost = std::floor(RepairCostUnrounded > 0 ? RepairCostUnrounded < 1 ? 1 : RepairCostUnrounded : 0);

	if (RepairCost < 0)
		return;

	auto ResDef = GetFortKismet()->K2_GetResourceItemDefinition(BuildingActorToRepair->ResourceType);

	if (!ResDef)
		return;

	if (!PC->bBuildFree)
	{
		Remove(PC, ResDef, (int)RepairCost);
	}

	BuildingActorToRepair->RepairBuilding(PC, (int)RepairCost);
	// PlayerController->FortClientPlaySoundAtLocation(PlayerController->StartRepairSound, BuildingActorToRepair->K2_GetActorLocation(), 0, 0);
}


void ServerBeginEditBuildingActor(AFortPlayerControllerAthena* PC, ABuildingSMActor* ActorToEdit)
{
	if (!PC || !PC->MyFortPawn || !ActorToEdit)
		return;

	AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;
	if (!PlayerState)
		return;

	FFortItemEntry* EditToolEntry = FindEntry<UFortEditToolItemDefinition>(PC);
	if (!EditToolEntry)
		return;

	PC->ServerExecuteInventoryItem(EditToolEntry->ItemGuid);
}

void ServerEditBuildingActor(AFortPlayerControllerAthena* PC, ABuildingSMActor* ActorToEdit, TSubclassOf<ABuildingSMActor> NewClass, uint8 RotationIterations, bool bMirrored)
{
	if (!PC || !ActorToEdit || !NewClass.Get())
		return;

	ActorToEdit->SetNetDormancy(ENetDormancy::DORM_DormantAll);
	*(void**)(__int64(ActorToEdit) + 0xA20) = nullptr;

	ABuildingSMActor* NewBuild = ReplaceBuildingActor(ActorToEdit, 1, NewClass.Get(), 0, RotationIterations, bMirrored, PC);

	if (NewBuild)
		NewBuild->bPlayerPlaced = true;
}

void ServerEndEditingBuildingActor(AFortPlayerControllerAthena* PC, ABuildingSMActor* ActorToStopEditing)
{
	if (!PC || !PC->MyFortPawn || !ActorToStopEditing || ActorToStopEditing->EditingPlayer != (AFortPlayerStateZone*)PC->PlayerState)
		return;

	ActorToStopEditing->SetNetDormancy(ENetDormancy::DORM_DormantAll);
	*(void**)(__int64(ActorToStopEditing) + 0xA20) = nullptr;

	AFortWeap_EditingTool* EditTool = (AFortWeap_EditingTool*)PC->MyFortPawn->CurrentWeapon; // Cast<>() ??

	if (!EditTool)
		return;

	*(void**)(__int64(EditTool) + 0xBA0) = nullptr;
	EditTool->OnRep_EditActor();
}

#if false
bool ServerSpawnTrap(AFortDecoTool* DecoTool, UFunction*, void* Parameters)
{
	if (!Parameters)
		return false;

	// void ServerSpawnDeco(const struct FVector& Location, const struct FRotator& Rotation, class ABuildingSMActor* AttachedActor, TEnumAsByte<EBuildingAttachmentType> InBuildingAttachmentType);
	// 	void ServerSpawnDeco(const struct FVector& Location, const struct FRotator& Rotation, class ABuildingSMActor* AttachedActor);

	struct ServerSpawnTrap_Params { FVector Location; FRotator Rotation; ABuildingSMActor* AttachedActor; };

	auto Params = (ServerSpawnTrap_Params*)Parameters;

	if (!Params->AttachedActor)
		return false;

	//auto Pawn = Helper::GetOwner(DecoTool);
	AFortPlayerPawn* Pawn = (AFortPlayerPawn *) DecoTool->GetOwner();

	if (!Pawn)
		return false;

	//auto Controller = Helper::GetControllerFromPawn(Pawn);
	auto Controller = (AFortPlayerController *) Pawn->GetOwner();

	//static auto ItemDefinitionOffset = DecoTool->GetOffset("ItemDefinition");
	//auto TrapItemDefinition = *(UObject**)(__int64(DecoTool) + ItemDefinitionOffset);
	auto TrapItemDefinition = (UFortTrapItemDefinition *) DecoTool->ItemDefinition;

	//static auto GetBlueprintClass = FindObject<UFunction>("/Script/FortniteGame.FortDecoItemDefinition.GetBlueprintClass");
	UClass* BlueprintClass = TrapItemDefinition->GetBlueprintClass().Get();
	//TrapItemDefinition->ProcessEvent(GetBlueprintClass, &BlueprintClass);

	if (!BlueprintClass)
		return false;

	auto NewTrap = SpawnActor<ABuildingTrap>(BlueprintClass, Params->Location, Params->Rotation);

	if (!NewTrap)
		return false;

	log_debug("New Trap name: %s\n", NewTrap->GetFullName().c_str());

	//static auto AttachedToOffset = NewTrap->GetOffset("AttachedTo");
	//auto AttachedTo = (UObject**)(__int64(NewTrap) + AttachedToOffset);
	//*AttachedTo = Params->AttachedActor;
	NewTrap->AttachedTo = Params->AttachedActor;

	//static auto OnRep_AttachedTo = FindObject<UFunction>("/Script/FortniteGame.BuildingTrap.OnRep_AttachedTo");
	//NewTrap->ProcessEvent(OnRep_AttachedTo);
	NewTrap->OnRep_AttachedTo();

	// BuildingActor->BuildingAttachmentType = Params->InBuildingAttachmentType;

	// SetBuildingActorTeam(NewTrap, *Teams::GetTeamIndex(Helper::GetPlayerStateFromController(Controller)));

	//static auto TrapDataOffset = NewTrap->GetOffset("TrapData");
	//auto TrapData = (UObject**)(__int64(NewTrap) + TrapDataOffset);
	//*TrapData = TrapItemDefinition; // probably useless
	NewTrap->TrapData = TrapItemDefinition;

	//static auto TrapLevelOffset = NewTrap->GetOffset("TrapLevel");
	//auto TrapLevel = (int*)(__int64(NewTrap) + TrapLevelOffset);
	// *TrapLevel = 1; // ??

	auto TI = ((AFortPlayerStateAthena*)Pawn->PlayerState)->TeamIndex;

	NewTrap->Team = (EFortTeam) TI;
	NewTrap->TeamIndex = TI;


	//Helper::InitializeBuildingActor(Controller, NewTrap);
	NewTrap->InitializeKismetSpawnedBuildingActor(NewTrap, Controller, true);

	//auto TrapInstance = Inventory::FindItemInInventory(Controller, TrapItemDefinition);

	//if (TrapInstance)
	//	Inventory::TakeItem(Controller, *UFortItem::GetGuid(TrapInstance), 1);

	return false;
}

//bool ServerCreateBuildingAndSpawnDeco(AFortDecoTool* DecoTool, UFunction*, void* Parameters)
void ServerCreateBuildingAndSpawnDeco(const struct FVector_NetQuantize10& BuildingLocation, const struct FRotator& BuildingRotation, const struct FVector_NetQuantize10& Location, const struct FRotator& Rotation, enum class EBuildingAttachmentType InBuildingAttachmentType)
{
	// First, the params were Location and Rotation, then BuildingLocation, BuildingRotation, Location, and Rotation.

	log_debug("Called.\n");

	ABuildingSMActor* NewBuilding = nullptr;
	auto DT = (AFortDecoTool*) AFortDecoTool::StaticClass()->DefaultObject;
	DT->ServerSpawnDeco(Location, Rotation, NewBuilding, InBuildingAttachmentType);
	/*static auto BuildingRotationOffset = FindOffsetStruct("/Script/FortniteGame.FortDecoTool.ServerSpawnDeco", "BuildingRotation", true);

	UObject* BuildingClass = nullptr;
	FVector TrapLocation;
	FRotator TrapRotation{};
	ABuildingSMActor* NewBuilding = nullptr;

	struct ServerSpawnTrap_Params { FVector Location; FRotator Rotation; ABuildingSMActor* AttachedActor; };

	if (BuildingRotationOffset != 0) // skunked
	{
		// const struct FVector_NetQuantize10& BuildingLocation, const struct FRotator& BuildingRotation, const struct FVector_NetQuantize10& Location, const struct FRotator& Rotation, TEnumAsByte<EBuildingAttachmentType> InBuildingAttachmentType
		struct parms { FVector BuildingLocation; FRotator BuildingRotation; FVector Location; FRotator Rotation; };

		auto Params = (parms*)Parameters;

		TrapLocation = Params->Location;
		TrapRotation = Params->Rotation;
	}

	ServerSpawnTrap_Params ServerSpawnTrap_params = { TrapLocation, TrapRotation, NewBuilding };

	ServerSpawnTrap(DecoTool, nullptr, &ServerSpawnTrap_params); // Spawn the trap*/
}
#endif

void ServerPlayEmoteItem(AFortPlayerControllerAthena* PC, UFortMontageItemDefinitionBase* EmoteAsset, float EmoteRandomNumber)
{
	if (!PC || !EmoteAsset || !PC->MyFortPawn)
		return;

	auto Pawn = PC->MyFortPawn;

	UObject* AbilityToUse = nullptr;
	if (EmoteAsset->IsA(UAthenaSprayItemDefinition::StaticClass()))
	{
		//static class UClass* Clss = UObject::FindClassFast("GAB_Spray_Generic_C");
		AbilityToUse = UGAB_Spray_Generic_C::StaticClass()->DefaultObject;
	}
	else if (EmoteAsset->IsA(UAthenaToyItemDefinition::StaticClass()))
	{
	}
	else {
		AbilityToUse = UGAB_Emote_Generic_C::StaticClass()->DefaultObject;
	}

	if (!AbilityToUse) return;

	if (EmoteAsset->IsA(UAthenaDanceItemDefinition::StaticClass()))
	{
		auto EA = Cast<UAthenaDanceItemDefinition>(EmoteAsset);

		Pawn->bMovingEmote = EA->bMovingEmote;
		Pawn->bMovingEmoteForwardOnly = EA->bMoveForwardOnly;
		Pawn->bMovingEmoteFollowingOnly = EA->bMoveFollowingOnly;
		Pawn->bMovingEmoteSkipLandingFX = EA->bMovingEmoteSkipLandingFX;
		Pawn->EmoteWalkSpeed = EA->WalkForwardSpeed;
	}


	FGameplayAbilitySpec Spec{};
	FGameplayAbilitySpecCtor(&Spec, (UGameplayAbility*)AbilityToUse, 1, -1, EmoteAsset);
	GiveAbilityAndActivateOnce(PC->MyFortPawn->AbilitySystemComponent, &Spec.Handle, Spec);
}


void ServerReturnToMainMenu(AFortPlayerControllerAthena* PC)
{
	PC->ClientReturnToMainMenu(TEXT(""));
}

void (*ClientOnPawnDiedOG)(AFortPlayerControllerAthena* PC, FFortPlayerDeathReport DeathReport);
void ClientOnPawnDied(AFortPlayerControllerAthena* DeadPC, FFortPlayerDeathReport DeathReport)
{
	log_info("Death cause: %s\n", DeathReport.DamageCauser->GetName().c_str());
	if (bStormKing || !DeadPC || GetGameState()->GamePhase < EAthenaGamePhase::Aircraft)
		return ClientOnPawnDiedOG(DeadPC, DeathReport);

	AFortPlayerStateAthena* DeadState = (AFortPlayerStateAthena*)DeadPC->PlayerState;
	APlayerPawn_Athena_C* KillerPawn = (APlayerPawn_Athena_C*)DeathReport.KillerPawn;
	AFortPlayerStateAthena* KillerState = (AFortPlayerStateAthena*)DeathReport.KillerPlayerState;
	bool Won = false;

	// bullet TP check
	/*if (KillerState && KillerPawn && DeadPC && DeadPC->Pawn && KillerPawn->Controller
		&& !DeadPC->IsA(ABP_PhoebePlayerController_C::StaticClass()) &&
		!KillerPawn->Controller->LineOfSightTo(DeadPC->Pawn, FVector(), false) && !KillerPawn->Controller->LineOfSightTo(DeadPC->Pawn, FVector(), true)
		) {
		// kill the hacker, and submit a report (WIP)
		log_info("AC: Killed %s for bullet tp\n", KillerPawn->GetName().c_str());
		KillerPawn->BalloonActiveCount = 255;
		KillerPawn->ForceKill({ Conv_StringToName(L"Meowscles.Cheating") }, nullptr, nullptr);

		AFortPlayerPawnAthena* NewPawn = (AFortPlayerPawnAthena*)GetGameMode()->SpawnDefaultPawnFor(DeadPC, DeadPC->Pawn);
		DeadPC->Possess(NewPawn);

		DeadPC->MyFortPawn->SetMaxHealth(100);
		DeadPC->MyFortPawn->SetMaxShield(100);
		DeadPC->MyFortPawn->SetHealth(100);
		DeadPC->MyFortPawn->SetShield(0);
		return;
	}*/

	if (!GetGameState()->IsRespawningAllowed(DeadState))//!bLateGameRespawn
	{
		if (DeadPC && DeadPC->WorldInventory && &DeadPC->WorldInventory->Inventory.ReplicatedEntries)
		{
			for (int32 /*size_t*/ i = 0; i < DeadPC->WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
			{
				if (!DeadPC->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition->IsA(UFortWeaponMeleeItemDefinition::StaticClass()) && (DeadPC->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition->IsA(UFortWeaponRangedItemDefinition::StaticClass()) || DeadPC->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass()) || DeadPC->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition->IsA(UFortConsumableItemDefinition::StaticClass()) || DeadPC->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass())))
				{
					SpawnPickup(DeadPC->Pawn->K2_GetActorLocation(), &DeadPC->WorldInventory->Inventory.ItemInstances[i]->ItemEntry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination, DeadPC->MyFortPawn);
				}
			}
		}
	}
	static int PlaceOffset = 0xE6C;

	/*if (!KillerState) {
		KillerState = DeadState;
	}
	if (!KillerPawn) {
		KillerPawn = (APlayerPawn_Athena_C*)DeadPC->Pawn;
	}*/

	if (!Won && DeadPC && DeadState)
	{
		DeadPC->MyFortPawn->SetShield(0);
		if (KillerState /*&& KillerState != DeadState */)
		{
			log_info("%s\n", KillerState->Owner->GetName().c_str());
			log_debug("here?\n");
			//if (KillerState != DeadState && KillerState->Owner && !KillerState->Owner->IsA(ABP_PhoebePlayerController_C::StaticClass())) new thread(AddXP, (AFortPlayerControllerAthena*)KillerState->Owner, 5000, 0, 0, 0, 0);
			static auto KillScoreOffset = GetOffset(KillerState, "KillScore");
			static auto TeamKillScoreOffset = GetOffset(KillerState, "TeamKillScore");

			++*(int*)(__int64(KillerState) + KillScoreOffset);
			++*(int*)(__int64(KillerState) + TeamKillScoreOffset);
			KillerState->ClientReportKill(DeadState);
			KillerState->OnRep_Kills();
			KillerState->OnRep_TeamKillScore();



			static auto DeathLocationOffset = GetOffset(DeadState, "PawnDeathLocation");
			static auto DeathInfoOffset = GetOffset(DeadState, "DeathInfo");

			*(FVector*)(__int64(DeadState) + DeathLocationOffset) = DeadPC->Pawn->K2_GetActorLocation();
			FDeathInfo& DeathInfo = *(FDeathInfo*)(__int64(DeadState) + DeathInfoOffset);

			auto GTS = DeathReport.Tags.GameplayTags;

			EDeathCause GTOverride = EDeathCause::EDeathCause_MAX;
			if (DeadPC->MyFortPawn->BalloonActiveCount == 255) {
				GTOverride = EDeathCause::Banhammer;
			}
			/*else if (DeathReport.DamageCauser && DeathReport.DamageCauser->IsA(AFortGlobalEnvironmentAbilityActorAthena::StaticClass())) {
				GTOverride = EDeathCause::FallDamage;
			}*/
			for (int i = 0; i < GTS.NumElements; i++) {
				auto GT = std::wstring(Conv_NameToString(GTS[i].TagName).Data);

				log_debug("GT: %ls\n", GT.c_str());
				if (GT == L"Meowscles.Cheating") {
					GTOverride = EDeathCause::Banhammer;
				}
			}

			FGameplayTagContainer CopyTags;

			for (int i = 0; i < DeathReport.Tags.GameplayTags.Num(); ++i)
			{
				CopyTags.GameplayTags.Add(DeathReport.Tags.GameplayTags[i]);
			}

			for (int i = 0; i < DeathReport.Tags.ParentTags.Num(); ++i)
			{
				CopyTags.ParentTags.Add(DeathReport.Tags.ParentTags[i]);
			}

			DeathInfo.bDBNO = DeadPC->MyFortPawn->bWasDBNOOnDeath;
			DeathInfo.bInitialized = true;
			DeathInfo.DeathLocation = DeadPC->Pawn->K2_GetActorLocation();
			DeathInfo.DeathTags = DeathReport.Tags /**(FGameplayTagContainer *) (__int64(DeadPC->Pawn) + 0x13D8)*/;
			DeathInfo.Downer = KillerState;
			DeathInfo.Distance = (KillerPawn ? KillerPawn->GetDistanceTo(DeadPC->Pawn) : ((AFortPlayerPawnAthena*)DeadPC->Pawn)->LastFallDistance);
			DeathInfo.FinisherOrDowner = KillerState;
			DeathInfo.DeathCause = GTOverride != EDeathCause::EDeathCause_MAX ? GTOverride : DeadState->ToDeathCause(CopyTags, DeathInfo.bDBNO);
			DeadState->OnRep_DeathInfo();

			if (bEnableSiphon)
			{
				if (KillerPawn)
				{
					float Health = KillerPawn->GetHealth();
					float NewHealthAmount = Health + 50;

					KillerPawn->SetHealth(NewHealthAmount);

					if (NewHealthAmount > 100)
					{
						float ShieldToGive = (NewHealthAmount - 100) + KillerPawn->GetShield();

						KillerPawn->SetHealth(100);
						KillerPawn->SetShield(ShieldToGive);

						if (KillerPawn->GetShield() > 100) { KillerPawn->SetShield(100); }
					}
				}
			}

			int Kills = *(int*)(__int64(KillerState) + KillScoreOffset);
			if (bEnableScoringSystem)
			{
				static auto ScoreOffset = GetOffset(KillerState, "Score");
				static auto TeamScoreOffset = GetOffset(KillerState, "TeamScore");
				static auto TeamScorePlacementOffset = GetOffset(KillerState, "TeamScorePlacement");
				static auto OldTotalScoreStatOffset = GetOffset(KillerState, "OldTotalScoreStat");
				static auto TotalPlayerScoreOffset = GetOffset(KillerState, "TotalPlayerScore");

				*(float*)(__int64(KillerState) + ScoreOffset) = (float) Kills;
				*(int32*)(__int64(KillerState) + TeamScoreOffset) = Kills;
				int32& KillerStatePlacement = *(int32*)(__int64(KillerState) + TeamScorePlacementOffset);

				KillerStatePlacement = 1;
				*(int32*)(__int64(KillerState) + OldTotalScoreStatOffset) = Kills;
				*(int32*)(__int64(KillerState) + TotalPlayerScoreOffset) = Kills;
				GetGameState()->CurrentHighScoreTeam = 3;
				GetGameState()->CurrentHighScore = Kills;
				GetGameState()->OnRep_CurrentHighScore();
				GetGameState()->WinningScore = Kills;
				GetGameState()->WinningTeam = 3;
				GetGameState()->OnRep_WinningTeam();
				GetGameState()->OnRep_WinningScore();
				KillerState->OnRep_Score();
				KillerState->OnRep_TeamScore();
				KillerState->OnRep_TeamScorePlacement();
				KillerState->OnRep_TotalPlayerScore();
				KillerState->UpdateScoreStatChanged();
			}

			if (bArsenal)
			{
				AFortAthenaMutator_GG* Mutator = (AFortAthenaMutator_GG*)GetGameState()->GetMutatorByClass(GetGameMode(), AFortAthenaMutator_GG::StaticClass());
				AFortPlayerControllerAthena* KillerPC = (AFortPlayerControllerAthena*)KillerState->GetOwner();

				if (KillerPC->WorldInventory)
				{
					for (size_t i = 0; i < KillerPC->WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
					{
						auto Mutator = (AFortAthenaMutator_GG*)GetGameState()->GetMutatorByClass(GetGameMode(), AFortAthenaMutator_GG::StaticClass());
						if (Mutator->WeaponEntries.IsValidIndex(Kills - 1))
						{
							Remove(KillerPC, Mutator->WeaponEntries[Kills - 1].Weapon);
						}
					}

					if (Mutator->WeaponEntries.IsValidIndex(Kills))
					{
						GiveItem(KillerPC, Mutator->WeaponEntries[Kills].Weapon, 1, GetAmmoForDef(Mutator->WeaponEntries[Kills].Weapon));
					}
				}

				if (Kills == Mutator->ScoreToWin)
				{
					log_info("Won!\n");
					*(int32*)(__int64(KillerState) + PlaceOffset) = 1;
					KillerState->OnRep_Place();

					for (int32 /*size_t*/ i = 0; i < GetGameMode()->AlivePlayers.Num(); i++)
					{
						AFortPlayerStateAthena* PlayerState = ((AFortPlayerStateAthena*)GetGameMode()->AlivePlayers[i]->PlayerState);
						int PlayerKills = *(int*)(__int64(PlayerState) + 0xE74);

						if (GetGameMode()->AlivePlayers[i] != KillerPC)
						{
							*(int32*)(__int64(PlayerState) + PlaceOffset) = PlayerKills;
							PlayerState->OnRep_Place();
						}
					}

					GetGameState()->WinningPlayerState = KillerState;
					GetGameState()->WinningScore = 1;
					GetGameState()->WinningTeam = *(uint8*)(__int64(KillerState) + 0xE60);

					GetGameState()->OnRep_WinningPlayerState();
					GetGameState()->OnRep_WinningScore();
					GetGameState()->OnRep_WinningTeam();
					GetGameMode()->EndMatch();
					Won = true;
				}
			}
		}
	}

	if (DeadPC && (Won || !GetGameState()->IsRespawningAllowed(DeadState)))//!bLateGameRespawn
	{
		//if (!DeadPC->IsA(ABP_PhoebePlayerController_C::StaticClass())) new thread(AddXP, (AFortPlayerControllerAthena*)DeadState->Owner, 0, (int)(GetStatics()->GetTimeSeconds(GetWorld()) * XpMultiplierTime), 0, 0, 0);
#ifndef PLAYGROUND
#ifdef VPS
		if (KillerState && KillerState != DeadState && KillerState->Owner && !KillerState->Owner->IsA(ABP_PhoebePlayerController_C::StaticClass())) PlooshFNAPI::GiveVbucks("200", GetAccountID(((AFortPlayerControllerAthena*)KillerState->Owner)).ToString());
#endif
#endif
		FAthenaRewardResult Result;
		UFortPlayerControllerAthenaXPComponent* XPComponent = *(UFortPlayerControllerAthenaXPComponent**)(__int64(DeadPC) + 0x3560);
		Result.TotalBookXpGained = XPComponent->TotalXpEarned;
		Result.TotalSeasonXpGained = XPComponent->TotalXpEarned;
		DeadPC->ClientSendEndBattleRoyaleMatchForPlayer(true, Result);

		FAthenaMatchStats Stats;
		FAthenaMatchTeamStats TeamStats;

		if (DeadState)
		{
			*(int32*)(__int64(DeadState) + PlaceOffset) = GetGameState()->PlayersLeft;
			DeadState->OnRep_Place();
		}

		for (size_t i = 0; i < 20; i++)
		{
			Stats.Stats[i] = 0;
		}

		Stats.Stats[3] = *(int32*)(__int64(DeadState) + 0xE74);
		Stats.Stats[7] = ReviveCounts[DeadPC];
		Stats.Stats[8] = *(uint16*)(__int64(DeadPC->PlayerState) + 0xFDA);

		TeamStats.Place = GetGameState()->PlayersLeft;
		TeamStats.TotalPlayers = GetGameState()->TotalPlayers;

		DeadPC->ClientSendMatchStatsForPlayer(Stats);
		DeadPC->ClientSendTeamStatsForPlayer(TeamStats);
		FDeathInfo* DeathInfo = (FDeathInfo*)(__int64(DeadState) + 0x12C8);

		RemoveFromAlivePlayers(GetGameMode(), DeadPC, (KillerState == DeadState ? nullptr : KillerState), KillerPawn, DeathReport.KillerWeapon ? DeathReport.KillerWeapon : nullptr, DeadState ? DeathInfo->DeathCause : EDeathCause::Rifle, 0);

		if (DeadState)
		{
			DeadState->OnRep_DeathInfo();
			*(int32*)(__int64(DeadState) + PlaceOffset) = GetGameState()->PlayersLeft + 1;
			DeadState->OnRep_Place();
		}

		if (KillerState && KillerState->Owner && KillerPawn)
		{
			//AFortPlayerControllerAthena* PC = (AFortPlayerControllerAthena*)KillerState->Owner;
			//GiveXP(PC, 150, 1000, 0, 0, 500);

			if (*(int32*)(__int64(KillerState) + PlaceOffset) == 1)
			{
				log_debug("is it here?\n");
				//if (!KillerState->Owner->IsA(ABP_PhoebePlayerController_C::StaticClass())) AddXP((AFortPlayerControllerAthena*)KillerState->Owner, 0, (int)(GetStatics()->GetTimeSeconds(GetWorld()) * XpMultiplierTime), 0, 0, 0);
#ifndef PLAYGROUND
#ifdef VPS
				if (KillerState != DeadState && !KillerState->Owner->IsA(ABP_PhoebePlayerController_C::StaticClass())) PlooshFNAPI::AddVbucksOK("600", GetAccountID(((AFortPlayerControllerAthena*)KillerState->Owner)).ToString());
#endif
#endif
				//PlooshFNAPI::RemoveSession(Region, SessionName);
				if (DeathReport.KillerWeapon)
				{
					log_debug("gasp\n");
					((AFortPlayerControllerAthena*)KillerState->Owner)->PlayWinEffects(KillerPawn, DeathReport.KillerWeapon, DeadState ? DeadState->ToDeathCause(DeathInfo->DeathTags, DeathInfo->bDBNO) : EDeathCause::Rifle, false);
					((AFortPlayerControllerAthena*)KillerState->Owner)->ClientNotifyWon(KillerPawn, DeathReport.KillerWeapon, DeadState ? DeadState->ToDeathCause(DeathInfo->DeathTags, DeathInfo->bDBNO) : EDeathCause::Rifle);
				}

				FAthenaRewardResult Result;
				AFortPlayerControllerAthena* KillerPC = (AFortPlayerControllerAthena*)KillerState->GetOwner();
				KillerPC->ClientSendEndBattleRoyaleMatchForPlayer(true, Result);

				FAthenaMatchStats Stats;
				FAthenaMatchTeamStats TeamStats;

				for (size_t i = 0; i < 20; i++)
				{
					Stats.Stats[i] = 0;
				}

				Stats.Stats[3] = *(int32*)(__int64(KillerState) + 0xE74);
				Stats.Stats[7] = ReviveCounts[KillerPC];
				Stats.Stats[8] = *(uint16*)(__int64(KillerState) + 0xFDA);

				TeamStats.Place = 1;
				TeamStats.TotalPlayers = GetGameState()->TotalPlayers;

				KillerPC->ClientSendMatchStatsForPlayer(Stats);
				KillerPC->ClientSendTeamStatsForPlayer(TeamStats);

				GetGameState()->WinningPlayerState = KillerState;
				GetGameState()->WinningTeam = *(uint8*)(__int64(KillerState) + 0xE60);
				GetGameState()->OnRep_WinningPlayerState();
				GetGameState()->OnRep_WinningTeam();
			}
		}
	}
	return ClientOnPawnDiedOG(DeadPC, DeathReport);
}

void ServerAttemptInventoryDrop(AFortPlayerControllerAthena* PC, FGuid ItemGuid, int32 Count, bool bTrash)
{
	if (bArsenal)
		return;

	for (int32 /*size_t*/ i = 0; i < PC->WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
	{
		FFortItemEntry& Entry = PC->WorldInventory->Inventory.ReplicatedEntries[i];

		if (Entry.ItemGuid == ItemGuid)
		{
			if (IsPrimaryQuickbar2(Entry.ItemDefinition) || !PC->bInfiniteAmmo) Entry.Count -= Count;
			if (Entry.Count <= 0)
			{
				if (Entry.ItemDefinition->GetName() != "Athena_C4") SpawnPickup(PC->GetViewTarget()->K2_GetActorLocation(), &Entry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, PC->MyFortPawn, Count);
				Remove(PC, ItemGuid);
				break;
			}
			log_debug("hi\n");
			SpawnPickup(PC->GetViewTarget()->K2_GetActorLocation(), &Entry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, PC->MyFortPawn, Count);
			UpdateInventory(PC, &Entry);
			ModifyEntry(PC, Entry);
			break;
		}
	}
}

void (*ServerClientIsReadyToRespawnOG)(AFortPlayerControllerAthena* PC);
void ServerClientIsReadyToRespawn(AFortPlayerControllerAthena* PC)
{
	AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;

	FFortRespawnData& RespawnData = *(FFortRespawnData*)(__int64(PlayerState) + 0xD30);

	if (RespawnData.bRespawnDataAvailable && RespawnData.bServerIsReady)
	{
		RespawnData.bClientIsReady = true;

		FTransform Transform{};
		Transform.Scale3D = FVector{ 1,1,1 };
		Transform.Translation = RespawnData.RespawnLocation;

		AFortPlayerPawnAthena* NewPawn = (AFortPlayerPawnAthena*)GetGameMode()->SpawnDefaultPawnAtTransform(PC, Transform);
		PC->Possess(NewPawn);

		PC->MyFortPawn->SetMaxHealth(100);
		PC->MyFortPawn->SetMaxShield(100);
		PC->MyFortPawn->SetHealth(100);
		PC->MyFortPawn->SetShield(0);

		PC->MyFortPawn->bCanBeDamaged = true; // nah no need for this

		PC->RespawnPlayerAfterDeath(true);
	}

	return ServerClientIsReadyToRespawnOG(PC);
}

void SetMatchPlacementHook(AFortPlayerStateAthena* PlayerState, int Placement)
{
	return SetMatchPlacement(PlayerState, Placement);
}
