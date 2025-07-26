#pragma once
#include "pch.h"
#include "misc.h"
#include "Vehicles.h"
#include "VendingMachines.h"
#include "Lootllama.h"

bool InitListenFunc(void* Driver, void* World, FURL& url, bool reuse, FString& error)
{
	if (!InitListen(Driver, World, url, reuse, error))
	{
		if(RandomPort)
			Port = GetMath()->RandomIntegerInRange(1000, 65535);
		url.Port = Port;
		return InitListenFunc(Driver, World, url, reuse, error);
	}
}

void DadBroHealthTest()
{
	Sleep(10000);
	
	while (true)
	{
		if (!DadBroPawn || DadBroPawn->IsDead() || DadBroPawn->GetHealth() <= 0)
		{
			log_info("Won!");

			for (int32 /*size_t*/ i = 0; i < GetGameState()->PlayerArray.Num(); i++)
			{
				AFortPlayerStateAthena* PlayerState = (AFortPlayerStateAthena*)GetGameState()->PlayerArray[i];
				if (PlayerState)
				{
					*(int32*)(__int64(PlayerState) + 0xE6C) = 1;
					PlayerState->OnRep_Place();
				}
			}
			break;
		}
	}
}

void StartDadBro()
{
	UClass* DadBroPawnClass = StaticLoadObject<UClass>("/Game/Athena/DADBRO/DADBRO_Pawn.DADBRO_Pawn_C");
	AFortAthenaMutator_DadBro* Mutator = (AFortAthenaMutator_DadBro*)GetGameState()->GetMutatorByClass(GetGameMode(), AFortAthenaMutator_DadBro::StaticClass());
	Mutator->DadBroSpawnLocation.Z = -193.048096f;
	DadBroPawn = SpawnActor<AFortAIPawn>(DadBroPawnClass, Mutator->DadBroSpawnLocation);

	Mutator->DadBroPawn = DadBroPawn;
	Mutator->DadBroCodeState = EDadBroState::Active;
	Mutator->OnRep_DadBroPawn();
	Mutator->OnRep_DadBroCodeState();

	new thread(DadBroHealthTest);
}

bool (*ReadyToStartMatchOG)(AFortGameModeAthena* GameMode);
bool ReadyToStartMatch(AFortGameModeAthena* GameMode)
{
	ReadyToStartMatchOG(GameMode);
	static bool InitPlaylist = false;
	static bool Listening = false;
	static string CustomCode = "DevGame";

	if (Loading && Conv_NameToString(GameMode->MatchState).ToString() == "WaitingToStart") Ready = true;
	if (!Ready)
		return false;

	if (!InitPlaylist)
	{
		InitPlaylist = true;
		if (RandomPort)
			Port = GetMath()->RandomIntegerInRange(1000, 9999);
		SessionName += to_string(GetMath()->RandomInteger(2147483646));
		PlooshFNAPI::SetupAPI();
		//Playlist_Cobalt_Squads_P_Respawn
		//Playlist_Respawn_Op
		//AFortAthenaMutator_Infiltration
		//UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("Playlist_Cobalt_Squads_P_Respawn.Playlist_Cobalt_Squads_P_Respawn");
		//UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("Playlist_Respawn_Op.Playlist_Respawn_Op");
		//UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("Playlist_Mash_Squads.Playlist_Mash_Squads");
		//UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("Playlist_DADBRO_Squads.Playlist_DADBRO_Squads");
		//UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("Playlist_DefaultDuo.Playlist_DefaultDuo");
		//UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("Playlist_Fill_Solo.Playlist_Fill_Solo");
		//UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("Playlist_Bots_DefaultSolo.Playlist_Bots_DefaultSolo");
#ifdef ONE_SHOT
		// UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("Playlist_Low_Solo.Playlist_Low_Solo");
#endif
#ifndef ONE_SHOT
	//	UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("Playlist_DefaultSolo.Playlist_DefaultSolo");
#endif
		//UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("Playlist_Fill_Solo.Playlist_Fill_Solo");
		//UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("Playlist_DefaultSquad.Playlist_DefaultSquad");
		//UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("Playlist_BattleLab.Playlist_BattleLab");
		//UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("Playlist_Gg_Reverse.Playlist_Gg_Reverse");
		//UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("Playlist_Respawn_24.Playlist_Respawn_24");
		UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("Playlist_Music_High.Playlist_Music_High");
		//UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("Playlist_PlaygroundV2.Playlist_PlaygroundV2");
#ifdef PLAYGROUND
		//UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("Playlist_Playground.Playlist_Playground");
#endif
		if (!Playlist)
		{
			log_error("Invalid playlist!\n");
			while (true) {
				Sleep(1000);
			}
			return false;
		}


		GetGameState()->CurrentPlaylistInfo.BasePlaylist = Playlist;
		GetGameState()->CurrentPlaylistInfo.PlaylistReplicationKey++;
		GetGameState()->CurrentPlaylistInfo.MarkArrayDirty();
		GetGameState()->CurrentPlaylistId = Playlist->PlaylistId;
		
		//MaxPlayersPerTeam = 64;

		//MaxSquadSize = *(int32*)(__int64(Playlist) + GetOffset(Playlist, "MaxSquadSize"));
		//MaxPlayersPerTeam = *(int32*)(__int64(Playlist) + GetOffset(Playlist, "MaxTeamSize"));
		MaxPlayersPerTeam = *(int32*)(__int64(Playlist) + GetOffset(Playlist, "MaxSquadSize"));

		if (bLateGame || bSolos)
			MaxPlayersPerTeam = 1;

		NextIdx = *(uint8*)(__int64(Playlist) + GetOffset(Playlist, "DefaultFirstTeam"));
		
		bIsLargeTeamGame = *(bool*)(__int64(Playlist) + GetOffset(Playlist, "bIsLargeTeamGame"));
		FirstTeam = NextIdx;
		LastTeam = *(uint8*)(__int64(Playlist) + GetOffset(Playlist, "DefaultLastTeam"));
		bSkipWarmup = *(bool*)(__int64(Playlist) + GetOffset(Playlist, "bSkipWarmup"));
		bSkipAircraft = *(bool*)(__int64(Playlist) + GetOffset(Playlist, "bSkipAircraft"));

		GetGameState()->bGameModeWillSkipAircraft = bSkipAircraft;

		if (MaxPlayersPerTeam > 1)
		{
			GameMode->bDBNOEnabled = true;
			GameMode->bAlwaysDBNO = true;
			GetGameState()->bDBNODeathEnabled = true;
			GetGameState()->SetIsDBNODeathEnabled(true);
		}

		TSoftObjectPtr<UCompositeDataTable>& LootTierDataPtr = *(TSoftObjectPtr<UCompositeDataTable>*)(__int64(Playlist) + GetOffset(Playlist, "LootTierData"));
		TSoftObjectPtr<UCurveTable>& GameDataPtr = *(TSoftObjectPtr<UCurveTable>*)(__int64(Playlist) + GetOffset(Playlist, "GameData"));
		TSoftObjectPtr<UCompositeDataTable>& LootPackagesPtr = *(TSoftObjectPtr<UCompositeDataTable>*)(__int64(Playlist) + GetOffset(Playlist, "LootPackages"));
		bTravis = Playlist->GetName() == "Playlist_Music_High";
		LootTierData = StaticLoadObject<UCompositeDataTable>(Conv_NameToString(LootTierDataPtr.ObjectID.AssetPathName).ToString());
		GameData = StaticLoadObject<UCurveTable>(Conv_NameToString(GameDataPtr.ObjectID.AssetPathName).ToString());
		LootPackages = StaticLoadObject<UCompositeDataTable>(Conv_NameToString(LootPackagesPtr.ObjectID.AssetPathName).ToString());
		
		log_debug("MaxPlayersPerTeam %d, NextIdx %d, FirstTeam %d, LastTeam %d\n", MaxPlayersPerTeam, NextIdx, FirstTeam, LastTeam);
		log_debug("LootPackages %p, GameData %p, LootTierData %p\n", LootPackages, GameData, LootTierData);
		//Sleep(1000);
		wstring SessionNameWStr = wstring(SessionName.begin(), SessionName.end());
		AFortGameSessionDedicatedAthena* GameSession = SpawnActor<AFortGameSessionDedicatedAthena>({});
		*(int32*)(__int64(GameSession) + GetOffset(GameSession, "MaxPlayers")) = *(int32*)(__int64(Playlist) + GetOffset(Playlist, "MaxPlayers"));
		if (bLateGame)
			*(int32*)(__int64(GameSession) + GetOffset(GameSession, "MaxPlayers")) = 40;
		if (bTravis)
			*(int32*)(__int64(GameSession) + GetOffset(GameSession, "MaxPlayers")) = 100;
		GameSession->SessionName = Conv_StringToName(SessionNameWStr.c_str());
		GameMode->GameSession = GameSession;
		GameMode->FortGameSession = GameSession;
		GameMode->GameModeSessionString = SessionNameWStr.c_str();

		float TimeSeconds = GetStatics()->GetTimeSeconds(GetWorld());

		GetGameState()->WarmupCountdownEndTime = TimeSeconds + Duration;
		GameMode->WarmupCountdownDuration = Duration;
		GetGameState()->WarmupCountdownStartTime = TimeSeconds;
		GameMode->WarmupEarlyCountdownDuration = Duration;
		
		if (bTravis)
		{
			UClass* ClassTest = StaticLoadObject<UClass>("/CycloneJerky/Gameplay/BP_Jerky_Scripting.BP_Jerky_Scripting_C");
			log_debug("CT: %s\n", ClassTest->GetName().c_str());

			TSoftObjectPtr<UObject>& Test = *(TSoftObjectPtr<UObject>*)(__int64(ClassTest->DefaultObject) + GetOffset(ClassTest->DefaultObject, "CountdownTimer"));
			log_debug("Path Name: %s\n", Conv_NameToString(Test.ObjectID.AssetPathName).ToString().c_str());
			log_debug("Sub Path: %s\n", Test.ObjectID.SubPathString.ToString().c_str());
			TestObj = ((UKismetSystemLibrary*)UKismetSystemLibrary::StaticClass()->DefaultObject)->Conv_SoftObjectReferenceToObject(Test);

			log_debug("Ref: %s\n", TestObj->GetName().c_str());

			auto foundation = StaticFindObject<ABuildingFoundation>("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.LF_Athena_POI_19x19_2");
			CountdownTimer = StaticFindObject<UObject>("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.BP_Countdown_Child_Jerky_2");

			log_debug("Countdown: %s\n", CountdownTimer->GetName().c_str());

			ShowFoundation(foundation);

			TArray<AActor*> BuildingFoundations;
			GetStatics()->GetAllActorsOfClass(GetWorld(), ABuildingFoundation::StaticClass(), &BuildingFoundations);

			for (int32 /*size_t*/ i = 0; i < BuildingFoundations.Num(); i++)
			{
				ABuildingFoundation* Foundation = (ABuildingFoundation*)BuildingFoundations[i];

				if (Foundation && Foundation->GetFullName().contains("Jerky_Head"))
				{
					ShowFoundation(Foundation);
				}
			}
			BuildingFoundations.FreeArray();
		}
	}

	if (!GetGameState() || !GetGameState()->MapInfo)
	{
		if (!bCreative)
			return false;
	}

	if (!Listening)
	{
		Listening = true;
		JerkyLoader = UObject::FindObject<UObject>("BP_Jerky_Loader_C JerkyLoaderLevel.JerkyLoaderLevel.PersistentLevel.BP_Jerky_Loader_2");

		FName GameNetDriver = Conv_StringToName(TEXT("GameNetDriver"));
		UNetDriver* Driver = CreateNetDriver(GetEngine(), GetWorld(), GameNetDriver);
		log_debug("Driver: %s\n", Driver->GetName().c_str());
		Driver->World = GetWorld();
		Driver->NetDriverName = GameNetDriver;

		FString Error;
		FURL InURL = FURL();
		InURL.Port = Port;

		InitListen(Driver, GetWorld(), InURL, false, Error);
		SetWorld(Driver, GetWorld());

		GetWorld()->NetDriver = Driver;
		GetWorld()->LevelCollections[0].NetDriver = Driver;
		GetWorld()->LevelCollections[1].NetDriver = Driver;

		GameMode->bWorldIsReady = true;
		GetGameState()->AirCraftBehavior = GetGameState()->CurrentPlaylistInfo.BasePlaylist->AirCraftBehavior;
		GetGameState()->CachedSafeZoneStartUp = GetGameState()->CurrentPlaylistInfo.BasePlaylist->SafeZoneStartUp;
		GetGameState()->OnRep_CurrentPlaylistId();
		GetGameState()->OnRep_CurrentPlaylistInfo();
		UFortPlaylistAthena* Playlist = GetGameState()->CurrentPlaylistInfo.BasePlaylist;
		TArray<TSoftObjectPtr<UWorld>>& AdditionalLevels = *(TArray<TSoftObjectPtr<UWorld>>*)(__int64(Playlist) + GetOffset(Playlist, "AdditionalLevels"));
		TArray<TSoftObjectPtr<UWorld>>& AdditionalLevelsServerOnly = *(TArray<TSoftObjectPtr<UWorld>>*)(__int64(Playlist) + GetOffset(Playlist, "AdditionalLevelsServerOnly"));

		for (int32 /*size_t*/ i = 0; i < GetGameState()->MutatorListComponent->Mutators.Num(); i++)
		{
			log_debug("Mutator: %s\n", GetGameState()->MutatorListComponent->Mutators[i]->GetFullName().c_str());
		}

		if (!bLateGame) {
			UFortServerBotManagerAthena* BotManager = (UFortServerBotManagerAthena*)GetStatics()->SpawnObject(UFortServerBotManagerAthena::StaticClass(), GameMode);
			GameMode->ServerBotManager = BotManager;
			BotManager->CachedGameState = GetGameState();
			BotManager->CachedGameMode = GameMode;

			//UClass *PhoebeMutatorClass = StaticLoadObject<UClass>("/Game/Athena/AI/Phoebe/BP_Phoebe_Mutator.BP_Phoebe_Mutator_C");
			//log_debug("PhoebeMutatorClass: %p\n", PhoebeMutatorClass);
			//AFortAthenaMutator_Bots *PhoebeMutator = (AFortAthenaMutator_Bots *) GetGameState()->GetMutatorByClass(GetGameMode(), PhoebeMutatorClass);

		
			BotMutator = SpawnActor<AFortAthenaMutator_Bots>({});
			//BotMutator = PhoebeMutator;
			//BotMutator = SpawnActor<AFortAthenaMutator_Bots>(PhoebeMutatorClass, {});
			BotManager->CachedBotMutator = BotMutator;
			BotMutator->CachedGameMode = GameMode;
			BotMutator->CachedGameState = GetGameState();

			AAthenaAIDirector* Director = SpawnActor<AAthenaAIDirector>({});
			GameMode->AIDirector = Director;
			Director->Activate();

			AFortAIGoalManager* GoalManager = SpawnActor<AFortAIGoalManager>({});
			GameMode->AIGoalManager = GoalManager;

			BotsEnabled = BotMutator;
		}
		log_info("Streaming Playlist AdditionalLevels\n");
		if (!bLateGame) {
			for (int32 /*size_t*/ i = 0; i < AdditionalLevelsServerOnly.Num(); i++)
			{
				FVector Loc{};
				FRotator Rot{};
				bool Success = false;
				((ULevelStreamingDynamic*)ULevelStreamingDynamic::StaticClass()->DefaultObject)->LoadLevelInstanceBySoftObjectPtr(GetWorld(), AdditionalLevelsServerOnly[i], Loc, Rot, &Success, FString());
				FAdditionalLevelStreamed NewLevel{};
				NewLevel.bIsServerOnly = true;
				NewLevel.LevelName = AdditionalLevelsServerOnly[i].ObjectID.AssetPathName;
				GetGameState()->AdditionalPlaylistLevelsStreamed.Add(NewLevel);
				log_debug("Loaded server-only %s\n", AdditionalLevelsServerOnly[i].ObjectID.AssetPathName.ToString().c_str());
			}
		}

		for (int32 /*size_t*/ i = 0; i < AdditionalLevels.Num(); i++)
		{
			FVector Loc{};
			FRotator Rot{};
			bool Success = false;
			((ULevelStreamingDynamic*)ULevelStreamingDynamic::StaticClass()->DefaultObject)->LoadLevelInstanceBySoftObjectPtr(GetWorld(), AdditionalLevels[i], Loc, Rot, &Success, FString());
			FAdditionalLevelStreamed NewLevel{};
			NewLevel.bIsServerOnly = false;
			NewLevel.LevelName = AdditionalLevels[i].ObjectID.AssetPathName;
			GetGameState()->AdditionalPlaylistLevelsStreamed.Add(NewLevel);
			log_debug("Loaded %s\n", AdditionalLevels[i].ObjectID.AssetPathName.ToString().c_str());
		}

		GetGameState()->OnRep_AdditionalPlaylistLevelsStreamed();
		GetGameState()->OnFinishedStreamingAdditionalPlaylistLevel();
		log_debug("Finished streaming additional levels\n");

		bArsenal = Playlist->GetName() == "Playlist_Gg_Reverse";
		bStormKing = Playlist->GetName() == "Playlist_DADBRO_Squads";
		bBattleLab = Playlist->GetName() == "Playlist_BattleLab";
		bDropZone = Playlist->GetName() == "Playlist_Respawn_Op";
		bTeamRumble = Playlist->GetName() == "Playlist_Respawn_24";

		bEnableLooting = !bArsenal && !bStormKing && !bLateGame && !bDropZone && !bTravis;
		bEnableScoringSystem = bArsenal || bTeamRumble || bDropZone;

		bEnableSiphon = bLateGame || bArsenal;

		if (bEnableLooting) {
			InitLooting();
			SpawnFloorLoot();
		}
		if (!bTravis) SpawnVehicles();

		//string cmd = "demorec " + SessionName;
		//((UKismetSystemLibrary*)UKismetSystemLibrary::StaticClass()->DefaultObject)->ExecuteConsoleCommand(GetWorld(), wstring(cmd.begin(), cmd.end()).c_str(), nullptr);
		// backend doesn't support this yet

		TArray<AActor*> BuildingContainers;

		GetStatics()->GetAllActorsOfClass(GetWorld(), ABuildingContainer::StaticClass(), &BuildingContainers);

		
		
		for (int32 /*size_t*/ i = 0; i < BuildingContainers.Num(); i++)
		{
			ABuildingContainer* Container = (ABuildingContainer*)BuildingContainers[i];
			if (bEnableLooting) {
				string LootTierGroup = Conv_NameToString(Container->SearchLootTierGroup).ToString();

				log_info("LootTierGroup: %s\n", LootTierGroup.c_str());

				if (LootTierGroup == "Loot_Treasure")
				{
					LootTierGroup = "Loot_AthenaTreasure";
				}
				else if (LootTierGroup == "Loot_Ammo")
				{
					LootTierGroup = "Loot_AthenaAmmoLarge";
				}

				Container->SearchLootTierGroup = Conv_StringToName(wstring(LootTierGroup.begin(), LootTierGroup.end()).c_str());
			}
			else if (!bLateGame) {
				Container->K2_DestroyActor();
			}
		}

		if (bEnableLooting) FillVendingMachines();
		else {
			TArray<AActor*> VendingMachinesArray;

			GetStatics()->GetAllActorsOfClass(GetWorld(), UObject::FindObject<UClass>("BlueprintGeneratedClass B_Athena_VendingMachine.B_Athena_VendingMachine_C"), &VendingMachinesArray);

			for (int32 /*size_t*/ i = 0; i < VendingMachinesArray.Num(); i++)
			{
				VendingMachinesArray[i]->K2_DestroyActor();
			}

			VendingMachinesArray.FreeArray();

			log_info("Destroyed Vending Machines\n");
		}

		log_debug("Finished setting LootTierGroups, freeing array\n");
		BuildingContainers.FreeArray();

		if (bEnableLooting) SpawnLlamas();
		
		//GetGameState()->EventTournamentRound = EEventTournamentRound::Arena;
		//GetGameState()->OnRep_EventTournamentRound();
		// 
		log_info("Server is open on port %d\n", Port);
		
		SetConsoleTitleA(("Port: " + to_string(Port)).c_str());

		/*UFortAthenaAIBotCustomizationData* customization = StaticLoadObject<UFortAthenaAIBotCustomizationData>("/Game/Athena/AI/MANG/BotData/BotData_MANG_POI_HMW_Alter");
		FFortAthenaAIBotRunTimeCustomizationData runtimeData{};
		runtimeData.CustomSquadId = 0;

		FRotator RandomYawRotator{};
		RandomYawRotator.Yaw = (float)rand() * 0.010986663f;

		auto Meowscles = SpawnBot(GetGameMode()->ServerBotManager, { 179899, -176186, -2611 }, RandomYawRotator, customization, runtimeData);
		Meowscles->SetMaxShield(400);
		Meowscles->SetShield(400);*/

		//SpawnBotsAtPlayerStarts();

		//PlooshFNAPI::MarkServerOnline(Region, to_string(GameMode->GameSession->MaxPlayers), to_string(Port), SessionName, "Playlist_DADBRO_Squads");
		if (!CustomCode.contains("DevGame") && bLateGame) {
			//Region == "EU" ? CustomCode = "LateGame" : CustomCode = "LateGame2";
			CustomCode = "LateGame"; // Forced This But Should Be LateGame and LateGame2 on both (if wanting 3 games then LateGame3 ofc)

			/* Devers
			DevGame
			DevGame2
			DevGame3
			*/
		}
		//PlooshFNAPI::MarkServerOnline(Region, to_string(GameMode->GameSession->MaxPlayers), to_string(Port), SessionName, "Playlist_DefaultSolo", CustomCode);
		
		// test new MMS


		// if notify mms AND it's NOT a staff game
		if (!bDontNotifyMMS || !bStaffGameOnly) {
			PlooshFNAPI::SetupAPI();
			bool as = PlooshMMSAPI::MarkServerOnlinev2(Region, to_string(GameMode->GameSession->MaxPlayers), to_string(Port), SessionName, GetGameState()->CurrentPlaylistInfo.BasePlaylist->PlaylistName.ToString(), CustomCode);

			if (as == true) {
				log_debug("Server marked online on MMS\n");
			}

			PlooshFNAPI::MarkServerOnline(Region, to_string(GameMode->GameSession->MaxPlayers), to_string(Port), SessionName, GetGameState()->CurrentPlaylistInfo.BasePlaylist->PlaylistName.ToString(), CustomCode);

			/*bool ab = PlooshMMSAPI::SetServerStatus("online");

			if (ab == true) {
				log_debug("Server status set to online on MMS\n");
			}*/
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
				curl_easy_setopt(curl, CURLOPT_URL, "https://backend.ploosh.dev:2053/gs/create");

				curl_slist* headers = curl_slist_append(NULL, "Content-Type: application/json");
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			}
			else {
				std::cerr << "Error: curl_easy_init() returned NULL pointer" << '\n';
			}
			
			auto p = GetGameState()->CurrentPlaylistInfo.BasePlaylist->PlaylistName.ToString();

			std::string json = "{\"region\":\"" + Region + "\", \"ip\": \"" + VPS_IP + "\", \"port\": \"" + to_string(Port) + "\", \"playlist\": \"" + p + "\",\"version\":\"12.41\"}";
			// std::cout << "json: " << json << '\n';
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

			curl_easy_perform(curl);
		}

		
		//PlooshFNAPI::MarkServerOnlinev2(Region, to_string(GameMode->GameSession->MaxPlayers), to_string(Port), SessionName, "Playlist_DefaultSolo", CustomCode);
		//PlooshFNAPI::SetServerStatus("online");
		//PlooshFNAPI::MarkServerOnline(Region, to_string(GameMode->GameSession->MaxPlayers), to_string(Port), SessionName, GetGameState()->CurrentPlaylistInfo.BasePlaylist->PlaylistName.ToString(), CustomCode);
		//static auto OverrideBattleBusSkin = StaticFindObject<UAthenaBattleBusItemDefinition>("/Game/Athena/Items/Cosmetics/BattleBuses/BBID_DonutBus.BBID_DonutBus");
		//GetGameState()->DefaultBattleBus = OverrideBattleBusSkin;
	}

	if (GetGameState()->PlayersLeft > 0)
	{
		return true;
	}
	else
	{
		float TimeSeconds = GetStatics()->GetTimeSeconds(GetWorld());

		GetGameState()->WarmupCountdownEndTime = TimeSeconds + Duration;
		GameMode->WarmupCountdownDuration = Duration;
		GetGameState()->WarmupCountdownStartTime = TimeSeconds;
		GameMode->WarmupEarlyCountdownDuration = Duration;
	}

	return false;
}

APawn* SpawnDefaultPawnFor(AFortGameMode* GM, AFortPlayerController* Player, AActor* StartSpot)
{
	FTransform transform = StartSpot->GetTransform();
	AFortPlayerPawnAthena* Ret = (AFortPlayerPawnAthena*)GM->SpawnDefaultPawnAtTransform(Player, transform);
	return Ret;
}

#include "PlooshAI.h"

void (*OnAircraftExitedDropZoneOG)(AFortGameModeAthena* GM, AFortAthenaAircraft* Aircraft);
void OnAircraftExitedDropZone(AFortGameModeAthena* GM, AFortAthenaAircraft* Aircraft)
{
	if (bLateGame || bTravis)
	{
		for (int32 /*size_t*/ i = 0; i < GM->AlivePlayers.Num(); i++)
		{
			if (GM->AlivePlayers[i] && GM->AlivePlayers[i]->IsInAircraft())
			{
				GM->AlivePlayers[i]->GetAircraftComponent()->ServerAttemptAircraftJump(FRotator());
			}
		}
	}
	
	OnAircraftExitedDropZoneOG(GM, Aircraft);

	if (bTravis/*false*/)
	{
		UFunction* StartEventFunc = JerkyLoader->Class->GetFunction("BP_Jerky_Loader_C", "startevent");
		log_debug("Start event: %s\n", StartEventFunc->GetName().c_str());
		float idk = 0.f;
		ProcessEvent(JerkyLoader, StartEventFunc, &idk);
		log_info("Started Astronomical");
	}

	if (bStormKing)
	{
		StartDadBro();
	}
}

void(__fastcall* PreloginOG)(AFortGameModeAthena* GameMode, FString& Options, FString& Address, FUniqueNetIdRepl& UniqueId, FString& ErrorMessage);
void __fastcall Prelogin(AFortGameModeAthena* GameMode, FString& Options, FString& Address, FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	return;
	if (bBattleLab)
		return;

	return PreloginOG(GameMode, Options, Address, UniqueId, ErrorMessage);
}

__int64 StartAircraftPhaseHook(AFortGameModeAthena* GameMode, char a2)
{
	log_debug("%s\n", to_string(a2).c_str());
	/*if (!bTravis && !bLateGame) {
		SpawnBotsAtPlayerStarts();
	}*/
	/*for (size_t i = 0; i < PlayerBots.size(); i++) {
		auto bot = PlayerBots[i];
		AFortPlayerPawn* closestPawn = nullptr;
		float closestDistance = 0;
		bool distanceSet = false;
		log_debug("st\n");

		for (int32 /*size_t* / z = 0; z < GM->AlivePlayers.Num(); z++)
		{
			auto PPC = GM->AlivePlayers[z];
			if (PPC && PPC->MyFortPawn)
			{
				log_debug("plooshai player\n");
				float Distance = bot->Pawn->GetDistanceTo(PPC->MyFortPawn);
				if (!distanceSet) {
					log_debug("Distance set\n");
					distanceSet = true;
					closestDistance = Distance;
					closestPawn = PPC->MyFortPawn;
					//continue;
				}
				else {
					if (Distance < closestDistance) {
						closestDistance = Distance;
						closestPawn = PPC->MyFortPawn;
					}
				}
			}
		}
		/*for (size_t i = 0; i < PlayerBots.size(); i++)
		{
			auto PPC = PlayerBots[i];
			if (PPC && PPC->Pawn)
			{
				float Distance = bot->Pawn->GetDistanceTo(PPC->Pawn);
				if (!distanceSet) {
					distanceSet = true;
					closestDistance = Distance;
					closestPawn = PPC->Pawn;
					continue;
				}
				else {
					if (Distance < closestDistance) {
						closestDistance = Distance;
						closestPawn = PPC->Pawn;
					}
				}
			}
		}* /

		if (closestPawn && distanceSet) {
			log_debug("Targeting %s\n", closestPawn->GetName().c_str());
			bot->FollowTarget = closestPawn;
		}
	}*/
	//auto ret = StartAircraftPhase(GM, a2);
	//if (BotsEnabled)
	//	((UKismetSystemLibrary*)UKismetSystemLibrary::StaticClass()->DefaultObject)->ExecuteConsoleCommand(GetWorld(), TEXT("startaircraft"), nullptr);//1:1
	//return ret;
	return StartAircraftPhase(GameMode, a2);
}
