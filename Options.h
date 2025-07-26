#pragma once
#include "PCH.h"

enum class ELoadoutType : uint8
{
    LateGame = 0,
    CustomLTM = 1,
    MAX = 2
};

struct Loadout
{
    map<UFortItemDefinition*, vector<int>> ItemDefinitions{};
    ELoadoutType LoadoutType;
};

vector<Loadout> Loadouts{};
int MaxPlayersPerTeam = 1;
int MaxSquadSize = 1;
int CurrentPlayersOnTeam = 0;
uint8 NextIdx = 3;
uint8 FirstTeam = 3;
uint8 LastTeam = 3;
bool bIsLargeTeamGame = false;
bool bEnableLooting = false;
bool bArsenal = false;
bool bStormKing = false;
bool bSkipAircraft = false;
bool bSolos = false;
// EXPERIMENTAL, DO NOT USE UNLESS YOU KNOW WHAT YOU ARE DOING
//bool bNoTeams = true;
bool bSkipWarmup = false;
bool bBattleLab = false;
bool bLateGame = false;
bool bCreative = false;
bool bTravis = false;
bool bDropZone = false;
bool bTeamRumble = false;
bool bEnableScoringSystem = false;
bool bEnableSiphon = false;
AFortAIPawn* DadBroPawn = nullptr;
#ifdef MineekPrivate
bool BotsEnabled = true;
#elif MineekPublic
bool BotsEnabled = true;
#else
bool BotsEnabled = false;
#endif

vector<string> AdminNames{ "ploosh", "itz.taking0ver" };//real real very secure
vector<string> AdminIps{ "127.0.0.1" };

static float XpToGivePerKill = 100.f;
static float XpMultiplierTime = 10.f;

// 60.0f = 1 min
#ifdef VPS
static float Duration = 120.f;
#elif MineekPrivate
static float Duration = 60.f;
#elif MineekPublic
static float Duration = 60.f;
#else
static float Duration = 69420.f;
#endif
#ifdef ONE_SHOT
static int Port = 7777;
#else
static int Port = 7777;
#endif