#pragma once
#include "Engine.h"
#include "PlooshAI.h"
#include "Looting.h"
#include "pch.h"
#include "XP.h"
#include "Utils.h"
#include "Backend.h"
#include <Windows.h>
#include "VendingMachines.h"

UFortWorldItem* GetPickaxeInstance(AFortInventory* inv)
{
    static auto FortWeaponMeleeItemDefinitionClass = StaticFindObject<UClass>("/Script/FortniteGame.FortWeaponMeleeItemDefinition");

    auto& ItemInstances = inv->Inventory.ItemInstances;

    for (int i = 0; i < ItemInstances.Num(); ++i)
    {
        auto ItemInstance = ItemInstances[i];

        if (/*ItemInstance->ItemEntry && */ItemInstance->ItemEntry.ItemDefinition != nullptr &&
            ItemInstance->ItemEntry.ItemDefinition->IsA(FortWeaponMeleeItemDefinitionClass)
            )
        {
            return ItemInstance;
        }
    }

    return nullptr;
}

UFortWorldItem* FindItemInstance(AFortInventory* inv, UFortItemDefinition* ItemDefinition)
{
    auto& ItemInstances = inv->Inventory.ItemInstances;

    for (int i = 0; i < ItemInstances.Num(); i++)
    {
        auto ItemInstance = ItemInstances[i];

        if (ItemInstance->ItemEntry.ItemDefinition == ItemDefinition)
            return ItemInstance;
    }

    return nullptr;
}

UFortWorldItem* FindItemInstance(AFortInventory* inv, const FGuid& Guid)
{
    auto& ItemInstances = inv->Inventory.ItemInstances;

    for (int i = 0; i < ItemInstances.Num(); i++)
    {
        auto ItemInstance = ItemInstances[i];

        if (ItemInstance->ItemEntry.ItemGuid == Guid)
            return ItemInstance;
    }

    return nullptr;
}

void LockServer() {
    PlooshFNAPI::LockGameSession(Region, SessionName, GetGameMode()->AlivePlayers.Num());

    if (!bDontNotifyMMS || !bStaffGameOnly) {
        #ifndef PLAYGROUND
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
        #endif
    }

    ExitThread(0);
}

void (*TickFlushOG)(UNetDriver* Driver, float a2);
void TickFlushHook(UNetDriver* Driver, float a2)
{
    static bool Done = false;
    static bool Done2 = false;
    if (!Driver)
        return;

    if (!Done)
    {
        if (GetGameState()->WarmupCountdownEndTime - GetStatics()->GetTimeSeconds(GetWorld()) <= 10)
        {
            Done = true;

            CreateThread(0, 0, (LPTHREAD_START_ROUTINE) LockServer, 0, 0, 0);
        }
    }

    //if (BotsEnabled)
    {
        if (!Done2)
        {
            if (GetGameState()->WarmupCountdownEndTime - GetStatics()->GetTimeSeconds(GetWorld()) <= 0 /*GetGameState()->WarmupCountdownEndTime - GetStatics()->GetTimeSeconds(GetWorld()) <= 113 && GetGameMode()->AlivePlayers.Num() > 0*/)
            {
                Done2 = true;
                //if (bEnableLooting) spawnMeowscles();

                StartAircraftPhase(GetGameMode(), 0);
            }
        }

        if (BotsEnabled) TickBots();
    }

    if (Driver && Driver->ReplicationDriver && Driver->ClientConnections.Num() > 0 && !Driver->ClientConnections[0]->InternalAck)
        ServerReplicateActors(Driver->ReplicationDriver);

    return TickFlushOG(Driver, a2);
}

char __fastcall KickPlayerHook(__int64, __int64, __int64)
{
    return 0;
}

__int64 WorldNetMode(UWorld* World)
{
    return 1;
}

__int64(__fastcall* MCP_DispatchRequest)(void* McpProfile, void* Context, int a3);
__int64 __fastcall MCP_DispatchRequestHook(void* McpProfile, void* Context, int a3)
{
    return MCP_DispatchRequest(McpProfile, Context, bMcp ? 3 : a3);
}
void __fastcall CollectGarbage(unsigned int a1, unsigned __int8 a2)
{
    return;
}

Loadout& GetRandomLoadout()
{
    static /*int*/int64 Wow = 0;
    static bool Once = false;
    if (!Once)
    {
        Once = true;
        Wow = GetMath()->RandomInteger64InRange /*RandomIntegerInRange*/(0, Loadouts.size() - 1);
    }
    Loadout& loadout = Loadouts[Wow];
    Wow++;
    if (Wow == Loadouts.size())
        Wow = 0;
    return loadout;
}

void SwapVTable(void* base, int Idx, void* Detour, void** OG = nullptr)
{
    if (!base)
        return;

    void** VTable = *(void***)base;
    if (!VTable || !VTable[Idx])
        return;
    if (OG)
    {
        *OG = VTable[Idx];
    }

    log_debug("Addr: 0x%llx\n", (__int64(VTable) + (Idx * 8)) - __int64(GetModuleHandleA(0)));

    DWORD oldProtection;

    VirtualProtect(&VTable[Idx], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtection);

    VTable[Idx] = Detour;

    VirtualProtect(&VTable[Idx], sizeof(void*), oldProtection, NULL);
}

std::map<string, long long> VTCache;

void SwapVTable(void* base, string Path, void* Detour, void** OG = nullptr)
{
    if (!base)
        return;
    
    long long Idx;
    if (!(Idx = VTCache[Path])) {
        auto Function = UObject::FindObject<UFunction>(Path);
        if (!Function) {
            log_error("Failed to find %s!\n", Path.c_str());
            while (!Function) {
                Sleep(1000);
                Function = UObject::FindObject<UFunction>(Path);
            }
        }
        Idx = VTableOff(Function);
        log_debug("VTable: %p, Idx: %llx, Func: %s\n", base, Idx, Path.c_str());
        VTCache[Path] = Idx;
    }

    void** VTable = *(void***)base;
    if (!VTable || !VTable[Idx])
        return;
    if (OG)
    {
        *OG = VTable[Idx];
    }

    log_debug("Addr: 0x%llx\n", (__int64(VTable) + (Idx * 8)) - __int64(GetModuleHandleA(0)));

    DWORD oldProtection;

    VirtualProtect(&VTable[Idx], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtection);

    VTable[Idx] = Detour;

    VirtualProtect(&VTable[Idx], sizeof(void*), oldProtection, NULL);
}

/*void SwapVTable(void* base, string Path, void* Detour, void** OG = nullptr)
{
    if (!base)
        return;
    auto Function = UObject::FindObject<UFunction>(Path);
    if (!Function) {
        log_error("Failed to find %s!\n", Path.c_str());
        while (!Function) {
            Sleep(1000);
            Function = UObject::FindObject<UFunction>(Path);
        }
    }
    auto Idx = VTableOff(Function);
    log_debug("Idx: %llx, Func: %s\n", Idx, Path.c_str());

    void** VTable = *(void***)base;
    if (!VTable || !VTable[Idx])
        return;
    if (OG)
    {
        *OG = VTable[Idx];
    }

    log_debug("Addr: 0x%llx\n", (__int64(VTable) + (Idx * 8)) - __int64(GetModuleHandleA(0)));

    DWORD oldProtection;

    VirtualProtect(&VTable[Idx], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtection);

    VTable[Idx] = Detour;

    VirtualProtect(&VTable[Idx], sizeof(void*), oldProtection, NULL);
}*/

void ServerChangeName(AFortPlayerControllerAthena* PC, FString NewName) {
    string IP = PC->PlayerState->SavedNetworkAddress.ToString();
    log_info("Banned %s (%s).\n", PC->PlayerState->GetPlayerName().ToString().c_str(), IP.c_str());
    PC->MyFortPawn->BalloonActiveCount = 255;
    new thread(PlooshFNAPI::BanPlayer, GetAccountID(PC).ToString());
    PC->ClientReturnToMainMenu(L"You have been banned from Astro. Please go to https://discord.gg/StWVWn6a3B to appeal.");
}

float GetMaxTickRate(UGameEngine* Engine, float a2, bool a3)
{
    return 30.f;
}

BYTE* __fastcall GameSessionIdThing(__int64, __int64)
{
    return nullptr;
}

__int64 ActorNetMode(AActor* Actor)
{
    return 1;
}

EFortTeam PickTeam(AFortGameModeAthena* Gamemode, uint8 PreferredTeam, AFortPlayerControllerAthena* PC)
{
    if (PC->IsA(AFortLiveBroadcastController::StaticClass()) || GetGameState()->GamePhase >= EAthenaGamePhase::Aircraft)
        return EFortTeam::Spectator;

    uint8 RetVal = NextIdx;

    if (!bIsLargeTeamGame)
    {
        CurrentPlayersOnTeam++;

        if (CurrentPlayersOnTeam >= MaxPlayersPerTeam)
        {
            log_debug("MAX\n");
            CurrentPlayersOnTeam = 0;
            NextIdx++;
        }
    }
    else
    {
        RetVal = NextIdx;
        NextIdx++;
        log_debug("NextIdx %d\n", NextIdx);
        if (NextIdx > FirstTeam + 1)
        {
            log_debug("NextIdx > FirstTeam + 1");
            NextIdx = FirstTeam;
        }
    }

    log_info("Selected team %d\n", RetVal);
    return EFortTeam(RetVal);
}

FName SpecName;
void (*GetPlayerViewPointOG)(AFortPlayerController* PC, FVector& out_Location, FRotator& out_Rotation);
void GetPlayerViewPoint(AFortPlayerController* PC, FVector& out_Location, FRotator& out_Rotation)
{
    if (PC->IsA(AFortLiveBroadcastController::StaticClass()))
        return GetPlayerViewPointOG(PC, out_Location, out_Rotation);

    if (PC->StateName.ComparisonIndex == SpecName.ComparisonIndex)
    {
        out_Location = PC->LastSpectatorSyncLocation;
        out_Rotation = PC->LastSpectatorSyncRotation;
    }
    else if (PC->GetViewTarget())
    {
        out_Location = PC->GetViewTarget()->K2_GetActorLocation();
        out_Rotation = PC->GetControlRotation();
    }
    else
    {
        return GetPlayerViewPointOG(PC, out_Location, out_Rotation);
    }
}

__int64 __fastcall context(__int64 a1, __int64 a2, __int64* a3, __int64 a4)
{
    return 1;
}

void TESTETSETSTS()
{
    return;
}

__int64(__fastcall* test3OG)(AActor* a1, __int64 a2);
__int64 __fastcall test3(AActor* a1, __int64 a2)
{
    AFortInventory* NewInv = SpawnActor<AFortInventory>({});
    NewInv->SetOwner(a1);

    *(unsigned long long*)(__int64(a1) + 1352) = __int64(NewInv);
    return test3OG(a1, a2);
}

__int64 CanCreateContext(__int64 a1)
{
    return 1;
}

int TimesCalled = 0;

int GetStormDuration()
{
    int Ret = 30;

    if (bStormKing)
    {
        switch (TimesCalled)
        {
        case 0:
            Ret = 30;
            break;

        case 1:
            Ret = 180;
            break;

        case 2:
            Ret = 300;
            break;

        case 3:
            Ret = 360;
            break;

        default:
            break;
        }
    }
    else
    {
        switch (TimesCalled)
        {
        case 0:
            Ret = 0/*.f*/;
            break;

        case 1:
            Ret = 60;
            break;

        case 2:
            Ret = 30;
            break;

        case 3:
            Ret = 15;
            break;

        default:
            break;
        }
    }

    return Ret;
}

float GetZoneStartDuration()
{
    if (bLateGame)
    {
        switch (TimesCalled)
        {
        case 0:
        {
            return 0.f;
        }

        case 1:
        {
            return 60;
        }

        case 2:
            return 60;

        case 3:
            return 60;

        default:
            return 0.f;
        }
    }
    return 0.f; /*breaking?*/
}

void(__fastcall* stormstuffOG)(__int64 a1, int a2);
void __fastcall stormstuff(__int64 a1, int a2)
{
    if (bTravis && !StartedEvent)
        return;

    stormstuffOG(a1, a2);

    if (bLateGame)
    {
        GetGameState()->SafeZoneIndicator->NextCenter = (FVector_NetQuantize100)BusLocation;
        GetGameState()->SafeZoneIndicator->NextNextCenter = (FVector_NetQuantize100)BusLocation;

        switch (TimesCalled)
        {
        case 0:
        {
            GetGameState()->SafeZoneIndicator->NextRadius = 10000;
            break;
        }

        case 1:
        {
            GetGameState()->SafeZoneIndicator->NextRadius = 7500;
            break;
        }

        case 2:
            GetGameState()->SafeZoneIndicator->NextRadius = 5000;
            break;

        case 3:
            GetGameState()->SafeZoneIndicator->NextRadius = 3500;
            break;

        default:
            GetGameState()->SafeZoneIndicator->NextRadius = 0;
            break;
        }

        FVector Center = GetGameState()->SafeZoneIndicator->GetSafeZoneCenter();
        FVector NextCenter = GetGameState()->SafeZoneIndicator->GetSafeZoneCenter();

        if (TimesCalled)
        {
            int Amount = GetMath()->RandomIntegerInRange(-25000, 25000);
            NextCenter.X += Amount;
            NextCenter.Y += Amount;
            GetGameState()->SafeZoneIndicator->NextCenter = (FVector_NetQuantize100)NextCenter;
        }

        int StormDuration = GetStormDuration();
        float Distance = GetMath()->Vector_Distance2D(Center, NextCenter);

        if (Distance >= 10000)
            StormDuration += 85;

        log_info("Distance: %f\n", Distance);

        GetGameState()->SafeZoneIndicator->SafeZoneStartShrinkTime = GetStatics()->GetTimeSeconds(GetWorld()) + GetZoneStartDuration();
        GetGameState()->SafeZoneIndicator->SafeZoneFinishShrinkTime = GetGameState()->SafeZoneIndicator->SafeZoneStartShrinkTime + StormDuration;
        GetGameState()->SafeZonePhase = 5;
        GetGameMode()->SafeZonePhase = 5;
        GetGameState()->OnRep_SafeZonePhase();
        TimesCalled++;
    }

    if (bStormKing && GetGameState()->GamePhase > EAthenaGamePhase::Aircraft)
    {
        AFortAthenaMutator_DadBro* Mutator = (AFortAthenaMutator_DadBro*)GetGameState()->GetMutatorByClass(GetGameMode(), AFortAthenaMutator_DadBro::StaticClass());
        GetGameState()->SafeZoneIndicator->NextCenter = (FVector_NetQuantize100)Mutator->DadBroSpawnLocation;
        GetGameState()->SafeZoneIndicator->NextNextCenter = (FVector_NetQuantize100)Mutator->DadBroSpawnLocation;

        switch (TimesCalled)
        {
        case 0:
            GetGameState()->SafeZoneIndicator->NextRadius = 10000;
            break;

        case 1:
            GetGameState()->SafeZoneIndicator->NextRadius = 7500;
            break;

        case 2:
            GetGameState()->SafeZoneIndicator->NextRadius = 5000;
            break;

        case 3:
            GetGameState()->SafeZoneIndicator->NextRadius = 4750;
            break;

        default:
            GetGameState()->SafeZoneIndicator->NextRadius = 4350;
            break;
        }

        GetGameState()->SafeZoneIndicator->SafeZoneStartShrinkTime = GetStatics()->GetTimeSeconds(GetWorld());
        GetGameState()->SafeZoneIndicator->SafeZoneFinishShrinkTime = GetStatics()->GetTimeSeconds(GetWorld()) + GetStormDuration();
        GetGameState()->SafeZonePhase = 5;
        GetGameMode()->SafeZonePhase = 5;
        GetGameState()->OnRep_SafeZonePhase();
        TimesCalled++;
    }
}

void GameSessionRestart(AFortGameSessionDedicated* Session)
{
    PlooshFNAPI::RemoveSession(Region, SessionName);
    //PlooshMMSAPI::SetServerStatus("offline");
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
    //((UKismetSystemLibrary*)UKismetSystemLibrary::StaticClass()->DefaultObject)->ExecuteConsoleCommand(GetWorld(), TEXT("demostop"), nullptr);
    exit(0);
}

__int64 uelog(__int64, unsigned int, __int64, unsigned __int8, wchar_t* Format, ...)
{
    log_info("%ls\n", Format);
    return 0;
}

void InitXpComp(UFortPlayerControllerAthenaXPComponent* XPComponent, AFortPlayerStateAthena* PlayerState)
{
    XPComponent->bRegisteredWithQuestManager = true;
    XPComponent->OnRep_bRegisteredWithQuestManager();
    *(int32*)(__int64(PlayerState) + 0xE78) = XPComponent->CurrentLevel;
    PlayerState->OnRep_SeasonLevelUIDisplay();

    // the function is called OnProfileReady but magma don't we need to call the ORigianl First??
}

//literally the most 1:1 thing ever
APlayerController* (*SpawnPlayActorOG)(UWorld* World, UPlayer* Player, ENetRole RemoteRole, FURL& InURL, void* UniqueID, FString& Error, uint8 InNetPlayerIndex);
APlayerController* SpawnPlayActor(UWorld* World, UPlayer* Player, ENetRole RemoteRole, FURL& InURL, void* UniqueID, FString& Error, uint8 InNetPlayerIndex)
{
    FString& JoinURL = *(FString*)(__int64(Player) + 0x1B8);
    if (JoinURL.ToString().contains("livespectator"))
    {
        GetGameMode()->PlayerControllerClass = GetGameMode()->LiveBroadcastPlayerControllerClass.Get();
    }
    else
    {
        GetGameMode()->PlayerControllerClass = AAthena_PlayerController_C::StaticClass();
    }
    auto Ret = SpawnPlayActorOG(World, Player, RemoteRole, InURL, UniqueID, Error, InNetPlayerIndex);

    if (GetGameMode()->PlayerControllerClass == AAthena_PlayerController_C::StaticClass() && GetGameState()->GamePhase >= EAthenaGamePhase::Aircraft)
    {
        //Ret->ClientReturnToMainMenu(TEXT(""));
        return Ret;
    }

    return Ret;
}

__int64 (*OnExplodedOG)(AB_Prj_Athena_Consumable_Thrown_C* Consumable, TArray<class AActor*>& HitActors, TArray<struct FHitResult>& HitResults) = decltype(OnExplodedOG)(__int64(GetModuleHandleA(0)) + 0x3EA2740);
__int64 OnExploded(AB_Prj_Athena_Consumable_Thrown_C* Consumable, /*unsigned __int64* a2, char* a3*/ TArray<class AActor*>& HitActors, TArray<struct FHitResult>& HitResults /*, __int64 a4, __int64 a5*/ /*ida is telling me a4 and a5 exist*/)
{
    if (!Consumable)
        return OnExplodedOG(Consumable, HitActors, HitResults); // 0
    log_info("%s exploded\n", Consumable->GetFullName().c_str());
    //if (Consumable->GetName().contains("B_Prj_Athena_AppleSun_")) return OnExplodedOG(Consumable, HitActors, HitResults);
    if (Consumable->GetName() == "B_Prj_Lotus_Mustache_C") {
        /*auto PC = Consumable->GetOwnerPlayerController();
        auto Pawn = PC->MyFortPawn;
        float Distance = Consumable->GetDistanceTo(Pawn);

        if (Distance <= 250) {
            auto NewHealth = (Pawn->GetHealth() + 15) >= 100 ? 100 : (Pawn->GetHealth() + 15);
            Pawn->SetHealth(NewHealth);
        }
        else {*/
            SpawnPickup(Consumable->K2_GetActorLocation(), StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/Bandage/Athena_Bandage.Athena_Bandage"), 1, 0, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset, nullptr, false);
        //}
    } else if (Consumable->GetName() == "B_Prj_Athena_Bucket_Old_C" || Consumable->GetName() == "B_Prj_Athena_Bucket_Nice_C") {
        auto PC = Consumable->GetOwnerPlayerController();
        auto Pawn = PC->MyFortPawn;
        auto Def = *(UFortItemDefinition**)(__int64(Consumable) + 0x888);
        if (!Def)
            return OnExplodedOG(Consumable, HitActors, HitResults); // 0

        //SpawnPickup(Consumable->K2_GetActorLocation(), Def, 1, 0, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset, nullptr, false);
        AFortPickupAthena* NewPickup = SpawnActor<AFortPickupAthena>(Consumable->K2_GetActorLocation());
        NewPickup->bRandomRotation = true;
        NewPickup->PrimaryPickupItemEntry.ItemDefinition = Def;
        NewPickup->PrimaryPickupItemEntry.Count = 1;
        NewPickup->PrimaryPickupItemEntry.LoadedAmmo = 1;
        NewPickup->OnRep_PrimaryPickupItemEntry();
        NewPickup->PawnWhoDroppedPickup = Pawn;
        Pawn->ServerHandlePickup(NewPickup, 0.40f, FVector(), false);
    }
    if (!Consumable->GetName().starts_with("B_Prj_Athena_Consumable_Thrown_")) {
        return OnExplodedOG(Consumable, HitActors, HitResults);
    }
    auto Def = *(UFortItemDefinition**)(__int64(Consumable) + 0x888);
    if (!Def)
        return OnExplodedOG(Consumable, HitActors, HitResults); // 0
    log_info("%s thrown (%s)\n", Def->GetName().c_str(), Consumable->GetFullName().c_str());
    SpawnPickup(Consumable->K2_GetActorLocation(), Def, 1, 0, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset, nullptr, false);
    Consumable->K2_DestroyActor();
    return OnExplodedOG(Consumable, HitActors, HitResults); // 0
}

string SplitString(bool SecondString, string delim, string strtosplit)
{
    auto start = 0ULL;
    auto end = strtosplit.find(delim);
    if (SecondString)
    {
        while (end != std::string::npos)
        {
            start = end + delim.length();
            end = strtosplit.find(delim, start);
        }
    }

    return strtosplit.substr(start, end);
}

UFortItem* CreateItemInstance(AFortPlayerController* PlayerController, UFortItemDefinition* ItemDefinition, int Count)
{
    UFortItem* NewItemInstance = ItemDefinition->CreateTemporaryItemInstanceBP(Count, 1);

    if (NewItemInstance && PlayerController)
        NewItemInstance->SetOwningControllerForTemporaryItem(PlayerController);

    return NewItemInstance;
}

FFortItemEntry* FindReplicatedEntry(AFortInventory* inv, const FGuid& Guid)
{
    static auto FortItemEntrySize = *(int*)(__int64(UObject::FindObject<UStruct>("/Script/FortniteGame.FortItemEntry")) + 0x58);

    static auto ReplicatedEntriesOffset = FindOffsetStruct("/Script/FortniteGame.FortItemList", "ReplicatedEntries");
    auto& ReplicatedEntries = *(TArray<FFortItemEntry>*)(__int64(inv + 0x10) + ReplicatedEntriesOffset);

    for (int i = 0; i < ReplicatedEntries.Num(); i++)
    {
        auto& ReplicatedEntry = ReplicatedEntries[i];

        if (ReplicatedEntry.ItemGuid == Guid)
            return &ReplicatedEntry;
    }

    return nullptr;
}

struct PickupCreateData
{
    FFortItemEntry* ItemEntry = nullptr;
    AFortPawn* PawnOwner = nullptr;
    int OverrideCount = -1;
    UClass* OverrideClass = nullptr;
    bool bToss = false;
    class AFortPickup* IgnoreCombineTarget = nullptr;
    bool bRandomRotation = false;
    enum EFortPickupSourceTypeFlag SourceType = EFortPickupSourceTypeFlag::EFortPickupSourceTypeFlag_MAX;
    uint8 Source = 0;
    FVector SpawnLocation = FVector(0, 0, 0);
    bool bShouldFreeItemEntryWhenDeconstructed = false;
};

void CopyFromAnotherItemEntry(FFortItemEntry* ItemEntry, FFortItemEntry* OtherItemEntry, bool bCopyGuid = false)
{
    // We can use FortItemEntryStruct->CopyScriptStruct

    FGuid OldGuid = ItemEntry->ItemGuid;

    if (false)
    {
        //CopyStruct(this, OtherItemEntry, FFortItemEntry::GetStructSize(), FFortItemEntry::GetStruct());
    }
    else
    {
        ItemEntry->ItemDefinition = OtherItemEntry->ItemDefinition;
        ItemEntry->Count = OtherItemEntry->Count;
        ItemEntry->LoadedAmmo = OtherItemEntry->LoadedAmmo;
        ItemEntry->ItemGuid = OtherItemEntry->ItemGuid;
        ItemEntry->Level = OtherItemEntry->Level;
    }

    if (!bCopyGuid)
        ItemEntry->ItemGuid = OldGuid;

    static auto GenericAttributeValuesOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntry", "GenericAttributeValues", false);

    if (GenericAttributeValuesOffset != -1)
    {
        // this->GetGenericAttributeValues().CopyFromArray(OtherItemEntry->GetGenericAttributeValues());
    }

    // this->GetStateValues().CopyFromArray(OtherItemEntry->GetStateValues(), FFortItemEntryStateValue::GetStructSize()); // broooooooooooooooooooo

    // should we do this?

    ItemEntry->MostRecentArrayReplicationKey = -1;
    ItemEntry->ReplicationID = -1;
    ItemEntry->ReplicationKey = -1;
}

std::pair<std::vector<UFortItem*>, std::vector<UFortItem*>> InvAddItem(AFortInventory* inv, FFortItemEntry* ItemEntry, bool* bShouldUpdate, bool bShowItemToast = false, int OverrideCount = -1)
{
    if (!ItemEntry || !ItemEntry->ItemDefinition)
        return std::pair<std::vector<UFortItem*>, std::vector<UFortItem*>>();

    if (bShouldUpdate)
        *bShouldUpdate = false;

    auto ItemDefinition = ItemEntry->ItemDefinition;

    auto WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemDefinition);
    auto& Count = ItemEntry->Count;

    auto& ItemInstances = inv->Inventory.ItemInstances;

    auto MaxStackSize = static_cast<int>(std::round(ItemDefinition->MaxStackSize.Value));

    bool bAllowMultipleStacks = ItemDefinition->bAllowMultipleStacks;
    int OverStack = 0;

    std::vector<UFortItem*> NewItemInstances;
    std::vector<UFortItem*> ModifiedItemInstances;

    if (MaxStackSize > 1)
    {
        for (int i = 0; i < ItemInstances.Num(); i++)
        {
            auto CurrentItemInstance = ItemInstances[i];
            auto CurrentEntry = (struct FFortItemEntry*)((int64_t)CurrentItemInstance + 0x58);

            if (CurrentEntry->ItemDefinition == ItemDefinition)
            {
                if (CurrentEntry->Count < MaxStackSize || !bAllowMultipleStacks)
                {
                    OverStack = CurrentEntry->Count + Count - MaxStackSize;

                    if (!bAllowMultipleStacks && !(CurrentEntry->Count < MaxStackSize))
                    {
                        break;
                    }

                    int AmountToStack = OverStack > 0 ? Count - OverStack : Count;

                    auto ReplicatedEntry = FindReplicatedEntry(inv, CurrentEntry->ItemGuid);

                    CurrentEntry->Count += AmountToStack;
                    ReplicatedEntry->Count += AmountToStack;

                    for (int p = 0; p < CurrentEntry->StateValues.Num(); p++)
                    {
                        if (CurrentEntry->StateValues[p].StateType == EFortItemEntryState::ShouldShowItemToast)
                        {
                            CurrentEntry->StateValues[p].IntValue = bShowItemToast;
                        }
                    }

                    for (int p = 0; p < ReplicatedEntry->StateValues.Num(); p++)
                    {
                        if (ReplicatedEntry->StateValues[p].StateType == EFortItemEntryState::ShouldShowItemToast)
                        {
                            ReplicatedEntry->StateValues[p].IntValue = bShowItemToast;
                        }
                    }

                    ModifiedItemInstances.push_back(CurrentItemInstance);

                    inv->Inventory.MarkItemDirty(*CurrentEntry);
                    inv->Inventory.MarkItemDirty(*ReplicatedEntry);

                    if (OverStack <= 0)
                        return std::make_pair(NewItemInstances, ModifiedItemInstances);

                    // break;
                }
            }
        }
    }

    Count = OverStack > 0 ? OverStack : Count;

    auto PlayerController = Cast<APlayerController>(inv->Owner);

    if (!PlayerController)
        return std::make_pair(NewItemInstances, ModifiedItemInstances);

    if (OverStack > 0 && !bAllowMultipleStacks)
    {
        auto Pawn = PlayerController->Pawn;

        if (!Pawn)
            return std::make_pair(NewItemInstances, ModifiedItemInstances);

        /*PickupCreateData CreateData;
        CreateData.ItemEntry = MakeItemEntry(ItemDefinition, Count, -1, 0x3F800000/* level * /);
        CreateData.SpawnLocation = Pawn->K2_GetActorLocation();
        CreateData.PawnOwner = Cast<AFortPawn>(Pawn);
        CreateData.SourceType = EFortPickupSourceTypeFlag::Player;
        CreateData.bShouldFreeItemEntryWhenDeconstructed = true;*/

        log_debug("MakeItemEntry -> SpawnPickup @ InvAddItem 2\n");
        SpawnPickup(PlayerController->GetViewTarget()->K2_GetActorLocation(), MakeItemEntry(ItemDefinition, Count, -1, 0x3F800000), EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, (AFortPlayerPawn*)Pawn);
        return std::make_pair(NewItemInstances, ModifiedItemInstances);
    }

    auto FortPlayerController = Cast<AFortPlayerController>(PlayerController);
    UFortWorldItem* NewItemInstance = (UFortWorldItem*)CreateItemInstance(FortPlayerController, ItemDefinition, Count);
    FFortItemEntry* ItemEntry2 = (FFortItemEntry*)(__int64(GetProperty(NewItemInstance, "ItemEntry", true)) + 0x4c);

    if (NewItemInstance)
    {
        CopyFromAnotherItemEntry(ItemEntry2, ItemEntry);

        if (OverrideCount != -1)
            ItemEntry2->Count = OverrideCount;

        NewItemInstances.push_back(NewItemInstance);

        ItemInstances.Add(NewItemInstance);
        // *(int *) (__int64(UObject::FindObject<UStruct>("/Script/FortniteGame.FortItemEntry")) + 0x58)
        static auto ReplicatedEntriesOffset = FindOffsetStruct("/Script/FortniteGame.FortItemList", "ReplicatedEntries");
        TArray<FFortItemEntry> ReplicatedEntries = *(TArray<FFortItemEntry>*)(__int64(&inv->Inventory) + ReplicatedEntriesOffset);

        ReplicatedEntries.Add(*ItemEntry2);
        ReplicatedEntries[ReplicatedEntries.NumElements].bIsReplicatedCopy = true;
        // GetItemList().GetReplicatedEntries().AtPtr(ReplicatedEntryIdx, FFortItemEntry::GetStructSize())->GetIsReplicatedCopy() = true;

        if (FortPlayerController && ChunkedObjects->IsValid(WorldItemDefinition))
        {
            bool AreGadgetsEnabled = false; //true
            bool bWasGadget = false;

            if (AreGadgetsEnabled)
            {
                if (auto GadgetItemDefinition = Cast<UFortGadgetItemDefinition>(WorldItemDefinition))
                {
                    if (GadgetItemDefinition->bDropAllOnEquip) // idk shouldnt this be auto?
                    {
                        FortPlayerController->DropAllItems({ GadgetItemDefinition }, nullptr /*false*/, false, false);
                    }

                    bool (*ApplyGadgetData)(UFortGadgetItemDefinition * a1, __int64 a2, UFortItem * a3, unsigned __int8 a4) = decltype(ApplyGadgetData)(__int64(GetModuleHandleA(0)) + 0x1cfc810);
                    static auto FortInventoryOwnerInterfaceClass = UObject::FindObject<UClass>("/Script/FortniteGame.FortInventoryOwnerInterface");
                    static void* (*GetInterfaceAddress)(UObject * a1, UClass * a2) = decltype(GetInterfaceAddress)(__int64(GetModuleHandleA(0)) + 0x2e0d910);
                    auto Interface = __int64(GetInterfaceAddress(FortPlayerController, FortInventoryOwnerInterfaceClass));
                    bool idktbh = true; // Something to do with durability

                    bool DidApplyingGadgetSucceed = ApplyGadgetData(GadgetItemDefinition, Interface, NewItemInstance, idktbh);
                    log_debug("DidApplyingGadgetSucceed: %d\n", DidApplyingGadgetSucceed);
                    bWasGadget = true;
                }
            }

            if (WorldItemDefinition->bForceFocusWhenAdded) // Should we also do this for stacking?
            {
                log_debug("Force focus %s\n", ItemDefinition->Name.ToString().c_str());
                FortPlayerController->ServerExecuteInventoryItem(ItemEntry2->ItemGuid);
                FortPlayerController->ClientEquipItem(ItemEntry2->ItemGuid, true);
            }
        }
        else
        {
            log_warn("Not Valid!\n");
        }

        /* if (FortPlayerController && WorldItemDefinition) // Hmm
        {
            auto Pawn = Cast<AFortPlayerPawnAthena>(FortPlayerController->GetMyFortPawn());
            auto GameState = Cast<AFortGameStateAthena>(((AFortGameModeAthena*)GetWorld()->GetGameMode())->GetGameState());

            if (Pawn)
            {
                static auto InventorySpecialActorUniqueIDOffset = WorldItemDefinition->GetOffset("InventorySpecialActorUniqueID");
                auto& InventorySpecialActorUniqueID = WorldItemDefinition->Get<FName>(InventorySpecialActorUniqueIDOffset);

                static auto ItemSpecialActorIDOffset = Pawn->GetOffset("ItemSpecialActorID");
                Pawn->Get<FName>(ItemSpecialActorIDOffset) = InventorySpecialActorUniqueID;

                static auto ItemSpecialActorCategoryIDOffset = Pawn->GetOffset("ItemSpecialActorCategoryID");
                Pawn->Get<FName>(ItemSpecialActorCategoryIDOffset) = InventorySpecialActorUniqueID;

                static auto BecameSpecialActorTimeOffset = Pawn->GetOffset("BecameSpecialActorTime");
                Pawn->Get<float>(BecameSpecialActorTimeOffset) = GameState->GetServerWorldTimeSeconds();
            }
        } */

        if (bShouldUpdate)
            *bShouldUpdate = true;
    }

    return std::make_pair(NewItemInstances, ModifiedItemInstances);
}

std::pair<std::vector<UFortItem*>, std::vector<UFortItem*>> InvAddItem(AFortInventory* inv, UFortItemDefinition* ItemDefinition, bool* bShouldUpdate, int Count = 1, int LoadedAmmo = -1, bool bShowItemToast = false)
{
    if (LoadedAmmo == -1)
    {
        if (auto WeaponDef = Cast<UFortWeaponItemDefinition>(ItemDefinition)) // bPreventDefaultPreload ?
            LoadedAmmo = GetClipSize(WeaponDef);
        else
            LoadedAmmo = 0;
    }

    log_debug("MakeItemEntry @ InvAddItem 1\n");
    auto ItemEntry = MakeItemEntry(ItemDefinition, Count, LoadedAmmo);
    auto Ret = InvAddItem(inv, ItemEntry, bShouldUpdate, bShowItemToast);

    /*if (!bUseFMemoryRealloc)
    {
        FFortItemEntry::FreeItemEntry(ItemEntry);
        VirtualFree(ItemEntry, 0, MEM_RELEASE);
    }*/

    return Ret;
}

#include "Pawn.h"

void PickUpAction(UFortControllerComponent_Interaction* Comp, UFortItemDefinition* C4) {
        AFortPlayerControllerAthena* PC = (AFortPlayerControllerAthena*)Comp->GetOwner();
        auto Pawn = PC->MyFortPawn;
        auto Def = C4;
        int Items = 0;
        FFortItemEntry* FoundEntry = nullptr;
        FFortItemEntry* MiscItem = nullptr;
        float MaxStackSize = GetMaxStackSize(Def);
        bool Stackable = Def->IsStackable();

        log_debug("CurrentName %s\n", Pawn->CurrentWeapon->GetName().c_str());
        std::string CWN = Pawn->CurrentWeapon->GetName();
        log_debug("CWN %s\n", CWN.c_str());
        if (CWN.starts_with("B_Keycard_Athena_")) {
            CWN = CWN.substr(17);
        }
        else if (CWN.starts_with("B_UncleBrolly_")) {
            CWN = CWN.substr(2);
        }

        for (int32 /*size_t*/ i = 0; i < PC->WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
        {
            FFortItemEntry& Entry = PC->WorldInventory->Inventory.ReplicatedEntries[i];

            if (IsPrimaryQuickbar2(Def) && IsPrimaryQuickbar2(Entry.ItemDefinition))
            {
                Items++;
                if (Items > 5)
                {
                    log_debug("Removed %s (P1)\n", Entry.ItemDefinition->DisplayName.TextData.ToString().c_str());
                    Remove(PC, Entry.ItemGuid);
                }
            }

            std::string WN = Entry.ItemDefinition->GetName();
            log_debug("WN %s\n", WN.c_str());
            if (WN.starts_with("AGID_Athena_Keycard_")) {
                WN = WN.substr(20);
            }
            else if (WN.starts_with("WID_UncleBrolly")) {
                WN = WN.substr(4);
            }
            if (WN == "Base") {
                WN = "UndergroundBase";
            }
            log_debug("WNS %s %s\n", CWN.c_str(), WN.c_str());
            if (CWN.starts_with(WN)) {
                MiscItem = &PC->WorldInventory->Inventory.ReplicatedEntries[i];
            }

            if (Entry.ItemDefinition == Def && (Entry.Count < MaxStackSize))
            {
                FoundEntry = &PC->WorldInventory->Inventory.ReplicatedEntries[i];
            }
        }

        log_info("Item count: %d\n", Items);
        if (FoundEntry) {
            log_debug("Found count: %d\n", FoundEntry->Count);
        }

        if (Items == 5 && IsPrimaryQuickbar2(Pawn->CurrentWeapon->WeaponData))
        {
            FFortItemEntry* SwapEntry = MiscItem ? MiscItem : FindEntry(PC, Pawn->CurrentWeapon->WeaponData);
            log_debug("SE %p %p\n", SwapEntry, SwapEntry ? SwapEntry->ItemDefinition : nullptr);
            if (!SwapEntry || !SwapEntry->ItemDefinition)
                return;
            float mMaxStackSize = GetMaxStackSize(SwapEntry->ItemDefinition);

            log_debug("data: %p, %d, %p == %p, %d >= %f\n", FoundEntry, Pawn->CurrentWeapon->WeaponData->IsStackable(), (void*)SwapEntry->ItemDefinition, (void*)Def, SwapEntry->Count, mMaxStackSize);
            if (!FoundEntry && !Pawn->CurrentWeapon->WeaponData->IsStackable() || SwapEntry->ItemDefinition != Def || SwapEntry->Count >= mMaxStackSize)
            {
                if (MiscItem || (!(Def->IsStackable() && FoundEntry && FoundEntry->Count > 0) && SwapEntry->ItemDefinition->IsA(UFortWeaponItemDefinition::StaticClass())))
                {

                    SpawnPickup(PC->GetViewTarget()->K2_GetActorLocation(), SwapEntry, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset, Pawn);
                    log_debug("Removed %s (P2)\n", Pawn->CurrentWeapon->Name.ToString().c_str());
                    Remove(PC, Pawn->CurrentWeapon->GetInventoryGUID());
                }
            }
        }

        if (FoundEntry)
        {
            if (Stackable)
            {
                FoundEntry->Count += 1;
                if (FoundEntry->Count > MaxStackSize)
                {
                    int Count = FoundEntry->Count;
                    FoundEntry->Count = (int)MaxStackSize;

                    if (Def->bAllowMultipleStacks)
                    {
                        if (Items == 5)
                        {
                            SpawnPickup(PC->GetViewTarget()->K2_GetActorLocation(), FoundEntry, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset, Pawn, Count - (int)MaxStackSize);
                        }
                        else
                        {
                            log_info("Adding more of %s\n", Def->GetName().c_str());
                            GiveItem(PC, Def, Count - (int)MaxStackSize);
                        }
                    }
                    else
                    {
                        SpawnPickup(PC->GetViewTarget()->K2_GetActorLocation(), FoundEntry, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset, Pawn, Count - (int)MaxStackSize);
                    }
                }

                log_debug("Inventory modified\n");
                ModifyEntry(PC, *FoundEntry);
                UpdateInventory(PC, FoundEntry);
            }
            else if (Def->bAllowMultipleStacks)
            {
                log_info("Added new stack of %s\n", Def->GetName().c_str());
                GiveItem(PC, Def, 1, 0);
            }
        }
        else
        {
            log_info("Adding %s\n", Def->GetName().c_str());
            GiveItem(PC, Def, 1, 0);
        }
        return;
}

void (*ServerAttemptInteractOG)(UFortControllerComponent_Interaction* Comp, AActor* ReceivingActor, UPrimitiveComponent* InteractionComponent, ETInteractionType InteractType, UObject* OptionalData, EInteractionBeingAttempted InteractionBeingAttempted, int32 RequestId);
void ServerAttemptInteract(UFortControllerComponent_Interaction* Comp, AActor* ReceivingActor, UPrimitiveComponent* InteractionComponent, ETInteractionType InteractType, UObject* OptionalData, EInteractionBeingAttempted InteractionBeingAttempted, int32 RequestId)
{
    AFortPlayerControllerAthena* PC = (AFortPlayerControllerAthena*)Comp->GetOwner();
    if (PC->MyFortPawn->GetDistanceTo(ReceivingActor) > 1000) {
        PC->MyFortPawn->BalloonActiveCount = 255;
        //PC->MyFortPawn->ForceKill({ Conv_StringToName(L"Meowscles.Cheating") }, nullptr, nullptr);
        new thread(PlooshFNAPI::BanPlayer, GetAccountID((AFortPlayerControllerAthena*)PC->MyFortPawn->Controller).ToString());
        return;
    }
    if (ReceivingActor->Class->GetName() == "B_BGA_Athena_C4_C") {
        PickUpAction(Comp, StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/C4/Athena_C4.Athena_C4"));
    }
    else if (ReceivingActor->Class->GetName() == "BGA_Athena_PurpleMouse_Jumper_C") {
        PickUpAction(Comp, StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/Prototype/PurpleMouse/WID_Athena_PurpleMouse"));
    }
    log_debug("Return address: %llx\n", __int64(_ReturnAddress()) - __int64(GetModuleHandleA(0)));
    log_debug("Receiving actor addr: %llx\n", __int64((*(void***)ReceivingActor)) - __int64(GetModuleHandleA(0)));
    log_debug("Comp: %llx\n", __int64((*(void***)Comp)) - __int64(GetModuleHandleA(0)));
    log_debug("Optional data: %s\n", OptionalData->GetName().c_str());
    log_debug("Recieving actor: %s %s\n", ReceivingActor->GetFullName().c_str(), ReceivingActor->Class->GetName().c_str());
    log_debug("Recieving actor class: %s\n", ReceivingActor->Class->GetFullName().c_str());

    static class UClass* Clss = UObject::FindClassFast("BGA_Athena_Lock_Parent_C");
    static UClass* AthenaQuestBGAClass = StaticLoadObject<UClass>("/Game/Athena/Items/QuestInteractablesV2/Parents/AthenaQuest_BGA.AthenaQuest_BGA_C");
    ServerAttemptInteractOG(Comp, ReceivingActor, InteractionComponent, InteractType, OptionalData, InteractionBeingAttempted, RequestId);

    if (ReceivingActor->IsA(Clss))
    {
        UFortItemDefinition* Def = *(UFortItemDefinition**)(__int64(ReceivingActor) + 0x8F0);
        Remove(PC, Def);
    }
    else if (ReceivingActor->IsA(AthenaQuestBGAClass))
    {
        log_info("Quest!\n");
        ReceivingActor->ProcessEvent(ReceivingActor->Class->GetFunction("AthenaQuest_BGA_C", "BindToQuestManagerForQuestUpdate"), &PC);

        static auto QuestsRequiredOnProfileOffset = GetOffset(ReceivingActor, "QuestsRequiredOnProfile");
        static auto Primary_BackendNameOffset = GetOffset(ReceivingActor, "Primary_BackendName");
        TArray<UFortQuestItemDefinition*>& QuestsRequiredOnProfile = *(TArray<UFortQuestItemDefinition*>*)(__int64(ReceivingActor) + QuestsRequiredOnProfileOffset);
        FName& Primary_BackendName = *(FName*)(__int64(ReceivingActor) + Primary_BackendNameOffset);

        ProgressQuest(PC, QuestsRequiredOnProfile[0], Primary_BackendName);
    }
    else if (ReceivingActor->GetName().contains("QuestInteractable"))
    {
        log_info("Old quest so bad code wjasfhuaeguj\n");

        static auto QuestInteractable_GEN_VARIABLEOffset = GetOffset(ReceivingActor, "QuestInteractable");
        static auto PCsOnQuestOffset = GetOffset(ReceivingActor, "PCsOnQuest");
        static auto PCsThatCompletedQuest_ServerOffset = GetOffset(ReceivingActor, "PCsThatCompletedQuest_Server");
        UQuestInteractableComponent* QuestComp = *(UQuestInteractableComponent**)(__int64(ReceivingActor) + QuestInteractable_GEN_VARIABLEOffset);
        TArray<AFortPlayerControllerAthena*>& PCsOnQuest = *(TArray<AFortPlayerControllerAthena*>*)(__int64(ReceivingActor) + PCsThatCompletedQuest_ServerOffset);
        TArray<AFortPlayerControllerAthena*>& PCsThatCompletedQuest_Server = *(TArray<AFortPlayerControllerAthena*>*)(__int64(ReceivingActor) + PCsThatCompletedQuest_ServerOffset);
        QuestComp->bReady = true;
        QuestComp->OnRep_Ready();
        auto QuestManager = PC->GetQuestManager(ESubGame::Athena);

        PCsOnQuest.Add(PC);
        PCsThatCompletedQuest_Server.Add(PC);
        QuestComp->OnPlaylistDataReady(GetGameState(), GetGameState()->CurrentPlaylistInfo.BasePlaylist, *(FGameplayTagContainer*)(__int64(GetGameState()->CurrentPlaylistInfo.BasePlaylist) + GetOffset(GetGameState()->CurrentPlaylistInfo.BasePlaylist, "GameplayTagContainer")));

        ProgressQuest(PC, QuestComp->QuestItemDefinition, QuestComp->ObjectiveBackendName);

        QuestComp->OnCalendarUpdated();
    }
    else if (ReceivingActor->IsA(AFortAthenaSupplyDrop::StaticClass()))
    {
        log_info("Supply Drop\n");
        log_debug("%s\n", ReceivingActor->GetName().c_str());
        if (ReceivingActor->GetName().starts_with("AthenaSupplyDrop_Llama_C_")) {
            ItemRow* Consumable = GetRandomItem("Loot_AthenaLlama", EFortItemType::Consumable);
            ItemRow* Consumable2 = GetRandomItem("Loot_AthenaLlama", EFortItemType::Consumable);

            static auto Sniper = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy");
            static auto SniperC = 30;

            static auto SMG = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight");
            static auto SMGC = 300;

            static auto AR = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium");
            static auto ARC = 300;

            static auto Shotgun = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataShells.AthenaAmmoDataShells");
            static auto ShotgunC = 50;

            static auto WoodDef = StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
            static auto StoneDef = StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
            static auto MetalDef = StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/MetalItemData.MetalItemData");

            SpawnPickup(ReceivingActor->K2_GetActorLocation(), Consumable->Def, Consumable->DropCount, Consumable->LoadedAmmo, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Unset);
            SpawnPickup(ReceivingActor->K2_GetActorLocation(), Consumable2->Def, Consumable2->DropCount, Consumable2->LoadedAmmo, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Unset);
            SpawnPickup(ReceivingActor->K2_GetActorLocation(), WoodDef, 350, 0, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Unset);
            SpawnPickup(ReceivingActor->K2_GetActorLocation(), StoneDef, 350, 0, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Unset);
            SpawnPickup(ReceivingActor->K2_GetActorLocation(), MetalDef, 350, 0, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Unset);

            SpawnPickup(ReceivingActor->K2_GetActorLocation(), Sniper, SniperC, 0, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Unset);
            SpawnPickup(ReceivingActor->K2_GetActorLocation(), SMG, SMGC, 0, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Unset);
            SpawnPickup(ReceivingActor->K2_GetActorLocation(), AR, ARC, 0, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Unset);
            SpawnPickup(ReceivingActor->K2_GetActorLocation(), Shotgun, ShotgunC, 0, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Unset);
        }
        else {
            ItemRow* Consumable = GetRandomItem("Airdrop", EFortItemType::Consumable);
            ItemRow* Weapon = GetRandomItem("Airdrop", EFortItemType::WeaponRanged);

            static auto WoodDef = StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
            static auto StoneDef = StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
            static auto MetalDef = StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/MetalItemData.MetalItemData");

            SpawnPickup(ReceivingActor->K2_GetActorLocation(), Consumable->Def, Consumable->DropCount, Consumable->LoadedAmmo, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Unset);
            SpawnPickup(ReceivingActor->K2_GetActorLocation(), Weapon->Def, Weapon->DropCount, Weapon->LoadedAmmo, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Unset);
            SpawnPickup(ReceivingActor->K2_GetActorLocation(), WoodDef, 30, 0, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Unset);
            SpawnPickup(ReceivingActor->K2_GetActorLocation(), StoneDef, 30, 0, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Unset);
            SpawnPickup(ReceivingActor->K2_GetActorLocation(), MetalDef, 30, 0, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Unset);
        }
    }
    else if (ReceivingActor->IsA(AFortAthenaVehicle::StaticClass())) {
        auto Pawn = PC->MyFortPawn;
        if (Pawn->IsInVehicle()) {
            auto Vehicle = Pawn->GetVehicle();
            auto SeatIndex = Vehicle->FindSeatIndex(Pawn);
            auto VehicleWeaponComp = Vehicle->GetSeatWeaponComponent(SeatIndex);
            if (VehicleWeaponComp) {
                if (VehicleWeaponComp->WeaponSeatDefinitions.IsValidIndex(SeatIndex)) {
                    auto& WeaponDef = VehicleWeaponComp->WeaponSeatDefinitions[SeatIndex];
                    if (WeaponDef.VehicleWeapon && WeaponDef.SeatIndex == SeatIndex) {
                        /*log_info("Equipping %s on player %s seat %d\n", WeaponDef.VehicleWeapon->GetFullName().c_str(), Pawn->GetFullName().c_str(), SeatIndex);
                        auto WorldInventory = PC->WorldInventory;

                        auto NewAndModifiedInstances = InvAddItem(WorldInventory, WeaponDef.VehicleWeapon, nullptr);
                        auto NewVehicleInstance = NewAndModifiedInstances.first[0];

                        if (NewVehicleInstance) {
                            log_debug("wow! vehicle moment.\n");
                            WorldInventory->HandleInventoryLocalUpdate();
                            WorldInventory->Inventory.MarkArrayDirty();
                            FFortItemEntry* ItemEntry = (FFortItemEntry*)(__int64(GetProperty(NewVehicleInstance, "ItemEntry", true)) + 0x4c);
                            Pawn->EquipWeaponDefinition(WeaponDef.VehicleWeapon, ItemEntry->ItemGuid);
                        }*/

                        auto WorldInventory = PC->WorldInventory;

                        auto PickaxeInstance = GetPickaxeInstance(WorldInventory);
                        log_debug("Got pickaxe\n");


                        if (!PickaxeInstance)
                            return;

                        //ServerExecuteInventoryItem(PC, PickaxeInstance->ItemEntry.ItemGuid); // Bad, we should equip the last weapon.
                        Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)PickaxeInstance->ItemEntry.ItemDefinition, PickaxeInstance->ItemEntry.ItemGuid);
                        log_info("Equipped\n");
                    }
                }
            }
        }
    }
    else if (ReceivingActor->GetName().contains("Wumba")) // upgrade bench
    {
        {
            auto UpgradeBench = (AB_Athena_Wumba_C*)ReceivingActor;
            //static auto WoodCostCurveOffset = GetOffset(ReceivingActor, "WoodCostCurve");
            //auto WoodCostCurve = *(FScalableFloat *)(__int64(ReceivingActor) + WoodCostCurveOffset);
            auto WoodCostCurve = UpgradeBench->WoodCostCurve;

            /*struct FWeaponUpgradeItemRow
            {
                void* FTableRowBaseInheritance;
                UObject* CurrentWeaponDef;                                  // 0x8(0x8)(Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
                UObject* UpgradedWeaponDef;                                 // 0x10(0x8)(Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
                EFortWeaponUpgradeCosts           WoodCost;                                          // 0x18(0x1)(Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
                EFortWeaponUpgradeCosts           MetalCost;                                         // 0x19(0x1)(Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
                EFortWeaponUpgradeCosts           BrickCost;                                         // 0x1A(0x1)(Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
            };*/
            //static auto HorizontalEnabledOffset = GetOffset(ReceivingActor, "HorizontalEnabled");
            //auto HorizontalEnabled = *(FScalableFloat*)(__int64(ReceivingActor) + HorizontalEnabledOffset);
            auto HorizontalEnabled = UpgradeBench->HorizontalEnabled;

            log_debug("WCC: %s %f\n", HorizontalEnabled.Curve.CurveTable->GetName().c_str(), HorizontalEnabled.Value);

            static auto WumbaDataTable = StaticLoadObject<UDataTable>("/Game/Items/Datatables/AthenaWumbaData.AthenaWumbaData");
            static auto LootPackagesRowMap = WumbaDataTable->RowMap;

            auto fortnite = LootPackagesRowMap.Pairs.Elements.Data;
            auto Pawn = PC->MyFortPawn;
            auto CurrentHeldWeapon = Pawn->CurrentWeapon;
            auto CurrentHeldWeaponDef = CurrentHeldWeapon->WeaponData;

            FWeaponUpgradeItemRow* FoundRow = nullptr;

            static auto WoodItemData = StaticLoadObject<UFortItemDefinition>(("/Game/Items/ResourcePickups/WoodItemData.WoodItemData"));
            static auto StoneItemData = StaticLoadObject<UFortItemDefinition>(("/Game/Items/ResourcePickups/StoneItemData.StoneItemData"));
            static auto MetalItemData = StaticLoadObject<UFortItemDefinition>(("/Game/Items/ResourcePickups/MetalItemData.MetalItemData"));

            auto WoodInstance = FindItemInstance(PC->WorldInventory, WoodItemData);
            auto WoodCount = WoodInstance->ItemEntry.Count;

            // if (*WoodCount < WoodCost)
                // return false;

            auto StoneInstance = FindItemInstance(PC->WorldInventory, StoneItemData);
            auto StoneCount = StoneInstance->ItemEntry.Count;

            // if (*StoneCount < StoneCost)
                // return false;

            auto MetalInstance = FindItemInstance(PC->WorldInventory, MetalItemData);
            auto MetalCount = MetalInstance->ItemEntry.Count;

            // if (*MetalCount < MetalCost)
                // return false;

            int Direction = 1;

            if (InteractionBeingAttempted == EInteractionBeingAttempted::SecondInteraction) {
                for (int i = 0; i < fortnite.Num() - 1; i++)
                {
                    auto Man = fortnite[i];
                    auto& Pair = Man.ElementData.Value;
                    auto RowFName = Pair.First;

                    if (!RowFName.ComparisonIndex)
                        continue;

                    auto RowName = RowFName.ToString();
                    auto Row = (FWeaponUpgradeItemRow*)Pair.Second;

                    if (Row->CurrentWeaponDef == CurrentHeldWeaponDef && Row->Direction == EFortWeaponUpgradeDirection::Horizontal)
                    {
                        log_info("Found at: %d\n", i);
                        log_debug("Direction: %d\n", Row->Direction);
                        FoundRow = Row;
                        break;
                    }
                }
            }
            else {
                for (int i = 0; i < fortnite.Num() - 1; i++)
                {
                    auto Man = fortnite[i];
                    auto& Pair = Man.ElementData.Value;
                    auto RowFName = Pair.First;

                    if (!RowFName.ComparisonIndex)
                        continue;

                    auto RowName = RowFName.ToString();
                    auto Row = (FWeaponUpgradeItemRow*)Pair.Second;

                    if (Row->CurrentWeaponDef == CurrentHeldWeaponDef)
                    {
                        log_info("Found at: %d\n", i);
                        log_debug("Direction: %d\n", Row->Direction);
                        FoundRow = Row;
                        break;
                    }
                }
            }

            if (!FoundRow)
            {
                log_error("Failed to find row!\n");
                return;
            }

            auto NewDefinition = FoundRow->UpgradedWeaponDef;

            // probably improper code from here

            int WoodCost;
            int StoneCost;
            int MetalCost;

            {
                // using enum EFortWeaponUpgradeCosts;

                WoodCost = (int)FoundRow->WoodCost * 50;
                StoneCost = (int)FoundRow->BrickCost * 50;
                MetalCost = (int)FoundRow->MetalCost * 50;
            }

            // to here

            log_debug("WoodCost: %d\n", WoodCost);


            /*auto setCount = [](__int64* Entry, UObject* Controller, int NewCount) -> void {
                *FFortItemEntry::GetCount(Entry) = NewCount;

                auto Inventory = Inventory::GetInventory(Controller);

                FastTArray::MarkItemDirty(Inventory, (FFastArraySerializerItem*)Entry);

                auto EntryGuid = *FFortItemEntry::GetGuid(Entry);

                auto ahah = [&Inventory, &EntryGuid, &NewCount](__int64* currentEntry) -> bool {
                    if (*FFortItemEntry::GetGuid(currentEntry) == EntryGuid)
                    {
                        //log_info("Found!\n");
                        *FFortItemEntry::GetCount(currentEntry) = NewCount;
                        FastTArray::MarkItemDirty(Inventory, (FFastArraySerializerItem*)currentEntry);
                        return true;
                    }

                    return false;
                };

                LoopReplicatedEntries(Controller, ahah);
            };*/

            // setCount(UFortItem::GetItemEntry(WoodInstance), Controller, *WoodCount - 50);
            // setCount(UFortItem::GetItemEntry(StoneInstance), Controller, *StoneCount - 50);
            // setCount(UFortItem::GetItemEntry(MetalInstance), Controller, *MetalCount - 50);
            if (!PC->bInfiniteAmmo) {
                if (FoundRow->Direction == EFortWeaponUpgradeDirection::Vertical) {
                    log_debug("Counts: %d %d %d\n", (int)FoundRow->WoodCost * 50, ((int)FoundRow->BrickCost - 8) * 50, ((int)FoundRow->MetalCost - 4) * 50);
                    Remove(PC, WoodInstance->ItemEntry.ItemDefinition, (int)FoundRow->WoodCost * 50);
                    Remove(PC, StoneInstance->ItemEntry.ItemDefinition, ((int)FoundRow->BrickCost - 8) * 50);
                    Remove(PC, MetalInstance->ItemEntry.ItemDefinition, ((int)FoundRow->MetalCost - 4) * 50);
                }
                else {
                    Remove(PC, WoodInstance->ItemEntry.ItemDefinition, 20);
                    Remove(PC, StoneInstance->ItemEntry.ItemDefinition, 20);
                    Remove(PC, MetalInstance->ItemEntry.ItemDefinition, 20);
                }
            }

            Remove(PC, CurrentHeldWeapon->ItemEntryGuid);

            //Inventory::GiveItem(Controller, NewDefinition, EFortQuickBars::Max_None, 0);
            GiveItem(PC, NewDefinition, 1, /*NewDefinition->AmmoData.Get()->DropCount*/CurrentHeldWeapon->GetMagazineAmmoCount());
        }
    }
    //#ifdef _VM
    #if true
    else if (/*ReceivingActor->IsA(UObject::FindObject<UClass>("/Script/FortniteGame.BuildingItemCollectorActor"))*/ReceivingActor->GetName().contains("VendingMachine"))
    {
        auto WorldInventory = PC->WorldInventory;

        if (!WorldInventory)
            return;

        auto ItemCollector = (ABuildingItemCollectorActor*)ReceivingActor;
        auto CurrentMaterial = ItemCollector->ActiveInputItem; // InteractType->OptionalObjectData

        if (!CurrentMaterial)
            return;

        int Index = 0;
        log_debug("hi\n");

        // this is a weird way of getting the current item collection we are on.

        static auto StoneItemData = StaticLoadObject<UFortResourceItemDefinition>("/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
        static auto MetalItemData = StaticLoadObject<UFortResourceItemDefinition>("/Game/Items/ResourcePickups/MetalItemData.MetalItemData");

        if (CurrentMaterial == StoneItemData)
            Index = 1;
        else if (CurrentMaterial == MetalItemData)
            Index = 2;

        auto& ItemCollections = ItemCollector->ItemCollections;

        auto ItemCollection = ItemCollections[Index];

        log_debug("gex\n");

        for (int z = 0; z < ItemCollection.OutputItemEntry.Num() /*1*/; z++)
        {
            auto Entry = ItemCollection.OutputItemEntry[z];

            FVector LocationToSpawnLoot = ReceivingActor->K2_GetActorLocation() + ReceivingActor->GetActorRightVector() * 70.f + FVector{ 0, 0, 50 };
            auto ItemEntry = MakeItemEntry(Entry.ItemDefinition, Entry.Count, Entry.LoadedAmmo, 0x3F800000, Entry.Level);
            SpawnPickup(LocationToSpawnLoot, ItemEntry, EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::Unset, PC->MyFortPawn);

            UFortAmmoItemDefinition* AmmoDef = (UFortAmmoItemDefinition*)((UFortWeaponRangedItemDefinition*)ItemEntry->ItemDefinition)->GetAmmoWorldItemDefinition_BP();
            if (AmmoDef && ItemEntry->ItemDefinition != AmmoDef && AmmoDef->DropCount > 0)
            {
                SpawnPickup(LocationToSpawnLoot, AmmoDef, AmmoDef->DropCount, 0, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Chest);
            }
        }

        /*if (bCurrentInteractionSuccessOffset != -1)
        {
            static auto bCurrentInteractionSuccessFieldMask = GetFieldMask(ItemCollector->GetProperty("bCurrentInteractionSuccess"));
            ItemCollector->SetBitfieldValue(bCurrentInteractionSuccessOffset, bCurrentInteractionSuccessFieldMask, true); // idek if this is needed

        }*/
        ItemCollector->bCurrentInteractionSuccess = true;
        log_debug("smashing success (the stack)\n");

        static auto DoVendDeath = StaticLoadObject<UFunction>("/Game/Athena/Items/Gameplay/VendingMachine/B_Athena_VendingMachine.B_Athena_VendingMachine_C.DoVendDeath");

        if (DoVendDeath)
        {
            ItemCollector->ProcessEvent(DoVendDeath, nullptr); 
            ItemCollector->K2_DestroyActor();
            //ItemCollector->K2_DestroyActor();
        }
    }
#endif
    //else if (ReceivingActor->IsA(AFortAthenaVehicle::StaticClass()))
    //{
    //    if (PC->MyFortPawn->IsInVehicle())
    //    {
    //        auto Vehicle = PC->MyFortPawn->GetVehicle();
    //        if (Vehicle)
    //        {
    //            int SeatIdx = PC->MyFortPawn->GetVehicleSeatIndex();
    //            auto WeaponComp = Vehicle->GetSeatWeaponComponent(SeatIdx);
    //            if (WeaponComp)
    //            {
    //                if (WeaponComp->WeaponSeatDefinitions.IsValidIndex(SeatIdx))
    //                {
    //                    auto& Def = WeaponComp->WeaponSeatDefinitions[SeatIdx];
    //                    if (Def.VehicleWeapon && Def.SeatIndex == SeatIdx)
    //                    {
    //                        GiveItem(PC, Def.VehicleWeapon, 1, 99999);
    //                        FFortItemEntry* Entry = FindEntry(PC, Def.VehicleWeapon); // BAD
    //                        if (Entry)
    //                        {
    //                            //PC->ServerExecuteInventoryItem(Entry->ItemGuid); // ClientEquipItem or else crash thanks
// 
    //                        }
    //                    }
    //                }
    //            }
    //        }
    //    }
    //}
}

void (*OnDamageServerOG)(ABuildingSMActor*, float, FGameplayTagContainer, FVector, FHitResult, AFortPlayerControllerAthena*, AActor*, FGameplayEffectContextHandle);
void OnDamageServer(ABuildingSMActor* Actor, float Damage, FGameplayTagContainer DamageTags, FVector Momentum, FHitResult HitInfo, AFortPlayerControllerAthena* InstigatedBy, AActor* DamageCauser, FGameplayEffectContextHandle EffectContext)
{
    bool FoundTag = false;
    //if (!InstigatedBy || Actor->bPlayerPlaced || Actor->GetHealth() == 1)
    //    return OnDamageServerOG(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

    if (!Actor || Actor->bDestroyed || !InstigatedBy || InstigatedBy->IsA(ABP_PhoebePlayerController_C::StaticClass()) || !DamageCauser)
        return OnDamageServerOG(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

    auto WeaponData = Cast<UFortWeaponMeleeItemDefinition>(((AFortWeapon*)DamageCauser)->WeaponData);

    if (!WeaponData)
        return OnDamageServerOG(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

    for (int32 /*size_t*/ i = 0; i < DamageTags.GameplayTags.Num(); i++)
    {
        if (DamageTags.GameplayTags[i].TagName.ComparisonIndex == PickaxeTagName.ComparisonIndex)
        {
            FoundTag = true;
            break;
        }
    }

    if (!FoundTag)
        return OnDamageServerOG(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

    auto ResourceItemDef = GetFortKismet()->K2_GetResourceItemDefinition(Actor->ResourceType);

    if (!ResourceItemDef)
        return OnDamageServerOG(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

    FCurveTableRowHandle& BuildingResourceAmountOverride = Actor->BuildingResourceAmountOverride;
    int ResCount = 0;

    if (Actor->BuildingResourceAmountOverride.RowName.ComparisonIndex > 0) {
        UCurveTable* CurveTable = /*GetGameState()->CurrentPlaylistInfo.BasePlaylist->ResourceRates.Get()*/ nullptr;

        if (!CurveTable)
            CurveTable = StaticLoadObject<UCurveTable>("/Game/Athena/Balance/DataTables/AthenaResourceRates.AthenaResourceRates");

        float Out;
        ((UDataTableFunctionLibrary*)UDataTableFunctionLibrary::StaticClass()->DefaultObject)->EvaluateCurveTableRow(CurveTable, Actor->BuildingResourceAmountOverride.RowName, 0.f, nullptr, &Out, FString());

        float RC = Out / (Actor->GetMaxHealth() / Damage);

        ResCount = (int)round(RC);
    }

    if (ResCount <= 0)
    {
        return OnDamageServerOG(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
    }
    bool Found = false;

    bool Weakspot = Damage == 100.f;
    InstigatedBy->ClientReportDamagedResourceBuilding(Actor, Actor->ResourceType, ResCount, false, Weakspot);

    for (int32 /*size_t*/ i = 0; i < InstigatedBy->WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
    {
        if (InstigatedBy->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition == ResourceItemDef)
        {
            Found = true;
            InstigatedBy->WorldInventory->Inventory.ReplicatedEntries[i].Count += ResCount;
            if (InstigatedBy->WorldInventory->Inventory.ReplicatedEntries[i].Count > 999)
            {
                SpawnPickup(InstigatedBy->Pawn->K2_GetActorLocation(), InstigatedBy->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition, InstigatedBy->WorldInventory->Inventory.ReplicatedEntries[i].Count - 999, 0, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset, InstigatedBy->MyFortPawn);
                InstigatedBy->WorldInventory->Inventory.ReplicatedEntries[i].Count = 999;
            }
            ModifyEntry(InstigatedBy, InstigatedBy->WorldInventory->Inventory.ReplicatedEntries[i]);
            UpdateInventory(InstigatedBy, &InstigatedBy->WorldInventory->Inventory.ReplicatedEntries[i]);
            break;
        }
    }

    if (!Found)
        GiveItem(InstigatedBy, ResourceItemDef, ResCount);

    return OnDamageServerOG(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
}


//imagine skidding pseudocode
__int64 GetAccID(__int64 Smth, __int64 a2)
{
    static  __int64 (*sub_7FF7C019DC30)(__int64, __int64) = decltype(sub_7FF7C019DC30)(__int64(GetModuleHandleA(0)) + 0x2C2DC30);
    __int64 v4 = 0; // rcx
    __int64 v5 = 0; // r14
    __int64 v6 = 0; // rax
    wchar_t* v7 = 0; // rbx
    wchar_t* v8 = 0; // rsi
    __int64 v9 = 0; // rax
    __int64 v10 = 0; // rax
    __int64* v11 = 0; // rcx
    char v12 = 0; // bl
    int v13 = 0; // eax
    __int64 v15 = 0; // [rsp+20h] [rbp-58h] BYREF
    __int64 v16 = 0; // [rsp+28h] [rbp-50h]
    //__int64 v17[2]; // [rsp+30h] [rbp-48h] BYREF /*ploosh:unrefed*/
    __int64 v18 = 0; // [rsp+40h] [rbp-38h] BYREF
    __int64 v19 = 0; // [rsp+50h] [rbp-28h] BYREF
    int v20 = 0; // [rsp+80h] [rbp+8h] BYREF

    v20 = 0;
    v4 = *(_QWORD*)(Smth + 8);
    if (v4 && (*(unsigned __int8(__fastcall**)(__int64))(*(_QWORD*)v4 + 40i64))(v4))
    {
        v5 = *(_QWORD*)(Smth + 8);
        v6 = (*(__int64(__fastcall**)(__int64, __int64*))(*(_QWORD*)v5 + 56i64))(v5, &v19);
        log_debug("%llx\n", __int64(*(_QWORD*)v5 + 56i64) - __int64(GetModuleHandleA(0)));
        wchar_t* TestChar = *(wchar_t**)(v5 + 24);
        log_debug("testchar %ls\n", TestChar);
        if (*(DWORD*)(v6 + 8))
            v8 = *(wchar_t**)v6;
        v9 = (*(__int64(__fastcall**)(__int64, int*))(*(_QWORD*)v5 + 16i64))(v5, &v20);
        v10 = sub_7FF7C019DC30(v9, (__int64)&v18);
        if (*(DWORD*)(v10 + 8))
            v7 = *(wchar_t**)v10;
        log_debug("test1 %ls\n", v8);
        log_debug("test2 %ls\n", v7);
    }

    *(_QWORD*)a2 = *v11;
    *v11 = 0i64;
    *(DWORD*)(a2 + 8) = *((DWORD*)v11 + 2);
    v13 = *((DWORD*)v11 + 3);
    v11[1] = 0i64;
    *(DWORD*)(a2 + 12) = v13;

    return a2;
}

bool CanBeDropped(UFortItemDefinition* Def)
{
    return !Def->IsA(UFortWeaponMeleeItemDefinition::StaticClass()) && (Def->IsA(UFortWeaponRangedItemDefinition::StaticClass()) || Def->IsA(UFortResourceItemDefinition::StaticClass()) || Def->IsA(UFortConsumableItemDefinition::StaticClass()) || Def->IsA(UFortAmmoItemDefinition::StaticClass()));
}