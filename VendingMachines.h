//#ifdef _VM
#if true
#pragma once

#include "Engine.h"
#include "pch.h"
#include "Utils.h"
#include "Pawn.h"

struct LootDrop
{
    FFortItemEntry* ItemEntry;

    FFortItemEntry* operator->() {
        return ItemEntry;
    }

    ~LootDrop()
    {

    }
};

static inline float RandomFloatForLoot(float AllWeightsSum)
{
    return (rand() * 0.000030518509f) * AllWeightsSum;
}

template <typename KeyType, typename ValueType>
static ValueType PickWeightedElement(const std::map<KeyType, ValueType>& Elements,
    std::function<float(ValueType)> GetWeightFn,
    std::function<float(float)> RandomFloatGenerator = RandomFloatForLoot,
    float TotalWeightParam = -1, bool bCheckIfWeightIsZero = false, int RandMultiplier = 1, KeyType* OutName = nullptr, bool bPrint = false, bool bKeepGoingUntilWeGetValue = false)
{
    float TotalWeight = TotalWeightParam;

    if (TotalWeight == -1)
    {
        TotalWeight = std::accumulate(Elements.begin(), Elements.end(), 0.0f, [&](float acc, const std::pair<KeyType, ValueType>& p) {
            auto Weight = GetWeightFn(p.second);

            if (bPrint)
            {
                // if (Weight != 0)
                {
                    log_debug("Adding weight %f\n", Weight);
                }
            }

            return acc + Weight;
            });
    }

    float RandomNumber = // UKismetMathLibrary::RandomFloatInRange(0, TotalWeight);
        RandMultiplier * RandomFloatGenerator(TotalWeight);

    if (bPrint)
    {
        log_debug("RandomNumber: %f TotalWeight: %f Elements.size(): %zd\n", RandomNumber, TotalWeight, Elements.size());
    }

    for (auto& Element : Elements)
    {
        float Weight = GetWeightFn(Element.second);

        if (bCheckIfWeightIsZero && Weight == 0)
            continue;

        if (RandomNumber <= Weight)
        {
            if (OutName)
                *OutName = Element.first;

            return Element.second;
        }

        RandomNumber -= Weight;
    }

    if (bKeepGoingUntilWeGetValue)
        return PickWeightedElement<KeyType, ValueType>(Elements, GetWeightFn, RandomFloatGenerator, TotalWeightParam, bCheckIfWeightIsZero, RandMultiplier, OutName, bPrint, bKeepGoingUntilWeGetValue);

    return ValueType();
}



template <typename KeyType, typename ValueType>
static ValueType PickWeightedElement(const std::unordered_map<KeyType, ValueType>& Elements,
    std::function<float(ValueType)> GetWeightFn,
    std::function<float(float)> RandomFloatGenerator = RandomFloatForLoot,
    float TotalWeightParam = -1, bool bCheckIfWeightIsZero = false, int RandMultiplier = 1, KeyType* OutName = nullptr, bool bPrint = false, bool bKeepGoingUntilWeGetValue = false)
{
    float TotalWeight = TotalWeightParam;

    if (TotalWeight == -1)
    {
        TotalWeight = std::accumulate(Elements.begin(), Elements.end(), 0.0f, [&](float acc, const std::pair<KeyType, ValueType>& p) {
            auto Weight = GetWeightFn(p.second);

            //if (bPrint)
            {
                // if (Weight != 0)
                {
                    log_debug("Adding weight %f\n", Weight);
                }
            }

            return acc + Weight;
            });
    }

    float RandomNumber = // UKismetMathLibrary::RandomFloatInRange(0, TotalWeight);
        RandMultiplier * RandomFloatGenerator(TotalWeight);

    //if (bPrint)
    {
        log_debug("RandomNumber: %f TotalWeight: %f Elements.size(): %d\n", RandomNumber, TotalWeight, Elements.size());
    }

    for (auto& Element : Elements)
    {
        float Weight = GetWeightFn(Element.second);

        if (bCheckIfWeightIsZero && Weight == 0)
            continue;

        if (RandomNumber <= Weight)
        {
            if (OutName)
                *OutName = Element.first;

            return Element.second;
        }

        RandomNumber -= Weight;
    }

    if (bKeepGoingUntilWeGetValue)
        return PickWeightedElement<KeyType, ValueType>(Elements, GetWeightFn, RandomFloatGenerator, TotalWeightParam, bCheckIfWeightIsZero, RandMultiplier, OutName, bPrint, bKeepGoingUntilWeGetValue);

    return ValueType();
}


template <typename RowStructType = uint8>
void CollectDataTablesRows(const std::vector<UDataTable*>& DataTables, std::map<FName, RowStructType*>* OutMap/*std::function<void(FName, RowStructType*)> AddToMap = []() {}*/, std::function<bool(FName, RowStructType*)> Check = []() { return true; })
{
    std::vector<UDataTable*> DataTablesToIterate;

    //static auto CompositeDataTableClass = StaticLoadObject<UClass>("/Script/Engine.CompositeDataTable");

    for (UDataTable* DataTable : DataTables)
    {

        // if (auto CompositeDataTable = Cast<UCompositeDataTable>(DataTable))
        if (DataTable->IsA(UCompositeDataTable::StaticClass()))
        {
            auto CompositeDataTable = (UCompositeDataTable *) DataTable;

            //static auto ParentTablesOffset = GetOffset(CompositeDataTable, "ParentTables");
            //auto ParentTables = *(TArray<UDataTable*> *) ((int64_t)CompositeDataTable + ParentTablesOffset);
            auto ParentTables = CompositeDataTable->ParentTables;

            for (int i = 0; i < ParentTables.Num(); ++i)
            {
                DataTablesToIterate.push_back(ParentTables[i]);
            }
        }

        DataTablesToIterate.push_back(DataTable);
    }


    //std::map<FName, RowStructType*>& OM = *OutMap;
    for (auto CurrentDataTable : DataTablesToIterate)
    {
        auto RM = *(RTMap<FName, RowStructType*> *) (&CurrentDataTable->RowMap);
        for (RTPair<FName, RowStructType*> CurrentPair : RM)
        {
            if (Check(CurrentPair.Key(), (RowStructType*)CurrentPair.Value()))
            {
                // LOG_INFO(LogDev, "Setting key with {} comp {} num: {} then iterating through map!", CurrentPair.Key().ToString(), CurrentPair.Key().ComparisonIndex.Value, CurrentPair.Key().Number);
                (*OutMap)[CurrentPair.Key()] = CurrentPair.Value();
                //log_debug("Set %s to %p\n", CurrentPair.Key().ToString().c_str(), (void*) CurrentPair.Value());
                //AddToMap(CurrentPair.Key(), CurrentPair.Value());

                /* for (auto PairInOutMap : *OutMap)
                {
                    // LOG_INFO(LogDev, "Current Row Key {} comp {} num: {}!", PairInOutMap.first.ToString(), PairInOutMap.first.ComparisonIndex.Value, PairInOutMap.first.Number);
                } */
            }
        }
    }
}

/*float*/int GetAmountOfLootPackagesToDrop(FFortLootTierData* LootTierData, int OriginalNumberLootDrops)
{
    if (LootTierData->LootPackageCategoryMinArray.Num() != LootTierData->LootPackageCategoryWeightArray.Num()
        || LootTierData->LootPackageCategoryMinArray.Num() != LootTierData->LootPackageCategoryMaxArray.Num()
        )
        return 0;

    // return OriginalNumberLootDrops;

    int/*float*/ MinimumLootDrops = 0;

    if (LootTierData->LootPackageCategoryMinArray.Num() > 0)
    {
        for (int i = 0; i < LootTierData->LootPackageCategoryMinArray.Num(); ++i)
        {
            // Fortnite does more here, we need to figure it out.
            MinimumLootDrops += LootTierData->LootPackageCategoryMinArray[i];
        }
    }

    if (MinimumLootDrops > OriginalNumberLootDrops)
    {
        log_warn("Requested %d loot drops but minimum drops is %d for loot package %s\n", OriginalNumberLootDrops, MinimumLootDrops, LootTierData->LootPackage.ToString().c_str());
        // Fortnite doesn't return here?
    }

    int SumLootPackageCategoryWeightArray = 0;

    if (LootTierData->LootPackageCategoryWeightArray.Num() > 0)
    {
        for (int i = 0; i < LootTierData->LootPackageCategoryWeightArray.Num(); ++i)
        {
            // Fortnite does more here, we need to figure it out.

            if (LootTierData->LootPackageCategoryWeightArray[i] > 0)
            {
                auto LootPackageCategoryMaxArrayIt = LootTierData->LootPackageCategoryMaxArray[i];

                float IDK = 0; // TODO

                if (LootPackageCategoryMaxArrayIt < 0 || IDK < LootPackageCategoryMaxArrayIt)
                {
                    SumLootPackageCategoryWeightArray += LootTierData->LootPackageCategoryWeightArray[i];
                }
            }
        }
    }

    // if (MinimumLootDrops < OriginalNumberLootDrops) // real commeneted one to one
    {
        // IDK

        while (SumLootPackageCategoryWeightArray > 0)
        {
            // HONESTLY IDEK WHAT FORTNITE DOES HERE

            /*float v29 = (float)rand() * 0.000030518509f;

            float v35 = (int)(float)((float)((float)((float)SumLootPackageCategoryWeightArray * v29)
                + (float)((float)SumLootPackageCategoryWeightArray * v29))
                + 0.5f) >> 1;*/

            // OutLootTierInfo->Hello++;
            MinimumLootDrops++;

            if (MinimumLootDrops >= OriginalNumberLootDrops)
                return MinimumLootDrops;

            SumLootPackageCategoryWeightArray--;
        }

        /* if (MinimumLootDrops < OriginalNumberLootDrops)
        {
            log_warn("Requested %d loot drops but maximum drops is %f for loot package %s\n", OriginalNumberLootDrops, MinimumLootDrops, LootTierData->LootPackage.ToString().c_str());
        } */
    }

    return MinimumLootDrops;
}

/*struct UFortLootPackage
{
    int CurrentIdx = 0;
    std::vector<FFortItemEntry> ItemEntries;
}; */

FFortLootTierData* PickLootTierData(const std::vector<UDataTable*>& LTDTables, FName LootTierGroup, int ForcedLootTier = -1, FName* OutRowName = nullptr) // Fortnite returns the row name and then finds the tier data again, but I really don't see the point of this.
{
    // This like isn't right, at all.

    int/*float*/ LootTier = ForcedLootTier;

    if (LootTier == -1)
    {
        // LootTier = ??
    }
    else
    {
        // buncha code im too lazy to reverse
    }

    // if (fabs(LootTier) <= 0.0000000099999999f)
      //  return 0;

    int Multiplier = LootTier == -1 ? 1 : LootTier; // Idk i think we need to fill out the code above for this to work properly maybe

    std::map<FName, FFortLootTierData*> TierGroupLTDs;

    CollectDataTablesRows<FFortLootTierData>(LTDTables, &TierGroupLTDs, [&](FName RowName, FFortLootTierData* TierData) -> bool {
        //log_debug("TG: %s, TTG: %s\n", TierData->TierGroup.ToString().c_str(), LootTierGroup.ToString().c_str());
        if (LootTierGroup == TierData->TierGroup)
        {
            //log_debug("LootTier: %d\n", TierData->LootTier);
            if ((LootTier == -1 ? true : LootTier == TierData->LootTier))
            {
                //log_debug("Weight Collect %f\n", TierData->Weight);
                TierData->Weight = 0.5f + GetMath()->RandomFloatInRange(0.5, 1);
                return true;
            }
        }

        return false;
        });

    // LOG_INFO(LogDev, "TierGroupLTDs.size(): {}", TierGroupLTDs.size());

    FFortLootTierData* ChosenRowLootTierData = PickWeightedElement<FName, FFortLootTierData*>(TierGroupLTDs,
        [](FFortLootTierData* LootTierData) -> float { /*log_debug("[PVendor] Weight LootTierData %f\n", LootTierData->Weight);*/  return LootTierData->Weight; }, RandomFloatForLoot, -1,
        true, Multiplier, OutRowName);

    return ChosenRowLootTierData;
}

class UFortLootLevel
{
public:
    static int GetItemLevel(const FDataTableCategoryHandle& LootLevelData, int WorldLevel);
};

int UFortLootLevel::GetItemLevel(const FDataTableCategoryHandle& LootLevelData, int WorldLevel)
{
    // OMG IM GONNA DIE

    // we should use GetRows but L

    auto DataTable = LootLevelData.DataTable;

    if (!DataTable)
        return 0;

    if (!LootLevelData.ColumnName.ComparisonIndex)
        return 0;

    if (!LootLevelData.RowContents.ComparisonIndex)
        return 0;

    std::vector<FFortLootLevelData*> OurLootLevelDatas;

    for (auto& LootLevelDataPair : (RTMap<FName, FFortLootLevelData*>&) LootLevelData.DataTable->RowMap)
    {
        if (LootLevelDataPair.Second->Category != LootLevelData.RowContents)
            continue;

        OurLootLevelDatas.push_back(LootLevelDataPair.Second);
    }

    if (OurLootLevelDatas.size() > 0)
    {
        int PickedIndex = -1;
        int PickedLootLevel = 0;

        for (int i = 0; i < OurLootLevelDatas.size(); i++)
        {
            auto CurrentLootLevelData = OurLootLevelDatas.at(i);

            if (CurrentLootLevelData->LootLevel <= WorldLevel && CurrentLootLevelData->LootLevel > PickedLootLevel)
            {
                PickedLootLevel = CurrentLootLevelData->LootLevel;
                PickedIndex = i;
            }
        }

        if (PickedIndex != -1)
        {
            auto PickedLootLevelData = OurLootLevelDatas.at(PickedIndex);

            const auto PickedMinItemLevel = PickedLootLevelData->MinItemLevel;
            const auto PickedMaxItemLevel = PickedLootLevelData->MaxItemLevel;
            auto v15 = PickedMaxItemLevel - PickedMinItemLevel;

            if (v15 + 1 <= 0)
            {
                v15 = 0;
            }
            else
            {
                auto v16 = (int)(float)((float)((float)rand() * 0.000030518509) * (float)(v15 + 1));
                if (v16 <= v15)
                    v15 = v16;
            }

            return v15 + PickedMinItemLevel;
        }
    }

    return 0;
}

int PickLevel(UFortWorldItemDefinition* def, int PreferredLevel) // well min level and maxlevel is sometimes in ufortowrlditemdeifnit9 then on older versions ufortitemdefinitoj so idk wher tyo put this
{
    int PickedLevel = 0;

    if (PreferredLevel >= def->MinLevel)
        PickedLevel = PreferredLevel;

    if (def->MaxLevel >= 0)
    {
        if (PickedLevel <= def->MaxLevel)
            return PickedLevel;
        return def->MaxLevel;
    }

    return PickedLevel;
}

void PickLootDropsFromLootPackage(const std::vector<UDataTable*>& LPTables, const FName& LootPackageName, std::vector<LootDrop>* OutEntries, int LootPackageCategory = -1, int WorldLevel = 0, bool bPrint = false, bool bCombineDrops = true)
{
    if (!OutEntries)
        return;

    std::map<FName, FFortLootPackageData*> LootPackageIDMap;

    CollectDataTablesRows<FFortLootPackageData>(LPTables, &LootPackageIDMap, [&](FName RowName, FFortLootPackageData* LootPackage) -> bool {
        if (LootPackage->LootPackageID != LootPackageName)
        {
            return false;
        }

        if (LootPackageCategory != -1 && LootPackage->LootPackageCategory != LootPackageCategory) // idk if proper
        {
            return false;
        }

        if (WorldLevel >= 0)
        {
            if (LootPackage->MaxWorldLevel >= 0 && WorldLevel > LootPackage->MaxWorldLevel)
                return 0;

            if (LootPackage->MinWorldLevel >= 0 && WorldLevel < LootPackage->MinWorldLevel)
                return 0;
        }

        return true;
        });

    if (LootPackageIDMap.size() == 0)
    {
        // log_warn("Loot Package %s has no valid weights.\n", LootPackageName.ToString().c_str());
        return;
    }

    FName PickedPackageRowName;
    FFortLootPackageData* PickedPackage = PickWeightedElement<FName, FFortLootPackageData*>(LootPackageIDMap,
        [](FFortLootPackageData* LootPackageData) -> float { return LootPackageData->Weight; }, RandomFloatForLoot,
        -1, true, 1, &PickedPackageRowName, bPrint);

    if (!PickedPackage)
        return;

    //if (bPrint)
    log_debug("PickLootDropsFromLootPackage selected package %s with loot package category %d with weight %f from LootPackageIDMap of size: %zd\n", PickedPackageRowName.ToString().c_str(), LootPackageCategory, PickedPackage->Weight, LootPackageIDMap.size());

    if (PickedPackage->LootPackageCall.Num() > 1)
    {
        if (PickedPackage->Count > 0)
        {
            int v9 = 0;

            while (v9 < PickedPackage->Count)
            {
                int LootPackageCategoryToUseForLPCall = 0; // hmm

                PickLootDropsFromLootPackage(LPTables,
                    Conv_StringToName(PickedPackage->LootPackageCall),
                    OutEntries, LootPackageCategoryToUseForLPCall, WorldLevel, bPrint
                );

                v9++;
            }
        }

        return;
    }

    auto ItemDefinition = PickedPackage->ItemDefinition.Get();

    if (!ItemDefinition)
    {
        log_warn("Loot Package %s does not contain a LootPackageCall or ItemDefinition.\n", PickedPackage->LootPackageID.ToString().c_str());
        return;
    }

    auto WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemDefinition);
    int LoadedAmmo = WeaponItemDefinition ? GetClipSize(WeaponItemDefinition) : 0; // we shouldnt set loaded ammo here techinally

    auto WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemDefinition);

    if (!WorldItemDefinition) // hahahah not proper!!
        return;

    int ItemLevel = UFortLootLevel::GetItemLevel(WorldItemDefinition->LootLevelData, WorldLevel);

    int CountMultiplier = 1;
    int FinalCount = CountMultiplier * PickedPackage->Count;

    if (FinalCount > 0)
    {
        int FinalItemLevel = 0;

        if (ItemLevel >= 0)
            FinalItemLevel = ItemLevel;

        while (FinalCount > 0)
        {
            int MaxStackSize = static_cast<int>(std::round(ItemDefinition->MaxStackSize.Value));

            int CurrentCountForEntry = MaxStackSize;

            if (FinalCount <= MaxStackSize)
                CurrentCountForEntry = FinalCount;

            if (CurrentCountForEntry <= 0)
                CurrentCountForEntry = 0;

            auto ActualItemLevel = PickLevel(WorldItemDefinition, FinalItemLevel);

            bool bHasCombined = false;

            if (bCombineDrops)
            {
                for (auto& CurrentLootDrop : *OutEntries)
                {
                    if (CurrentLootDrop->ItemDefinition == ItemDefinition)
                    {
                        int NewCount = CurrentLootDrop->Count + CurrentCountForEntry;

                        if (NewCount <= static_cast<int>(std::round(ItemDefinition->MaxStackSize.Value)))
                        {
                            bHasCombined = true;
                            CurrentLootDrop->Count = NewCount;
                        }
                    }
                }
            }

            if (!bHasCombined)
            {
                OutEntries->push_back(LootDrop(MakeItemEntry(ItemDefinition, CurrentCountForEntry, LoadedAmmo, 0x3F800000, ActualItemLevel)));
            }

            if (true)
            {
                /*

                Alright, so Fortnite literally doesn't reference the first loot package category for chests and floor loot (didnt check rest).
                Usually the first loot package category in our case is ammo, so this is quite weird.
                I have no clue how Fortnite would actually add the ammo.

                Guess what, on the chapter 2 new loot tier groups, like FactionChests, they don't even have a package which has ammo as its loot package call.

                */

                bool IsWeapon = PickedPackage->LootPackageID.ToString().contains(".Weapon.") && WeaponItemDefinition; // ONG?

                if (IsWeapon)
                {
                    auto AmmoData = WeaponItemDefinition->AmmoData.Get();

                    //if (AmmoData)
                    {
                        int AmmoCount = AmmoData->DropCount; // idk about this one

                        OutEntries->push_back(LootDrop(MakeItemEntry(AmmoData, AmmoCount)));
                    }
                }
            }

            //if (bPrint)
            {
                log_debug("Adding Item: %s\n", ItemDefinition->Name.ToString().c_str());
            }

            FinalCount -= CurrentCountForEntry;
        }
    }
}

std::vector<LootDrop> PickLootDrops(FName TierGroupName, int WorldLevel, int ForcedLootTier = -1, bool bPrint = false, int recursive = 0, bool bCombineDrops = true)
{
    std::vector<LootDrop> LootDrops;

    if (recursive > 6)
        return LootDrops;

    auto GameState = GetGameState();

    static std::vector<UDataTable*> LTDTables;
    static std::vector<UDataTable*> LPTables;

    static auto CompositeDataTableClass = UObject::FindObject<UClass>("/Script/Engine.CompositeDataTable");

    static int LastNum1 = 14915;

    auto CurrentPlaylist = GetGameState()->CurrentPlaylistInfo.BasePlaylist;

    if (LastNum1 != 0)
    {
        LastNum1 = 0;

        LTDTables.clear();
        LPTables.clear();

        bool bFoundPlaylistTable = false;

        if (CurrentPlaylist)
        {
            auto& LootTierDataSoft = CurrentPlaylist->LootTierData;
            auto& LootPackagesSoft = CurrentPlaylist->LootPackages;

            //if (LootTierDataSoft.IsValid() && LootPackagesSoft.IsValid())
            {
                /*auto LootTierDataStr = LootTierDataSoft.ObjectID.AssetPathName.ToString();
                auto LootPackagesStr = LootPackagesSoft.ObjectID.AssetPathName.ToString();
                auto LootTierDataTableIsComposite = LootTierDataStr.contains("Composite");
                auto LootPackageTableIsComposite = LootPackagesStr.contains("Composite");*/

                UDataTable* StrongLootTierData = LootTierDataSoft.Get();
                UDataTable* StrongLootPackage = LootPackagesSoft.Get();

                /*if (!Addresses::LoadAsset)
                {
                    StrongLootTierData = LootTierDataSoft.Get(LootTierDataTableIsComposite ? CompositeDataTableClass : UDataTable::StaticClass(), true);
                    StrongLootPackage = LootPackagesSoft.Get(LootPackageTableIsComposite ? CompositeDataTableClass : UDataTable::StaticClass(), true);
                }
                else
                {
                    StrongLootTierData = (UDataTable*)Assets::LoadAsset(LootTierDataSoft.ObjectID.AssetPathName);
                    StrongLootPackage = (UDataTable*)Assets::LoadAsset(LootPackagesSoft.ObjectID.AssetPathName);
                }*/

                if (StrongLootTierData && StrongLootPackage)
                {
                    LTDTables.push_back(StrongLootTierData);
                    LPTables.push_back(StrongLootPackage);

                    bFoundPlaylistTable = true;
                }
            }
        }

        if (!bFoundPlaylistTable)
        {

            LTDTables.push_back(StaticLoadObject<UDataTable>("/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client"));
            LPTables.push_back(StaticLoadObject<UDataTable>("/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client"));
        }

        // LTDTables.push_back(LoadObject<UDataTable>(L"/Game/Athena/Playlists/Playground/AthenaLootTierData_Client.AthenaLootTierData_Client"));
        // LPTables.push_back(LoadObject<UDataTable>(L"/Game/Athena/Playlists/Playground/AthenaLootPackages_Client.AthenaLootPackages_Client"));

        static auto FortGameFeatureDataClass = UObject::FindObject<UClass>("/Script/FortniteGame.FortGameFeatureData");

        if (FortGameFeatureDataClass)
        {
            for (int i = 0; i < ChunkedObjects->Num(); ++i)
            {
                auto Object = ChunkedObjects->GetObjectByIndex(i);

                if (!Object)
                    continue;

                if (Object->IsA(FortGameFeatureDataClass))
                {
                    auto GameFeatureData = Object;
                    static auto DefaultLootTableDataOffset = GetOffset(GameFeatureData, "DefaultLootTableData");

                    if (DefaultLootTableDataOffset != -1)
                    {
                        FFortGameFeatureLootTableData* DefaultLootTableData = (FFortGameFeatureLootTableData*)((int64_t)GameFeatureData + DefaultLootTableDataOffset);

                        /*auto LootTierDataTableStr = DefaultLootTableData->LootTierData.ObjectID.AssetPathName.ToString();

                        auto LootTierDataTableIsComposite = LootTierDataTableStr.contains("Composite");
                        auto LootPackageTableStr = DefaultLootTableData->LootPackageData.ObjectID.AssetPathName.ToString();
                        auto LootPackageTableIsComposite = LootPackageTableStr.contains("Composite");*/

                        auto LootTierDataPtr = DefaultLootTableData->LootTierData.Get();
                        auto LootPackagePtr = DefaultLootTableData->LootPackageData.Get();

                        if (LootPackagePtr)
                        {
                            LPTables.push_back(LootPackagePtr);
                        }

                        if (CurrentPlaylist)
                        {
                            static auto PlaylistOverrideLootTableDataOffset = GetOffset(GameFeatureData, "PlaylistOverrideLootTableData");
                            RTMap<FGameplayTag, FFortGameFeatureLootTableData>& PlaylistOverrideLootTableData = *(RTMap<FGameplayTag, FFortGameFeatureLootTableData> *) ((int64_t)GameFeatureData + PlaylistOverrideLootTableDataOffset);

                            static auto GameplayTagContainerOffset = GetOffset(CurrentPlaylist, "GameplayTagContainer");
                            auto GameplayTagContainer = (FGameplayTagContainer*)((int64_t)CurrentPlaylist + GameplayTagContainerOffset);

                            for (int i = 0; i < GameplayTagContainer->GameplayTags.Num(); ++i)
                            {
                                auto& Tag = GameplayTagContainer->GameplayTags[i];

                                for (auto& Value : PlaylistOverrideLootTableData)
                                {
                                    auto CurrentOverrideTag = Value.First;

                                    if (Tag.TagName == CurrentOverrideTag.TagName)
                                    {

                                        auto ptr = Value.Second.LootPackageData.Get();

                                        if (ptr)
                                        {
                                            /* if (bOverrideIsComposite)
                                            {
                                                static auto ParentTablesOffset = ptr->GetOffset("ParentTables");

                                                auto ParentTables = ptr->GetPtr<TArray<UDataTable*>>(ParentTablesOffset);

                                                for (int z = 0; z < ParentTables->size(); z++)
                                                {
                                                    auto ParentTable = ParentTables->At(z);

                                                    if (ParentTable)
                                                    {
                                                        LPTables.push_back(ParentTable);
                                                    }
                                                }
                                            } */

                                            LPTables.push_back(ptr);
                                        }
                                    }
                                }
                            }
                        }

                        if (LootTierDataPtr)
                        {
                            LTDTables.push_back(LootTierDataPtr);
                        }

                        if (CurrentPlaylist)
                        {
                            static auto PlaylistOverrideLootTableDataOffset = GetOffset(GameFeatureData, "PlaylistOverrideLootTableData");
                            RTMap<FGameplayTag, FFortGameFeatureLootTableData>& PlaylistOverrideLootTableData = *(RTMap<FGameplayTag, FFortGameFeatureLootTableData> *) ((int64_t)GameFeatureData + PlaylistOverrideLootTableDataOffset);

                            static auto GameplayTagContainerOffset = GetOffset(CurrentPlaylist, "GameplayTagContainer");
                            auto GameplayTagContainer = (FGameplayTagContainer*)((int64_t)CurrentPlaylist + GameplayTagContainerOffset);

                            for (int i = 0; i < GameplayTagContainer->GameplayTags.Num(); ++i)
                            {
                                auto& Tag = GameplayTagContainer->GameplayTags[i];

                                for (auto& Value : PlaylistOverrideLootTableData)
                                {
                                    auto CurrentOverrideTag = Value.First;

                                    if (Tag.TagName == CurrentOverrideTag.TagName)
                                    {
                                        auto ptr = Value.Second.LootTierData.Get();

                                        if (ptr)
                                        {
                                            /* if (bOverrideIsComposite)
                                            {
                                                static auto ParentTablesOffset = ptr->GetOffset("ParentTables");

                                                auto ParentTables = ptr->GetPtr<TArray<UDataTable*>>(ParentTablesOffset);

                                                for (int z = 0; z < ParentTables->size(); z++)
                                                {
                                                    auto ParentTable = ParentTables->At(z);

                                                    if (ParentTable)
                                                    {
                                                        LTDTables.push_back(ParentTable);
                                                    }
                                                }
                                            } */

                                            LTDTables.push_back(ptr);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        for (int i = 0; i < LTDTables.size(); ++i)
        {
            auto& Table = LTDTables.at(i);

            if (!ChunkedObjects->IsValid(Table))
            {
                continue;
            }

            GetItemByIndex(Table->Index)->SetRootSet();
            log_debug("[%d] LTD %s\n", i, Table->GetFullName().c_str());
        }

        for (int i = 0; i < LPTables.size(); ++i)
        {
            auto& Table = LPTables.at(i);

            if (!ChunkedObjects->IsValid(Table))
            {
                continue;
            }

            GetItemByIndex(Table->Index)->SetRootSet();
            log_debug("[%d] LP %s\n", i, Table->GetFullName().c_str());
        }
    }

    /*if (!Addresses::LoadAsset)
    {
        if (Fortnite_Version <= 6 || std::floor(Fortnite_Version) == 9) // ahhh
        {
            LTDTables.clear();
            LPTables.clear();

            bool bFoundPlaylistTable = false;

            if (CurrentPlaylist)
            {
                static auto LootTierDataOffset = CurrentPlaylist->GetOffset("LootTierData");
                auto& LootTierDataSoft = CurrentPlaylist->Get<TSoftObjectPtr<UDataTable>>(LootTierDataOffset);

                static auto LootPackagesOffset = CurrentPlaylist->GetOffset("LootPackages");
                auto& LootPackagesSoft = CurrentPlaylist->Get<TSoftObjectPtr<UDataTable>>(LootPackagesOffset);

                if (LootTierDataSoft.IsValid() && LootPackagesSoft.IsValid())
                {
                    auto LootTierDataStr = LootTierDataSoft.SoftObjectPtr.ObjectID.AssetPathName.ToString();
                    auto LootPackagesStr = LootPackagesSoft.SoftObjectPtr.ObjectID.AssetPathName.ToString();
                    auto LootTierDataTableIsComposite = LootTierDataStr.contains("Composite");
                    auto LootPackageTableIsComposite = LootPackagesStr.contains("Composite");

                    UDataTable* StrongLootTierData = nullptr;
                    UDataTable* StrongLootPackage = nullptr;

                    StrongLootTierData = LootTierDataSoft.Get(LootTierDataTableIsComposite ? CompositeDataTableClass : UDataTable::StaticClass(), true);
                    StrongLootPackage = LootPackagesSoft.Get(LootPackageTableIsComposite ? CompositeDataTableClass : UDataTable::StaticClass(), true);

                    if (StrongLootTierData && StrongLootPackage)
                    {
                        LTDTables.push_back(StrongLootTierData);
                        LPTables.push_back(StrongLootPackage);

                        bFoundPlaylistTable = true;
                    }
                }
            }

            if (!bFoundPlaylistTable)
            {
                LTDTables.push_back(LoadObject<UDataTable>(L"/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client"));
                LPTables.push_back(LoadObject<UDataTable>(L"/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client"));
            }
        }

    }*/

    if (LTDTables.size() <= 0 || LPTables.size() <= 0)
    {
        log_warn("Empty tables! (%zd %zd)\n", LTDTables.size(), LPTables.size());
        return LootDrops;
    }

    FName LootTierRowName;
    auto ChosenRowLootTierData = PickLootTierData(LTDTables, TierGroupName, ForcedLootTier, &LootTierRowName);

    if (!ChosenRowLootTierData)
    {
        log_warn("Failed to find LootTierData row for %s with loot tier %d\n", TierGroupName.ToString().c_str(), ForcedLootTier);
        return LootDrops;
    }
    else if (bPrint)
    {
        log_info("Picked loot tier data row %s\n", LootTierRowName.ToString().c_str());
    }

    // auto ChosenLootPackageName = ChosenRowLootTierData->GetLootPackage().ToString();

    // if (ChosenLootPackageName.contains(".Empty")) { return PickLootDropsNew(TierGroupName, bPrint, ++recursive); }

    float NumLootPackageDrops = ChosenRowLootTierData->NumLootPackageDrops;

    /*float*/int NumberLootDrops = 0;

    if (NumLootPackageDrops > 0)
    {
        if (NumLootPackageDrops < 1)
        {
            NumberLootDrops = 1;
        }
        else
        {
            NumberLootDrops = (int)(float)((float)(NumLootPackageDrops + NumLootPackageDrops) - 0.5f) >> 1;
            float v20 = NumLootPackageDrops - NumberLootDrops;
            if (v20 > 0.0000099999997f)
            {
                NumberLootDrops += v20 >= (rand() * 0.000030518509f);
            }
        }
    }

    int AmountOfLootPackageDrops = GetAmountOfLootPackagesToDrop(ChosenRowLootTierData, NumberLootDrops);

    LootDrops.reserve(/*(size_t)*/ AmountOfLootPackageDrops);

    if (AmountOfLootPackageDrops > 0)
    {
        for (int i = 0; i < AmountOfLootPackageDrops; ++i)
        {
            if (i >= ChosenRowLootTierData->LootPackageCategoryMinArray.Num())
                break;

            for (int j = 0; j < ChosenRowLootTierData->LootPackageCategoryMinArray[i]; ++j)
            {
                if (ChosenRowLootTierData->LootPackageCategoryMinArray[i] < 1)
                    break;

                int LootPackageCategory = i;

                PickLootDropsFromLootPackage(LPTables, ChosenRowLootTierData->LootPackage, &LootDrops, LootPackageCategory, WorldLevel, bPrint);
            }
        }
    }

    return LootDrops;
}

struct VFCollectorUnitInfo
{
    static std::string GetStructName()
    {
        static std::string StructName = UObject::FindObject<UStruct>("/Script/FortniteGame.CollectorUnitInfo") ? "/Script/FortniteGame.CollectorUnitInfo" : "/Script/FortniteGame.ColletorUnitInfo"; // nice one fortnite
        return StructName;
    }

    static UStruct* GetStruct()
    {
        static auto Struct = UObject::FindObject<UStruct>(GetStructName());
        return Struct;
    }

    static int GetPropertiesSize()
    {
        static auto Struct = UObject::FindObject<UStruct>("/Script/FortniteGame.CollectorUnitInfo");
        return Struct->Size;
    }

    FScalableFloat* GetInputCount()
    {
        static auto InputCountOffset = FindOffsetStruct(GetStructName(), "InputCount");
        return (FScalableFloat*)(__int64(this) + InputCountOffset);
    }

    TArray<FFortItemEntry>* GetOutputItemEntry()
    {
        static auto OutputItemEntryOffset = FindOffsetStruct(GetStructName(), "OutputItemEntry");
        return (TArray<FFortItemEntry>*)(__int64(this) + OutputItemEntryOffset);
    }

    UFortWorldItemDefinition*& GetInputItem()
    {
        static auto InputItemOffset = FindOffsetStruct(GetStructName(), "InputItem");
        return *(UFortWorldItemDefinition**)(__int64(this) + InputItemOffset);
    }

    UFortWorldItemDefinition*& GetOutputItem()
    {
        static auto OutputItemOffset = FindOffsetStruct(GetStructName(), "OutputItem");
        return *(UFortWorldItemDefinition**)(__int64(this) + OutputItemOffset);
    }
};

static inline UCurveTable* GetGameData()
{
    auto GameState = Cast<AFortGameStateAthena>(GetGameState());

    UCurveTable* FortGameData = nullptr;

    auto CurrentPlaylist = GameState->CurrentPlaylistInfo.BasePlaylist;

    if (CurrentPlaylist)
    {
        //static auto GameDataOffset = CurrentPlaylist->GetOffset("GameData");
        //FortGameData = CurrentPlaylist ? CurrentPlaylist->GetPtr<TSoftObjectPtr<UCurveTable>>(GameDataOffset)->Get() : nullptr;
        FortGameData = CurrentPlaylist->GameData.Get();
    }

    if (!FortGameData)
        FortGameData = /*UObject::FindObject*/StaticLoadObject<UCurveTable>("/Game/Athena/Balance/DataTables/AthenaGameData.AthenaGameData"); // uhm so theres one without athena and on newer versions that has it so idk // after i wrote this cokmment idk what i meant

    return FortGameData;
}

static inline void FillItemCollector(ABuildingItemCollectorActor* ItemCollector, FName& LootTierGroup, bool bUseInstanceLootValueOverrides, int LootTier, int recursive = 0)
{
    if (recursive >= 10)
        return;

    auto GameModeAthena = (AFortGameModeAthena*)GetGameMode();
    auto GameState = Cast<AFortGameStateAthena>(GetGameState());

    //static auto ItemCollectionsOffset = ItemCollector->GetOffset("ItemCollections");
    //auto& ItemCollections = ItemCollector->Get<TArray<FCollectorUnitInfo>>(ItemCollectionsOffset);
    auto ItemCollections = ItemCollector->ItemCollections;

    static UCurveTable* FortGameData = GetGameData();

    auto WoodName = Conv_StringToName(L"Default.VendingMachine.Cost.Wood");
    auto StoneName = Conv_StringToName(L"Default.VendingMachine.Cost.Stone");
    auto MetalName = Conv_StringToName(L"Default.VendingMachine.Cost.Metal");

    static auto StoneItemData = StaticFindObject<UFortResourceItemDefinition>("/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
    static auto MetalItemData = StaticFindObject<UFortResourceItemDefinition>("/Game/Items/ResourcePickups/MetalItemData.MetalItemData");

    // TODO: Pull prices from datatables.

    bool bLowerPrices = true;

    static int CommonPrice = bLowerPrices ? 75 : 100;
    static int UncommonPrice = bLowerPrices ? 150 : 200;
    static int RarePrice = bLowerPrices ? 225 : 300;
    static int EpicPrice = bLowerPrices ? 300 : 400;
    static int LegendaryPrice = bLowerPrices ? 375 : 500;

    if (true)
    {
        CommonPrice = 0;
        UncommonPrice = 0;
        RarePrice = 0;
        EpicPrice = 0;
        LegendaryPrice = 0;
    }

    int itemCollectorRecursive = 0;

    for (int ItemCollectorIt = 0; ItemCollectorIt < ItemCollections.Num() /*3*/; ItemCollectorIt++)
    {
        if (itemCollectorRecursive > 3)
        {
            itemCollectorRecursive = 0;
            continue;
        }

        auto ItemCollection = ItemCollections.Data + ItemCollectorIt;

        if (ItemCollection->OutputItemEntry.Num() > 0)
        {
            ItemCollection->OutputItemEntry.FreeArray();
            ItemCollection->OutputItem = nullptr;
        }

        constexpr bool bPrint = true;

        log_debug("Tier: %d\n", LootTier);
        /*std::vector<LootDrop> LootDrops = PickLootDrops(LootTierGroup, GameState->WorldLevel, LootTier, bPrint);

        if (LootDrops.size() == 0)
        {
            // LOG_WARN(LogGame, "Failed to find LootDrops for vending machine loot tier: {}", LootTier);
            ItemCollectorIt--; // retry (?)
            itemCollectorRecursive++;
            continue;
        }*/


        ItemRow* row = GetRandomItem(LootTierGroup.ToString(), EFortItemType::WeaponRanged, LootTier, true);
        UFortWorldItemDefinition* WorldItemDefinition = (UFortWorldItemDefinition*)row->Def;

        for (int LootDropIt = 0; LootDropIt < /*LootDrops.size()*/1; LootDropIt++)
        {
            //UFortWorldItemDefinition* WorldItemDefinition = Cast<UFortWorldItemDefinition>(LootDrops[LootDropIt]->ItemDefinition);

            if (!WorldItemDefinition)
                continue;

            if (!IsPrimaryQuickbar(WorldItemDefinition)) // i dont think we need this check
                continue;

            bool bItemAlreadyInCollector = false;

            for (int ItemCollectorIt2 = 0; ItemCollectorIt2 < ItemCollections.Num(); ItemCollectorIt2++)
            {
                auto ItemCollection2 = ItemCollections.Data + ItemCollectorIt2;

                if (ItemCollection2->OutputItem == WorldItemDefinition)
                {
                    //bItemAlreadyInCollector = true;
                    break;
                }
            }

            if (bItemAlreadyInCollector)
                break;

            ItemCollection->OutputItem = WorldItemDefinition;

            break;
        }

        if (!ItemCollection->OutputItem)
        {
            ItemCollectorIt--; // retry
            itemCollectorRecursive++;
            continue;
        }

        for (int LootDropIt = 0; LootDropIt < /*LootDrops.size()*/1; LootDropIt++)
        {
            /*auto ItemEntry = LootDrops[LootDropIt].ItemEntry; // FFortItemEntry::*/
            auto ItemEntry = MakeItemEntry(row->Def, row->DropCount, row->LoadedAmmo, 0x3f800000, 0);

            if (!ItemEntry)
                continue;

            //ItemCollection->OutputItemEntry.AddPtr(ItemEntry, *(int*)((int64_t)UObject::FindObject<UStruct>("/Script/FortniteGame.FortItemEntry") + 0x58));
            ItemCollection->OutputItemEntry.Add(*ItemEntry);
        }

        // The reason I set the curve to 0 is because it will force it to return value, probably not how we are supposed to do it but whatever.

        bool bShouldBeNullTable = true; // Fortnite_Version < 5

        ItemCollection->InputCount.Curve.CurveTable = bShouldBeNullTable ? nullptr : FortGameData; // scuffed idc
        ItemCollection->InputCount.Curve.RowName = bShouldBeNullTable ? FName(0) : WoodName; // Scuffed idc 
        ItemCollection->InputCount.Value = (float) (LootTier == 0 ? CommonPrice
            : LootTier == 1 ? UncommonPrice
            : LootTier == 2 ? RarePrice
            : LootTier == 3 ? EpicPrice
            : LootTier == 4 ? LegendaryPrice
            : -1);
        
        log_info("Added %s\n", row->Def->GetName().c_str());
    }

    //static auto bUseInstanceLootValueOverridesOffset = ItemCollector->GetOffset("bUseInstanceLootValueOverrides", false);

    ItemCollector->bUseInstanceLootValueOverrides = bUseInstanceLootValueOverrides;
    //if (bUseInstanceLootValueOverridesOffset != -1)
    //	ItemCollector->Get<bool>(bUseInstanceLootValueOverridesOffset) = bUseInstanceLootValueOverrides;

    // LOG_INFO(LogDev, "LootTier: {}", LootTier);

    //static auto StartingGoalLevelOffset = ItemCollector->GetOffset("StartingGoalLevel");

    ItemCollector->StartingGoalLevel = LootTier;
    //if (StartingGoalLevelOffset != -1)
    //	ItemCollector->Get<int32>(StartingGoalLevelOffset) = LootTier;

    static auto VendingMachineClass = StaticLoadObject<UClass>("/Game/Athena/Items/Gameplay/VendingMachine/B_Athena_VendingMachine.B_Athena_VendingMachine_C");

    if (ItemCollector->IsA(VendingMachineClass))
    {
        static int OverrideVendingMachineRarityOffset = -1;
        //auto Property = GetProperty(ItemCollector, "OverrideVendingMachineRarity", true);
        OverrideVendingMachineRarityOffset = GetOffset(ItemCollector, "OverrideVendingMachineRarity");

        /*if (!Property)
            goto ovmro;

        OverrideVendingMachineRarityOffset = *(int*)((int64_t)Property + 0x4c);*/

    //ovmro:
        if (OverrideVendingMachineRarityOffset != -1) {
            *(uint8_t*)((int64_t)ItemCollector + OverrideVendingMachineRarityOffset) = LootTier;
        }

        //static auto OverrideGoalOffset = ItemCollector->GetOffset("OverrideGoal", false);

        //if (OverrideGoalOffset != -1)
        {
            ItemCollector->OverrideGoal = LootTier == 0 ? CommonPrice
                : LootTier == 1 ? UncommonPrice
                : LootTier == 2 ? RarePrice
                : LootTier == 3 ? EpicPrice
                : LootTier == 4 ? LegendaryPrice
                : -1;
        }
    }
}

float EvaluateCurveTableRow(UCurveTable* CurveTable, FName RowName, float InXY,
    const FString& ContextString = FString(), EEvaluateCurveTableResult* OutResult = nullptr)
{
    /*static auto fn = StaticLoadObject<UFunction>("/Script/Engine.DataTableFunctionLibrary.EvaluateCurveTableRow");

    float wtf{};
    EEvaluateCurveTableResult wtf1{};

    struct { UCurveTable* CurveTable; FName RowName; float InXY; EEvaluateCurveTableResult OutResult; float OutXY; FString ContextString; }
    UDataTableFunctionLibrary_EvaluateCurveTableRow_Params{ CurveTable, RowName, InXY, wtf1, wtf, ContextString };

    static auto DefaultClass = UDataTableFunctionLibrary::StaticClass();
    DefaultClass->ProcessEvent(fn, &UDataTableFunctionLibrary_EvaluateCurveTableRow_Params);

    if (OutResult)
        *OutResult = UDataTableFunctionLibrary_EvaluateCurveTableRow_Params.OutResult;

    return UDataTableFunctionLibrary_EvaluateCurveTableRow_Params.OutXY;*/

    float wtf{};
    //EEvaluateCurveTableResult wtf1{};
    struct FCurveTableRowHandle RH {
        CurveTable, RowName
    };
    return GetFortKismet()->EvaluateCurveTableRow(RH, InXY, &wtf, ContextString);
}

static inline void FillVendingMachines()
{
    auto VendingMachineClass = StaticLoadObject<UClass>("/Game/Athena/Items/Gameplay/VendingMachine/B_Athena_VendingMachine.B_Athena_VendingMachine_C");
    TArray<AActor*> AllVendingMachines;
    GetStatics()->GetAllActorsOfClass(GetWorld(), VendingMachineClass, &AllVendingMachines);

    auto OverrideLootTierGroup = Conv_StringToName(L"Loot_AthenaVending"); // ItemCollector->GetLootTierGroupOverride();

    std::map<int, float> ThingAndWeights; // Bro IDK WHat to name it!

    auto RarityWeightsName = Conv_StringToName(L"Default.VendingMachine.RarityWeights");

    auto FortGameData = GetGameData();
    //auto FortGameData = StaticFindObject<UCurveTable>("/Game/Athena/Balance/DataTables/AthenaGameData.AthenaGameData");

    float WeightSum = 0;

    for (int i = 0; i < 6; i++)
    {
        //auto Weight = EvaluateCurveTableRow(FortGameData, RarityWeightsName, i);
        float Weight = (i == 0) ? 0.0f : (100.0f / (7 - i));
        ThingAndWeights[i] = Weight;
        WeightSum += Weight;
        //log_debug("Weight: %f\n", Weight);
    }

    //for (int i = 0; i < ThingAndWeights.size(); i++)
    {
        // LOG_INFO(LogDev, "[{}] bruh: {}", i, ThingAndWeights.at(i));
    }

    std::map<int, int> PickedRarities;

    for (int i = 0; i < AllVendingMachines.Num(); i++)
    {
        auto VendingMachine = (ABuildingItemCollectorActor*)AllVendingMachines[i];

        if (!VendingMachine)
            continue;

        auto randomFloatGenerator = [&](float Max) -> float { return GetMath()->RandomFloatInRange(0, Max); };

        int Out;
        PickWeightedElement<int, float>(ThingAndWeights, [&](float Weight) -> float { return Weight; }, randomFloatGenerator, WeightSum, false, 1, &Out, false, true);

        PickedRarities[Out]++;

        if (Out == 0)
        {
            VendingMachine->K2_DestroyActor();
            continue;
        }

        /*

        LOOT LEVELS:

        0 - Common
        1 - Uncommon
        2 - Rare
        3 - Epic
        4 - Legendary

        */

        #ifndef TESTING_LOOTPOOL
        #ifndef ONE_SHOT
        FillItemCollector(VendingMachine, OverrideLootTierGroup, true, Out - 1);
        #endif
        #endif
    }

    auto AllVendingMachinesNum = AllVendingMachines.Num();

    AllVendingMachines.FreeArray();

    bool bPrintDebug = true;

    if (bPrintDebug)
    {
        log_debug("Destroyed %d/%d vending machines.\n", PickedRarities[0], AllVendingMachinesNum);
        log_debug("Filled %d/%d vending machines with common items.\n", PickedRarities[1], AllVendingMachinesNum);
        log_debug("Filled %d/%d vending machines with uncommon items.\n", PickedRarities[2], AllVendingMachinesNum);
        log_debug("Filled %d/%d vending machines with rare items.\n", PickedRarities[3], AllVendingMachinesNum);
        log_debug("Filled %d/%d vending machines with epic items.\n", PickedRarities[4], AllVendingMachinesNum);
        log_debug("Filled %d/%d vending machines with legendary items.\n", PickedRarities[5], AllVendingMachinesNum);
    }
    else
    {
        log_info("Filled %d vending machines!\n", AllVendingMachinesNum);
    }
}
#endif