#pragma once
#include "Engine.h"
#include "pch.h"
#include "memcury.h"


inline int FindOffsetStruct(const std::string& StructName, const std::string& MemberName, bool bWarnIfNotFound = true)
{
    UObject* Struct = StaticFindObject<UObject>(StructName);

    if (!Struct)
    {
        if (bWarnIfNotFound)
            log_warn("Unable to find struct %s\n", StructName.c_str());

        return 0;
    }

    // LOG_INFO(LogFinder, "Struct: {}", Struct->GetFullName());

    for (auto CurrentClass = Struct; CurrentClass; CurrentClass = *(UObject**)(__int64(CurrentClass) + 0x40))
    {
        void* Property = *(void**)(__int64(CurrentClass) + 0x50);

        if (Property)
        {
            std::string PropName = ((FName*)(__int64(Property) + 0x28))->ToString();

            if (PropName == MemberName)
            {
                return *(int*)(__int64(Property) + 0x4c);
            }

            while (Property)
            {
                // LOG_INFO(LogFinder, "PropName: {}", PropName);

                if (PropName == MemberName)
                {
                    return *(int*)(__int64(Property) + 0x4c);
                }

                Property = *(void**)(__int64(Property) + 0x20);
                PropName = Property ? ((FName*)(__int64(Property) + 0x28))->ToString() : "";
            }
        }
    }

    log_warn("Unable to find %s\n", MemberName.c_str());

    return -1;
}


void* GetProperty(UObject* object, const std::string& ChildName, bool bWarnIfNotFound)
{
    for (auto CurrentClass = object->Class; CurrentClass; CurrentClass = *(UClass**)(__int64(CurrentClass) + 0x40))
    {
        void* Property = *(void**)(__int64(CurrentClass) + 0x50);

        if (Property)
        {
            // LOG_INFO(LogDev, "Reading prop name..");

            std::string PropName = ((FName*)(__int64(Property) + 0x28))->ToString();

            // LOG_INFO(LogDev, "PropName: {}", PropName);

            if (PropName == ChildName)
            {
                return Property;
            }

            while (Property)
            {
                if (PropName == ChildName)
                {
                    return Property;
                }

                Property = *(void**)(__int64(Property) + 0x20);
                PropName = Property ? ((FName*)(__int64(Property) + 0x28))->ToString() : "";
            }
        }
    }

    //if (bWarnIfNotFound)
    log_warn("Unable to find %s\n", ChildName.c_str());

    return nullptr;
}

int GetClipSize(UFortWeaponItemDefinition* WeaponItemDefinition) {
    int ClipSize = 0;

    auto Table = WeaponItemDefinition->WeaponStatHandle.DataTable;
    if (!Table)
        goto end_clblock;
    else {

        auto& RowMap = Table->RowMap;

        void* Row = nullptr;

        for (int i = 0; i < RowMap.Pairs.Elements.Data.Num(); ++i)
        {
            TPair<FName, uint8_t*>& Pair = (TPair<FName, uint8_t*>&) RowMap.Pairs.Elements.Data[i].ElementData.Value;

            if (Pair.First/*Key()*/ == WeaponItemDefinition->WeaponStatHandle.RowName)
            {
                Row = Pair.Second/*Value()*/;
                break;
            }
        }

        if (!Row)
            goto end_clblock;

        static auto ClipSizeOffset = FindOffsetStruct("/Script/FortniteGame.FortBaseWeaponStats", "ClipSize");
        ClipSize = *(int*)(__int64(Row) + ClipSizeOffset);
    }

end_clblock:
    return ClipSize;
}

FFortItemEntry* MakeItemEntry(UFortItemDefinition* ItemDefinition, int Count = 1, int LoadedAmmo = 0, float Durability = 0x3F800000, int Level = 0)
{
    auto Entry = (FFortItemEntry*)VirtualAlloc(0, /*(int*)((int64_t)StaticFindObject<UStruct>("/Script/FortniteGame.FortItemEntry") + 0x58)*/ 0x144, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    if (!Entry)
        return nullptr;

    if (LoadedAmmo == -1)
    {
        if (auto WeaponDef = Cast<UFortWeaponItemDefinition>(ItemDefinition)) // bPreventDefaultPreload ?
            LoadedAmmo = GetClipSize(WeaponDef);
        else
            LoadedAmmo = 0;
    }

    Entry->MostRecentArrayReplicationKey = -1; // idk if we need to set this
    Entry->ReplicationID = -1;
    Entry->ReplicationKey = -1;

    Entry->ItemDefinition = ItemDefinition;
    Entry->Count = Count;
    Entry->LoadedAmmo = LoadedAmmo;
    Entry->Durability = Durability;
    Entry->GameplayAbilitySpecHandle = FGameplayAbilitySpecHandle(-1);
    Entry->ParentInventory.ObjectIndex = -1;
    Entry->Level = Level;
    // We want to add StateValues.Add(DurabilityInitialized); orwnatefc erwgearf yk
    // CoCreateGuid((GUID*)&Entry->GetItemGuid());
    // Entry->DoesUpdateStatsOnCollection() = true; // I think fortnite does this?

    return Entry;
}


enum class EInternalObjectFlags : int
{
    None = 0,

    LoaderImport = 1 << 20, ///< Object is ready to be imported by another package during loading
    Garbage = 1 << 21, ///< Garbage from logical point of view and should not be referenced. This flag is mirrored in EObjectFlags as RF_Garbage for performance
    PersistentGarbage = 1 << 22, ///< Same as above but referenced through a persistent reference so it can't be GC'd
    ReachableInCluster = 1 << 23, ///< External reference to object in cluster exists
    ClusterRoot = 1 << 24, ///< Root of a cluster
    Native = 1 << 25, ///< Native (UClass only). 
    Async = 1 << 26, ///< Object exists only on a different thread than the game thread.
    AsyncLoading = 1 << 27, ///< Object is being asynchronously loaded.
    Unreachable = 1 << 28, ///< Object is not reachable on the object graph.
    // PendingKill UE_DEPRECATED(5.0, "PendingKill flag should no longer be used. Use Garbage flag instead.") = 1 << 29, ///< Objects that are pending destruction (invalid for gameplay but valid objects). This flag is mirrored in EObjectFlags as RF_PendingKill for performance
    PendingKill = 1 << 29,
    RootSet = 1 << 30, ///< Object will not be garbage collected, even if unreferenced.
    PendingConstruction = 1 << 31 ///< Object didn't have its class constructor called yet (only the UObjectBase one to initialize its most basic members)

    /* GarbageCollectionKeepFlags = Native | Async | AsyncLoading | LoaderImport,
    PRAGMA_DISABLE_DEPRECATION_WARNINGS
    MirroredFlags = Garbage | PendingKill, /// Flags mirrored in EObjectFlags

    //~ Make sure this is up to date!
    AllFlags = LoaderImport | Garbage | PersistentGarbage | ReachableInCluster | ClusterRoot | Native | Async | AsyncLoading | Unreachable | PendingKill | RootSet | PendingConstruction
    PRAGMA_ENABLE_DEPRECATION_WARNINGS */
};


struct FUObjectItem
{
    UObject* Object;
    int32 Flags;
    int32 ClusterRootIndex;
    int32 SerialNumber;

    bool IsPendingKill() const
    {
        return !!(Flags & int32(EInternalObjectFlags::PendingKill));
    }

    void SetFlag(EInternalObjectFlags FlagToSet)
    {
        // static_assert(sizeof(int32) == sizeof(Flags), "Flags must be 32-bit for atomics.");
        int32 StartValue = int32(Flags);

        if ((StartValue & int32(FlagToSet)) == int32(FlagToSet))
        {
            return;
        }

        int32 NewValue = StartValue | int32(FlagToSet);
    }

    void SetRootSet()
    {
        SetFlag(EInternalObjectFlags::RootSet);
    }
};

class FChunkedFixedUObjectArray
{
    // enum { NumElementsPerChunk = 64 * 1024, };

    FUObjectItem** Objects;
    FUObjectItem* PreAllocatedObjects;
    int32 MaxElements;
    int32 NumElements;
    int32 MaxChunks;
    int32 NumChunks;
public:
    int32 Num() const { return NumElements; }
    int32 Capacity() const { return MaxElements; }
    bool IsValidIndex(int32 Index) const { return Index < Num() && Index >= 0; }

    FUObjectItem* GetItemByIndex(int32 Index)
    {
        if (!IsValidIndex(Index)) return nullptr;

        const int32 ChunkIndex = Index / 0x10000;
        const int32 WithinChunkIndex = Index % 0x10000;

        // checkf(ChunkIndex < NumChunks, TEXT("ChunkIndex (%d) < NumChunks (%d)"), ChunkIndex, NumChunks);
        // checkf(Index < MaxElements, TEXT("Index (%d) < MaxElements (%d)"), Index, MaxElements);
        FUObjectItem* Chunk = Objects[ChunkIndex];

        if (!Chunk)
            return nullptr;

        return Chunk + WithinChunkIndex;
    }

    bool IsValid(UObject* Object)
    {
        int32 Index = Object->Index;
        if (Index == -1)
        {
            // UE_LOG(LogUObjectArray, Warning, TEXT("Object is not in global object array"));
            return false;
        }
        if (!IsValidIndex(Index))
        {
            // UE_LOG(LogUObjectArray, Warning, TEXT("Invalid object index %i"), Index);
            return false;
        }

        FUObjectItem* Slot = GetItemByIndex(Index);
        if (!Slot || Slot->Object == nullptr)
        {
            // UE_LOG(LogUObjectArray, Warning, TEXT("Empty slot"));
            return false;
        }
        if (Slot->Object != Object)
        {
            // UE_LOG(LogUObjectArray, Warning, TEXT("Other object in slot"));
            return false;
        }
        return true;
    }

    UObject* GetObjectByIndex(int32 Index)
    {
        if (auto Item = GetItemByIndex(Index))
            return Item->Object;

        return nullptr;
    }
};

uint64_t ObjectArray = (int64_t) GetModuleHandleA(0) + 0x8067df0;
FChunkedFixedUObjectArray* ChunkedObjects = decltype(ChunkedObjects)(ObjectArray);

FUObjectItem* GetItemByIndex(int32 Index)
{
    return ChunkedObjects ? ChunkedObjects->GetItemByIndex(Index) : nullptr;
}

std::map<UFunction*, __int64> ICache = {};
std::map<UFunction*, __int64> ICache2 = {};


inline __int64 GetFunctionIdxOrPtr2(UFunction* Function)
{
    if (ICache2[Function])
        return ICache2[Function];
    auto NativeAddr = __int64(*(void**)(__int64(Function) + 0xF0));

    auto FuncName = Function->GetName();

    std::wstring ValidateWStr = (std::wstring(FuncName.begin(), FuncName.end()) + L"_Validate");
    const wchar_t* ValidateWCStr = ValidateWStr.c_str();
    bool bHasValidateFunc = Memcury::Scanner::FindStringRef(ValidateWCStr, false).Get();

    bool bFoundValidate = !bHasValidateFunc;

    __int64 RetAddr = 0;

    for (int i = 0; i < 2000; i++)
    {
        // std::cout << std::format("CURRENT 0x{:x}\n", __int64((uint8_t*)(NativeAddr + i)) - __int64(GetModuleHandleW(0)));

        if (!RetAddr && *(uint8_t*)(NativeAddr + i) == 0xC3)
        {
            RetAddr = NativeAddr + i;
            break;
        }
    }

    int i = 0;

    __int64 functionAddyOrOffset = 0;

    for (__int64 CurrentAddy = RetAddr; CurrentAddy != NativeAddr && i < 2000; CurrentAddy -= 1) // Find last call
    {
        // LOG_INFO(LogDev, "[{}] 0x{:x}", i, *(uint8_t*)CurrentAddy);

        /* if (*(uint8_t*)CurrentAddy == 0xE8) // BAD
        {
            // LOG_INFO(LogDev, "CurrentAddy 0x{:x}", CurrentAddy - __int64(GetModuleHandleW(0)));
            functionAddyOrOffset = (CurrentAddy + 1 + 4) + *(int*)(CurrentAddy + 1);
            break;
        }

        else */ if ((*(uint8_t*)(CurrentAddy) == 0xFF && *(uint8_t*)(CurrentAddy + 1) == 0x90) ||
            (*(uint8_t*)(CurrentAddy) == 0xFF && *(uint8_t*)(CurrentAddy + 1) == 0x93) ||
            (*(uint8_t*)(CurrentAddy) == 0xFF && *(uint8_t*)(CurrentAddy + 1) == 0x92))
        {
            auto SecondByte = *(uint8_t*)(CurrentAddy + 2);
            auto ThirdByte = *(uint8_t*)(CurrentAddy + 3);

            std::string bytes = GetBytes(CurrentAddy + 2, 2);

            std::string last2bytes;
            last2bytes += bytes[3];
            last2bytes += bytes[4];

            std::string neww;

            if (last2bytes != "00")
                neww = last2bytes;

            neww += bytes[0];
            neww += bytes[1];
            bytes = neww;

            functionAddyOrOffset = /*HexToDec(bytes)*/ strtoull(bytes.c_str(), nullptr, 16);
            break;
        }

        i++;
    }

    if (functionAddyOrOffset) ICache2[Function] = functionAddyOrOffset;
    return functionAddyOrOffset;
}

inline __int64 GetIndexFromVirtualFunctionCall(__int64 NativeAddr)
{
    std::string wtf = "";

    int shots = 0;

    bool bFoundFirstNumber = false;

    for (__int64 z = (NativeAddr + 5); z != (NativeAddr + 1); z -= 1)
    {
        auto anafa = (int)(*(uint8_t*)z);

        auto asfk = anafa < 10 ? "0" + std::format("{:x}", anafa) : std::format("{:x}", anafa);

        // std::cout << std::format("[{}] 0x{}\n", shots, asfk);

        if (*(uint8_t*)z == 0 ? bFoundFirstNumber : true)
        {
            wtf += asfk;
            bFoundFirstNumber = true;
        }

        shots++;
    }

    //std::transform(wtf.begin(), wtf.end(), wtf.begin(), ::toupper);

    //log_debug("burgerfn2: %s\n", wtf.c_str());

    // LOG_INFO(LogDev, "wtf: {}", wtf);

    return strtoull(wtf.c_str(), nullptr, 16);
}

inline __int64 GetFunctionIdxOrPtr(UFunction* Function, bool bBreakWhenHitRet = false)
{
    if (!Function)
        return 0;
    if (ICache[Function])
        return ICache[Function];

    auto NativeAddr = __int64(*(void**)(__int64(Function) + 0xF0));

    auto FuncName = Function->GetName();

    std::wstring ValidateWStr = (std::wstring(FuncName.begin(), FuncName.end()) + L"_Validate");
    const wchar_t* ValidateWCStr = ValidateWStr.c_str();
    bool bHasValidateFunc = Memcury::Scanner::FindStringRef(ValidateWCStr, false).Get();

    // LOG_INFO(LogDev, "[{}] bHasValidateFunc: {}", Function->GetName(), bHasValidateFunc);
    // LOG_INFO(LogDev, "NativeAddr: 0x{:x}", __int64(NativeAddr) - __int64(GetModuleHandleW(0)));

    bool bFoundValidate = !bHasValidateFunc;

    __int64 RetAddr = 0;

    for (int i = 0; i < 2000; i++)
    {
        // LOG_INFO(LogDev, "0x{:x} {}", *(uint8_t*)(NativeAddr + i), bFoundValidate);


        if ((*(uint8_t*)(NativeAddr + i) == 0xFF && *(uint8_t*)(NativeAddr + i + 1) == 0x90) || // call qword ptr
             *(uint8_t*)(NativeAddr + i) == 0xFF && *(uint8_t*)(NativeAddr + i + 1) == 0x93) // call qword ptr
        {
            if (bFoundValidate) {
                return GetIndexFromVirtualFunctionCall(NativeAddr + i);
            }
            else
            {
                bFoundValidate = true;
                continue;
            }
        }

        if (*(uint8_t*)(NativeAddr + i) == 0x48 && *(uint8_t*)(NativeAddr + i + 1) == 0xFF && *(uint8_t*)(NativeAddr + i + 2) == 0xA0) // jmp qword ptr
        {
            if (bFoundValidate)
            {
                std::string wtf = "";

                int shots = 0;

                bool bFoundFirstNumber = false;

                for (__int64 z = (NativeAddr + i + 6); z != (NativeAddr + i + 2); z -= 1)
                {
                    auto anafa = (int)(*(uint8_t*)z);

                    auto asfk = anafa < 10 ? "0" + std::format("{:x}", anafa) : std::format("{:x}", anafa);

                    // std::cout << std::format("[{}] 0x{}\n", shots, asfk);

                    if (*(uint8_t*)z == 0 ? bFoundFirstNumber : true)
                    {
                        wtf += asfk;
                        bFoundFirstNumber = true;
                    }

                    shots++;
                }

                //std::transform(wtf.begin(), wtf.end(), wtf.begin(), ::toupper);

                // LOG_INFO(LogDev, "wtf: {}", wtf);


                return strtoull(wtf.c_str(), nullptr, 16);
            }
        }

        if (!RetAddr && *(uint8_t*)(NativeAddr + i) == 0xC3)
        {
            RetAddr = NativeAddr + i;

            if (bBreakWhenHitRet)
                break;
        }
    }

    // The function isn't virtual

    __int64 functionAddy = 0;

    // LOG_INFO(LogDev, "not virtgual");

    if (RetAddr)
    {
        // LOG_INFO(LogDev, "RetAddr 0x{:x}", RetAddr - __int64(GetModuleHandleW(0)));

        int i = 0;

        for (__int64 CurrentAddy = RetAddr; CurrentAddy != NativeAddr && i < 2000; CurrentAddy -= 1) // Find last call
        {
            // LOG_INFO(LogDev, "[{}] 0x{:x}", i, *(uint8_t*)CurrentAddy);

            if (*(uint8_t*)CurrentAddy == 0xE8)
            {
                // LOG_INFO(LogDev, "CurrentAddy 0x{:x}", CurrentAddy - __int64(GetModuleHandleW(0)));
                functionAddy = (CurrentAddy + 1 + 4) + *(int*)(CurrentAddy + 1);

                for (int i = 0; i < 2000; i++)
                {
                    // LOG_INFO(LogDev, "0x{:x} {}", *(uint8_t*)(NativeAddr + i), bFoundValidate);


                    if ((*(uint8_t*)(functionAddy + i) == 0xFF && *(uint8_t*)(functionAddy + i + 1) == 0x90) || // call qword ptr
                        *(uint8_t*)(functionAddy + i) == 0xFF && *(uint8_t*)(functionAddy + i + 1) == 0x93) // call qword ptr
                    {
                        return GetIndexFromVirtualFunctionCall(functionAddy + i);
                    }
                }
                break;
            }

            i++;
        }
    }

    if (functionAddy) ICache[Function] = functionAddy;
    return !functionAddy ? -1 : functionAddy;
}

inline __int64 GetFunctionIdx(UFunction* Function)
{
    if (!Function)
        return 0;
    if (ICache[Function])
        return ICache[Function];

    auto NativeAddr = __int64(*(void**)(__int64(Function) + 0xF0));

    auto FuncName = Function->GetName();

    std::wstring ValidateWStr = (std::wstring(FuncName.begin(), FuncName.end()) + L"_Validate");
    const wchar_t* ValidateWCStr = ValidateWStr.c_str();
    uintptr_t validateFunc = Memcury::Scanner::FindStringRef(ValidateWCStr, false).Get();

    // LOG_INFO(LogDev, "[{}] bHasValidateFunc: {}", Function->GetName(), bHasValidateFunc);
    // LOG_INFO(LogDev, "NativeAddr: 0x{:x}", __int64(NativeAddr) - __int64(GetModuleHandleW(0)));

    if (!validateFunc) return 0;

    auto currentAddy = validateFunc;
    bool foundCall = false;
    bool followedCall = false;

    for (int i = 0; i < 40; i++) {
        if (*(uint8_t*)(validateFunc + i) == 0xE8) // call
        {
            foundCall = true;
            currentAddy = validateFunc + i + 5;
            break;
        }
    }

    if (foundCall) {
        for (int i = 0; i < 250; i++) {
            if (*(uint8_t*)(currentAddy + i) == 0xE8) // call
            {
                auto offset = *(int32_t*)(currentAddy + i + 1);

                currentAddy = currentAddy + i + 5 + offset;
                followedCall = true;
                break;
            }
        }
    }

    for (int i = 0; i < 2000; i++)
    {
        // LOG_INFO(LogDev, "0x{:x} {}", *(uint8_t*)(NativeAddr + i), bFoundValidate);

        if ((*(uint8_t*)(currentAddy + i) == 0xFF && *(uint8_t*)(currentAddy + i + 1) == 0x90) || // call qword ptr
            (followedCall ? (*(uint8_t*)(currentAddy + i) == 0xFF && *(uint8_t*)(currentAddy + i + 1) == 0x92) : false) || // call qword ptr
            *(uint8_t*)(currentAddy + i) == 0xFF && *(uint8_t*)(currentAddy + i + 1) == 0x93) // call qword ptr
        {
             return GetIndexFromVirtualFunctionCall(currentAddy + i);
        }

        if (*(uint8_t*)(NativeAddr + i) == 0x48 && *(uint8_t*)(NativeAddr + i + 1) == 0xFF && *(uint8_t*)(NativeAddr + i + 2) == 0xA0) // jmp qword ptr
        {
            if (!validateFunc)
            {
                std::string wtf = "";

                int shots = 0;

                bool bFoundFirstNumber = false;

                for (__int64 z = (NativeAddr + i + 6); z != (NativeAddr + i + 2); z -= 1)
                {
                    auto anafa = (int)(*(uint8_t*)z);

                    auto asfk = anafa < 10 ? "0" + std::format("{:x}", anafa) : std::format("{:x}", anafa);

                    // std::cout << std::format("[{}] 0x{}\n", shots, asfk);

                    if (*(uint8_t*)z == 0 ? bFoundFirstNumber : true)
                    {
                        wtf += asfk;
                        bFoundFirstNumber = true;
                    }

                    shots++;
                }

                //std::transform(wtf.begin(), wtf.end(), wtf.begin(), ::toupper);

                // LOG_INFO(LogDev, "wtf: {}", wtf);


                return strtoull(wtf.c_str(), nullptr, 16);
            }
        }
    }

    return 0;
}

#define VTableOff(Function) (int) (((GetFunctionIdxOrPtr2(Function) > 0 && GetFunctionIdxOrPtr(Function) > 0) ? \
    ((GetFunctionIdxOrPtr(Function) - GetFunctionIdxOrPtr2(Function) == -8) ? GetFunctionIdxOrPtr2(Function) + 8 : GetFunctionIdxOrPtr(Function)) : \
    (GetFunctionIdxOrPtr2(Function) > 0 ? GetFunctionIdxOrPtr2(Function) + 16 : GetFunctionIdxOrPtr(Function) + 8)) >> 3)
//#define VTableOff(Function) (int) (GetFunctionIdx(Function) >> 3)
#define FOff(Function) (LPVOID) GetFunctionIdxOrPtr(Function)
#define FOff2(Function) (LPVOID) GetFunctionIdxOrPtr2(Function)

/*static bool IsPrimaryQuickbar(UFortItemDefinition* ItemDefinition)
{
    /* if (ItemDefinition->IsA(UFortDecoItemDefinition::StaticClass()))
    {
        if (ItemDefinition->IsA(UFortTrapItemDefinition::StaticClass()))
            return false;
        else
            return true;
    }
    else if (ItemDefinition->IsA(UFortWeaponItemDefinition::StaticClass()))
        return true; * /

    static auto FortWeaponMeleeItemDefinitionClass = UObject::FindObject<UClass>("/Script/FortniteGame.FortWeaponMeleeItemDefinition");
    static auto FortEditToolItemDefinitionClass = UObject::FindObject<UClass>("/Script/FortniteGame.FortEditToolItemDefinition");
    static auto FortBuildingItemDefinitionClass = UObject::FindObject<UClass>("/Script/FortniteGame.FortBuildingItemDefinition");
    static auto FortAmmoItemDefinitionClass = UObject::FindObject<UClass>("/Script/FortniteGame.FortAmmoItemDefinition");
    static auto FortResourceItemDefinitionClass = UObject::FindObject<UClass>("/Script/FortniteGame.FortResourceItemDefinition");
    static auto FortTrapItemDefinitionClass = UObject::FindObject<UClass>("/Script/FortniteGame.FortTrapItemDefinition");

    if (!ItemDefinition->IsA(FortWeaponMeleeItemDefinitionClass) && !ItemDefinition->IsA(FortEditToolItemDefinitionClass) &&
        !ItemDefinition->IsA(FortBuildingItemDefinitionClass) && !ItemDefinition->IsA(FortAmmoItemDefinitionClass)
        && !ItemDefinition->IsA(FortResourceItemDefinitionClass) && !ItemDefinition->IsA(FortTrapItemDefinitionClass))
        return true;

    return false;

}*/