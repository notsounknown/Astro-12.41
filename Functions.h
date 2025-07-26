#pragma once
#include "pch.h"
#include "memcury.h"
#include "Version.h"

static inline uintptr_t FindBytes(Memcury::Scanner& Scanner, const std::vector<uint8_t>& Bytes, int Count = 255, int SkipBytes = 0, bool bGoUp = false, int Skip = 0, const bool bPrint = false)
{
	if (!Scanner.Get())
	{
		return 0;
	}

	auto Base = __int64(GetModuleHandleW(0));

	for (int i = 0 + SkipBytes; i < Count + SkipBytes; i++) // we should subtract from skip if goup
	{
		auto CurrentByte = *(Memcury::ASM::MNEMONIC*)(bGoUp ? Scanner.Get() - i : Scanner.Get() + i);

		if (bPrint)
			log_info("[%d] CurrentByte: 0x%x (0x%llx)\n", i, (int)CurrentByte, (bGoUp ? Scanner.Get() - i : Scanner.Get() + i) - Base);

		if (CurrentByte == Bytes[0])
		{
			bool Found = true;
			for (int j = 1; j < Bytes.size(); j++)
			{
				if (*(Memcury::ASM::MNEMONIC*)(bGoUp ? Scanner.Get() - i + j : Scanner.Get() + i + j) != Bytes[j])
				{
					Found = false;
					break;
				}
			}
			if (Found)
			{
				if (Skip > 0)
				{
					Skip--;
					continue;
				}

				return bGoUp ? Scanner.Get() - i : Scanner.Get() + i;
			}
		}

		// std::cout << std::format("CurrentByte: 0x{:x}\n", (uint8_t)CurrentByte);
	}

	return -1;// Scanner.Get();
}

uint64 FindStartAircraftPhase()
{
	if (Engine_Version < 4.27) // they scuf it
	{
		auto strRef = Memcury::Scanner::FindStringRef(L"STARTAIRCRAFT").Get();

		if (!strRef)
			return 0;

		int NumCalls = 0;

		for (int i = 0; i < 150; i++)
		{
			if (*(uint8_t*)(strRef + i) == 0xE8)
			{
				log_debug("Found call 0x%llx\n", __int64(strRef + i) - __int64(GetModuleHandleW(0)));
				NumCalls++;

				if (NumCalls == 2) // First is the str compare ig
				{
					return Memcury::Scanner(strRef + i).RelativeOffset(1).Get();
				}
			}
		}
	}
	else
	{
		auto StatAddress = Memcury::Scanner::FindStringRef(L"STAT_StartAircraftPhase").Get();

		for (int i = 0; i < 1000; i++)
		{
			if (*(uint8_t*)(uint8_t*)(StatAddress - i) == 0x48 && *(uint8_t*)(uint8_t*)(StatAddress - i + 1) == 0x8B && *(uint8_t*)(uint8_t*)(StatAddress - i + 2) == 0xC4)
			{
				return StatAddress - i;
			}
		}
	}

	return 0;
}

uint64 FindGetPlayerViewpoint()
{
	// We find FailedToSpawnPawn and then go back on VFT by 1.

	uint64 FailedToSpawnPawnAddr = 0;

	auto FailedToSpawnPawnStrRefAddr = Memcury::Scanner::FindStringRef(L"%s failed to spawn a pawn", true, 0, Fortnite_Version >= 19).Get();

	for (int i = 0; i < 1000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(FailedToSpawnPawnStrRefAddr - i) == 0x40 && *(uint8_t*)(uint8_t*)(FailedToSpawnPawnStrRefAddr - i + 1) == 0x53)
		{
			FailedToSpawnPawnAddr = FailedToSpawnPawnStrRefAddr - i;
			break;
		}

		if (*(uint8_t*)(uint8_t*)(FailedToSpawnPawnStrRefAddr - i) == 0x48 && *(uint8_t*)(uint8_t*)(FailedToSpawnPawnStrRefAddr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(FailedToSpawnPawnStrRefAddr - i + 2) == 0x5C)
		{
			FailedToSpawnPawnAddr = FailedToSpawnPawnStrRefAddr - i;
			break;
		}
	}

	if (!FailedToSpawnPawnAddr)
	{
		log_error("Failed to find FailedToSpawnPawn!\n");
		return 0;
	}

	static auto FortPlayerControllerAthenaDefault = AFortPlayerControllerAthena::StaticClass()->DefaultObject; // FindObject<UClass>(L"/Game/Athena/Athena_PlayerController.Default__Athena_PlayerController_C");
	//void** const PlayerControllerVFT = FortPlayerControllerAthenaDefault->Vft;
	void** PlayerControllerVFT = *(void***)FortPlayerControllerAthenaDefault;

	int FailedToSpawnPawnIdx = 0;

	for (int i = 0; i < 500; i++)
	{
		if (PlayerControllerVFT[i] == (void*)FailedToSpawnPawnAddr)
		{
			FailedToSpawnPawnIdx = i;
			break;
		}
	}

	if (FailedToSpawnPawnIdx == 0)
	{
		log_error("Failed to find FailedToSpawnPawn in virtual function table!\n");
		return 0;
	}

	return __int64(PlayerControllerVFT[FailedToSpawnPawnIdx - 1]);
}

static inline uint64 FindPickSupplyDropLocation()
{
	auto Addrr = Memcury::Scanner::FindStringRef(L"PickSupplyDropLocation: Failed to find valid location using rejection.  Using safe zone location.", true, 0).Get();

	if (!Addrr)
		return 0;

	// Newer versions it is "AFortAthenaMapInfo::PickSupplyDropLocation" (no wide str), but they also changed params so ill add later.

	for (int i = 0; i < 1000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x5C)
		{
			return Addrr - i;
		}
	}

	return 0;
}

static inline uint64 FindGetMaxTickRate() // UEngine::getmaxtickrate
{
	// TODO switch to index maybe?

	/* auto GetMaxTickRateIndex = *Memcury::Scanner::FindStringRef(L"GETMAXTICKRATE")
		.ScanFor({ 0x4D, 0x8B, 0xC7, 0xE8 })
		.RelativeOffset(4)
		.ScanFor({ 0xFF, 0x90 })
		.AbsoluteOffset(2)
		.GetAs<int*>() / 8;

	LOG_INFO(LogHook, "GetMaxTickRateIndex {}", GetMaxTickRateIndex); */

	if (Engine_Version == 5.00)
		return Memcury::Scanner::FindPattern("40 53 48 83 EC 50 0F 29 74 24 ? 48 8B D9 0F 29 7C 24 ? 0F 28 F9 44 0F 29").Get(); // the string is in func + it's in function chunks.

	if (Engine_Version == 4.27)
		return Memcury::Scanner::FindPattern("40 53 48 83 EC 60 0F 29 74 24 ? 48 8B D9 0F 29 7C 24 ? 0F 28").Get(); // function chunks woo!

	auto Addrr = Memcury::Scanner::FindStringRef(L"Hitching by request!").Get();

	if (!Addrr)
		return 0;

	for (int i = 0; i < 400; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x40 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x53)
		{
			return Addrr - i;
		}

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x5C)
		{
			return Addrr - i;
		}
	}

	return 0;
	// return FindBytes(stringRef, Fortnite_Version <= 4.1 ? std::vector<uint8_t>{ 0x40, 0x53 } : std::vector<uint8_t>{ 0x48, 0x89, 0x5C }, 1000, 0, true);
}

static inline uint64 FindOnDamageServer()
{
	auto Addr = FindFunctionCall(L"OnDamageServer",
		Engine_Version == 4.16 ? std::vector<uint8_t>{ 0x4C, 0x89, 0x4C } :
		Engine_Version == 4.19 || Engine_Version >= 4.27 ? std::vector<uint8_t>{ 0x48, 0x8B, 0xC4 } : std::vector<uint8_t>{ 0x40, 0x55 }
	);

	return Addr;
}

static inline uint64 FindTickFlush()
{
	// auto add = Memcury::Scanner::FindStringRef(L"UDemoNetDriver::TickFlush: ReplayStreamer ERROR: %s");
	// return Memcury::Scanner(FindBytes(add, { 0xE8 }, 500, 0, true, 1)).RelativeOffset(1).Get();

	if (Engine_Version == 4.16)
		return Memcury::Scanner::FindPattern("4C 8B DC 55 53 56 57 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 41 0F 29 7B").Get(); // 2.4.2

	if (Engine_Version == 4.19)
		return Memcury::Scanner::FindPattern("4C 8B DC 55 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 45 0F 29 43 ? 45 0F 29 4B ? 48 8B 05 ? ? ? ? 48").Get(); // 2.4.2

	if (Engine_Version == 4.27)
	{
		auto addr = Memcury::Scanner::FindPattern("48 8B C4 48 89 58 18 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 B8 0F 29 78 A8 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 8A", false).Get();

		if (!addr) // s18
			addr = Memcury::Scanner::FindPattern("48 8B C4 48 89 58 18 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 B8 0F 29 78 A8 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 44 0F", false).Get();

		if (!addr)
			addr = Memcury::Scanner::FindPattern("48 8B C4 48 89 58 18 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 B8 0F 29 78 A8 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 8B F9 48 89 4D 38 48 8D 4D 40").Get(); // 16.50

		return addr;
	}

	auto Addr = Memcury::Scanner::FindStringRef(L"STAT_NetTickFlush", false);

	if (!Addr.Get())
	{
		if (Engine_Version == 4.20) // 2.5
		{
			return Memcury::Scanner::FindPattern("4C 8B DC 55 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 45 0F 29 43 ? 45 0F 29 4B ? 48 8B 05 ? ? ? ? 48 33").Get();
		}
	}

	return FindBytes(Addr, (Fortnite_Version < 18 ? std::vector<uint8_t>{ 0x4C, 0x8B } : std::vector<uint8_t>{ 0x48, 0x8B, 0xC4 }), 1000, 0, true);
}

static inline uint64 FindGIsServer()
{
	// auto add = Memcury::Scanner::FindStringRef(L"STAT_UpdateLevelStreaming");
	// return Memcury::Scanner(FindBytes(add, { 0x80, 0x3D }, 100, 0, true, 1)).RelativeOffset(2).Get();

	// if (Fortnite_Version == 19.10)
		// return __int64(GetModuleHandleW(0)) + 0xB30CF9D;

	// if (Fortnite_Version == 2.5)
		// return __int64(GetModuleHandleW(0)) + 0x46AD735;

	auto Addrr = Memcury::Scanner::FindStringRef(L"AllowCommandletRendering").Get();

	/* int found = 0;

	for (int i = 0; i < 600; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x88 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x1D)
		{
			for (int z = 0; z < 15; z++)
			{
				LOG_INFO(LogDev, "[{}] [{}] GIsServerTest: 0x{:x}", found, z, Memcury::Scanner(Addrr - i).RelativeOffset(z).Get() - __int64(GetModuleHandleW(0)));
			}

			found++;
		}

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0xC6 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x05)
		{
			for (int z = 0; z < 15; z++)
			{
				LOG_INFO(LogDev, "[{}] [{}] GIsServerTest: 0x{:x}", found, z, Memcury::Scanner(Addrr - i).RelativeOffset(z).Get() - __int64(GetModuleHandleW(0)));
			}

			found++;
		}
	} */

	if (Fortnite_Version == 4.1)
		return __int64(GetModuleHandleW(0)) + 0x4BF6F18;
	if (Fortnite_Version == 10.40)
		return __int64(GetModuleHandleW(0)) + 0x637925C;
	if (Fortnite_Version == 12.41)
		return __int64(GetModuleHandleW(0)) + 0x804B65A;
	if (Fortnite_Version == 14.60)
		return __int64(GetModuleHandleW(0)) + 0x939930E;
	if (Fortnite_Version == 17.30)
		return __int64(GetModuleHandleW(0)) + 0x973E499;

	//return 0;

	auto Addr = Memcury::Scanner::FindStringRef(L"AllowCommandletRendering");

	std::vector<std::vector<uint8_t>> BytesArray = { { 0xC6, 0x05 }, { 0x88, 0x1D } };

	int Skip = 1;

	uint64 Addy;

	for (int i = 0; i < 50; i++) // we should subtract from skip if goup
	{
		auto CurrentByte = *(Memcury::ASM::MNEMONIC*)(Addr.Get() - i);

		// if (bPrint)
			// std::cout << "CurrentByte: " << std::hex << (int)CurrentByte << '\n';

		bool ShouldBreak = false;

		for (auto& Bytes : BytesArray)
		{
			if (CurrentByte == Bytes[0])
			{
				bool Found = true;
				for (int j = 1; j < Bytes.size(); j++)
				{
					if (*(Memcury::ASM::MNEMONIC*)(Addr.Get() - i + j) != Bytes[j])
					{
						Found = false;
						break;
					}
				}
				if (Found)
				{
					log_debug("[%d] Skip: 0x%llx\n", Skip, Memcury::Scanner(Addr.Get() - i).RelativeOffset(2).Get() - __int64(GetModuleHandleW(0)));

					if (Skip > 0)
					{
						Skip--;
						continue;
					}

					Addy = Addr.Get() - i;
					ShouldBreak = true;
					break;
				}
			}
		}

		if (ShouldBreak)
			break;

		// std::cout << std::format("CurrentByte: 0x{:x}\n", (uint8_t)CurrentByte);
	}

	/* int Skip = 2;
	auto Addy = FindBytes(Addr, { 0xC6, 0x05 }, 50, 0, true, Skip);
	Addy = Addy ? Addy : FindBytes(Addr, { 0x44, 0x88 }, 50, 0, true, Skip);
	Addy = Addy ? Addy : FindBytes(Addr, { 0x88, 0x1D }, 50, 0, true, Skip); */

	log_debug("Addy: 0x%llx\n", Addy - __int64(GetModuleHandleW(0)));

	return Memcury::Scanner(Addy).RelativeOffset(2).Get();
}

static inline uint64 FindGIsClient()
{
	/* if (Fortnite_Version >= 20)
		return 0; */

	auto Addr = Memcury::Scanner::FindStringRef(L"AllowCommandletRendering");

	std::vector<std::vector<uint8_t>> BytesArray = { {0x88, 0x05}, {0xC6, 0x05}, {0x88, 0x1D}, {0x44, 0x88} };

	int Skip = Engine_Version <= 4.20 ? 1 : 2;

	uint64 Addy;

	for (int i = 0; i < 50; i++) // we should subtract from skip if goup
	{
		auto CurrentByte = *(Memcury::ASM::MNEMONIC*)(Addr.Get() - i);

		// if (bPrint)
			// std::cout << "CurrentByte: " << std::hex << (int)CurrentByte << '\n';

		bool ShouldBreak = false;

		// LOG_INFO(LogDev, "[{}] Byte: 0x{:x}", i, (int)CurrentByte);

		for (auto& Bytes : BytesArray)
		{
			if (CurrentByte == Bytes[0])
			{
				bool Found = true;
				for (int j = 1; j < Bytes.size(); j++)
				{
					if (*(Memcury::ASM::MNEMONIC*)(Addr.Get() - i + j) != Bytes[j])
					{
						Found = false;
						break;
					}
				}
				if (Found)
				{
					int Relative = Bytes[0] == 0x44 ? 3 : 2;
					// LOG_INFO(LogDev, "[{}] No Rel 0x{:x} Rel: 0x{:x}", Skip, Memcury::Scanner(Addr.Get() - i).Get() - __int64(GetModuleHandleW(0)), Memcury::Scanner(Addr.Get() - i).RelativeOffset(Relative).Get() - __int64(GetModuleHandleW(0)));

					if (Skip > 0)
					{
						Skip--;
						continue;
					}

					Addy = Memcury::Scanner(Addr.Get() - i).RelativeOffset(Relative).Get();
					ShouldBreak = true;
					break;
				}
			}
		}

		if (ShouldBreak)
			break;

		// std::cout << std::format("CurrentByte: 0x{:x}\n", (uint8_t)CurrentByte);
	}

	// LOG_INFO(LogDev, "Addy: 0x{:x}", Addy - __int64(GetModuleHandleW(0)));

	return Addy; // 0; // Memcury::Scanner(Addy3).RelativeOffset(2).Get();

	/*
	auto Addr = Memcury::Scanner::FindStringRef(L"AllowCommandletRendering");
	int Skip = 1;
	auto Addy = FindBytes(Addr, { 0xC6, 0x05 }, 50, 0, true, Skip);
	Addy = Addy ? Addy : FindBytes(Addr, { 0x44, 0x88 }, 50, 0, true, Skip);
	Addy = Addy ? Addy : FindBytes(Addr, { 0x88, 0x1D }, 50, 0, true, Skip);

	return Memcury::Scanner(Addy).RelativeOffset(2).Get();
	*/
}

static inline uint64 FindCollectGarbage()
{
	// return 0;

	auto Addr = Memcury::Scanner::FindStringRef(L"STAT_CollectGarbageInternal");
	return FindBytes(Addr, { 0x48, 0x89, 0x5C }, 2000, 0, true, 1);
}

static inline uint64 FindActorGetNetMode()
{
	// return 0;

	if (Engine_Version == 500) // hah well this and 427 does like nothing cuz inline mostly
	{
		auto addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 48 83 EC 20 F6 41 08 10 48 8B D9 0F 85 ? ? ? ? 48 8B 41 20 48 85 C0 0F 84 ? ? ? ? F7 40", false).Get();

		if (!addr)
			addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 F6 41 08 10 48 8B D9 0F 85").Get(); // 20.40

		return addr;
	}

	if (Engine_Version == 427)
	{
		auto addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8B 93 ? ? ? ? 48 8B C8 48 8B F8 E8 ? ? ? ? 48 85 C0 75 29", false).Get();

		if (!addr)
			addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 48 83 EC 20 F6 41 08 10 48 8B D9 0F 85 ? ? ? ? 48 8B 41 20 48 85 C0 0F 84").Get(); // 17.50 & 18.40

		return addr;
	}

	auto AActorGetNetmodeStrRef = Memcury::Scanner::FindStringRef(L"STAT_ServerUpdateCamera", false);

	if (!AActorGetNetmodeStrRef.Get())
	{
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 48 83 EC 20 48 8B 01 48 8B D9 FF 90 ? ? ? ? 4C 8B").Get(); // 2.5 i think
	}

	return Memcury::Scanner(FindBytes(AActorGetNetmodeStrRef, { 0xE8 }, 255, 0, true)).RelativeOffset(1).Get();
}

static inline uint64 FindPickTeam()
{
	if (Engine_Version == 4.26)
	{
		auto testAddr = Memcury::Scanner::FindPattern("88 54 24 10 53 56 41 54 41 55 41 56 48 83 EC 60 4C 8B A1", false).Get(); // 14.60 what is happening lol ????

		if (!testAddr)
			testAddr = Memcury::Scanner::FindPattern("88 54 24 10 53 55 56 41 55 41 ? 48 83 EC 70 48", false).Get(); // 15.10 & 15.50

		if (testAddr)
			return testAddr;
	}

	else if (Engine_Version == 5.00)
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 88 54 24 10 55 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 70 45 33 ED 4D").Get(); // 19.10

	else if (Engine_Version >= 4.27) // different start
		return Memcury::Scanner::FindPattern("48 89 5C 24 ? 88 54 24 10 55 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 70 4C 8B A1").Get();

	if (Fortnite_Version == 7.20 || Fortnite_Version == 7.30)
		return Memcury::Scanner::FindPattern("89 54 24 10 53 56 41 54 41 55 41 56 48 81 EC").Get();

	auto Addr = Memcury::Scanner::FindStringRef(L"PickTeam for [%s] used beacon value [%d]", false, 0, Engine_Version >= 4.27); // todo check if its just s18+ but this doesn't matter for now cuz we hardcode sig

	if (!Addr.Get())
		Addr = Memcury::Scanner::FindStringRef(L"PickTeam for [%s] used beacon value [%s]"); // i don't even know what version this is

	return FindBytes(Addr, Fortnite_Version <= 4.1 ? std::vector<uint8_t>{ 0x48, 0x89, 0x6C } : std::vector<uint8_t>{ 0x40, 0x55 }, 1000, 0, true);
}

static inline uint64 FindInternalTryActivateAbility()
{
	auto Addrr = Memcury::Scanner::FindStringRef(L"InternalTryActivateAbility called with invalid Handle! ASC: %s. AvatarActor: %s", true, 0, Fortnite_Version >= 16).Get(); // checked 16.40

	for (int i = 0; i < 1000; i++)
	{
		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x48 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x8B && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0xC4)
		{
			return Addrr - i;
		}

		if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x4C && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(Addrr - i + 2) == 0x4C)
		{
			return Addrr - i;
		}
	}

	return 0;
	// return FindBytes(Addr, { 0x4C, 0x89, 0x4C }, 1000, 0, true);
}

static inline int FindInternalTryActivateAbility2() {
	int InternalServerTryActivateAbilityIndex = 0;
	static auto FortAbilitySystemComponentAthenaDefault = UFortAbilitySystemComponentAthena::StaticClass()->DefaultObject;
	auto vft = *(void***)FortAbilitySystemComponentAthenaDefault;

	if (Engine_Version > 4.20)
	{
		static auto OnRep_ReplicatedAnimMontageFn = UObject::FindObject<UFunction>("AbilitySystemComponent.OnRep_ReplicatedAnimMontage");
		InternalServerTryActivateAbilityIndex = ((int) GetFunctionIdxOrPtr(OnRep_ReplicatedAnimMontageFn) - 8) / 8;
	}
	else
	{
		static auto ServerTryActivateAbilityWithEventDataFn = UObject::FindObject<UFunction>("ServerTryActivateAbilityWithEventData");
		auto ServerTryActivateAbilityWithEventDataNativeAddr = __int64(vft[GetFunctionIdxOrPtr(ServerTryActivateAbilityWithEventDataFn) / 8]);

		for (int i = 0; i < 400; i++)
		{
			if ((*(uint8_t*)(ServerTryActivateAbilityWithEventDataNativeAddr + i) == 0xFF && *(uint8_t*)(ServerTryActivateAbilityWithEventDataNativeAddr + i + 1) == 0x90) || // call qword ptr
				(*(uint8_t*)(ServerTryActivateAbilityWithEventDataNativeAddr + i) == 0xFF && *(uint8_t*)(ServerTryActivateAbilityWithEventDataNativeAddr + i + 1) == 0x93)) // call qword ptr
			{
				InternalServerTryActivateAbilityIndex = (int) GetIndexFromVirtualFunctionCall(ServerTryActivateAbilityWithEventDataNativeAddr + i) / 8;
				break;
			}
		}
	}

	log_debug("InternalServerTryActivateAbilityIndex: 0x%x\n", InternalServerTryActivateAbilityIndex);
	return InternalServerTryActivateAbilityIndex;
}