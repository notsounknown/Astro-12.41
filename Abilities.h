#pragma once
#include "Engine.h"
#include "Inventory.h"

void GiveAbility(AFortPlayerControllerAthena* PC, UClass* Ability)
{
    if (!PC || !PC->PlayerState || !PC->MyFortPawn) { return; }
    auto AbilitySystemComponent = PC->MyFortPawn->AbilitySystemComponent;

    if (!AbilitySystemComponent)
        return;

    if (!Ability)
        return;

    FGameplayAbilitySpec Spec{};
    FGameplayAbilitySpecCtor(&Spec, (UGameplayAbility*)Ability->DefaultObject, 1, -1, nullptr);
    GiveAbilityOG(AbilitySystemComponent, &Spec.Handle, Spec);
}

FGameplayAbilitySpec* FindAbilitySpecFromHandle(UAbilitySystemComponent* Component, FGameplayAbilitySpecHandle& Handle)
{
    for (int32 /*size_t*/ i = 0; i < Component->ActivatableAbilities.Items.Num(); i++)
    {
        if (Component->ActivatableAbilities.Items[i].Handle.Handle == Handle.Handle) { return &Component->ActivatableAbilities.Items[i]; }
    }

    return nullptr;
}

std::map<AFortPlayerController*, int> Shots{};
std::map<AFortPlayerController*, UFortWeaponItemDefinition*> Weapons{};
int LastTime = 0;

int64_t(*ApplyCostOG)(UGameplayAbility* arg1, int32_t arg2, void* arg3, void /*int128_t*/* arg4);
int64_t ApplyCost(UFortGameplayAbility* arg1, int32_t arg2, void* arg3, void /*int128_t*/* arg4) {
    if (arg1->GetName().starts_with("GA_Athena_AppleSun_Passive_C_")) {
        auto Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/AppleSun/WID_Athena_AppleSun.WID_Athena_AppleSun");
        auto ASC = arg1->GetActivatingAbilityComponent();
        AFortPlayerStateAthena* PS = (AFortPlayerStateAthena*)ASC->GetOwner();
        auto Pawn = PS->GetCurrentPawn();
        AFortPlayerController* PC = nullptr;
        PC = (AFortPlayerController*)Pawn->GetOwner();

        if (!PC->bInfiniteAmmo) {
            Remove(PC, Def);
        }
    }
    else if (arg1->GetName().starts_with("GA_Ranged_") || arg1->GetName().starts_with("GAB_Melee_ImpactCombo_Athena_")) {
        auto ASC = arg1->GetActivatingAbilityComponent();
        AFortPlayerStateAthena* PS = (AFortPlayerStateAthena*)ASC->GetOwner();
        auto Pawn = PS->GetCurrentPawn();
        AFortPlayerController* PC = nullptr;
        PC = (AFortPlayerController*)Pawn->GetOwner();
        if (PC->IsA(ABP_PhoebePlayerController_C::StaticClass())) {
            return ApplyCostOG(arg1, arg2, arg3, arg4);
        }
        auto t = std::floor(GetStatics()->GetTimeSeconds(GetWorld()));
        if (LastTime != (int)t) {
            LastTime = (int)t;
            Shots[PC] = 0;
        }
        if (Weapons[PC] != Pawn->CurrentWeapon->WeaponData) {
            Shots[PC] = 0;
            Weapons[PC] = Pawn->CurrentWeapon->WeaponData;
        }
        
        auto WSH = Weapons[PC]->WeaponStatHandle;
        auto v = ((RTMap<FName, FFortBaseWeaponStats *> *) &WSH.DataTable->RowMap)->GetByKeyNoRef(WSH.RowName);

        Shots[PC]++;
        log_debug("Shots: %d, T: %d\n", Shots[PC], LastTime);
        //int SPS = 15;
        string WN = Pawn->CurrentWeapon->WeaponData->GetName();
        int SPS = WN.starts_with("WID_Harvest_") ? 69 : -1;
        if (SPS == -1) {
            auto s = (FFortRangedWeaponStats*)v;
            auto frToUse = s->BurstFiringRate > s->FiringRate ? s->BurstFiringRate : s->FiringRate;
            SPS = (int) ceil(frToUse) + 5;
        }
        /*if (WN.starts_with("WID_Shotgun_SemiAuto_") || WN.starts_with("WID_Shotgun_HighSemiAuto_")) {
            SPS = 5; // 2
        }
        else if (WN.starts_with("WID_Shotgun_Standard_")) {
            SPS = 4;
        }
        else if (WN.starts_with("WID_Sniper_NoScope_")) {
            SPS = 4;
        }
        else if (WN.starts_with("WID_Sniper_Standard_") || WN.starts_with("WID_Assault_Auto_") || WN.starts_with("WID_Assault_Suppressed_") || WN.starts_with("WID_Boss_Adventure_AR")) {
            SPS = 10; // 6
        }
        else if (WN.starts_with("WID_Assault_SemiAuto_")) {
            SPS = 14; // 3
        }
        else if (WN.starts_with("WID_Sniper_") || WN.starts_with("WID_Launcher_Rocket_")) {
            SPS = 3; // 1
        }
        else if (WN.starts_with("WID_Pistol_AutoHeavySuppressed_")) {
            SPS = 9; // 8
        }
        else if (WN.starts_with("WID_Assault_LMG_")) {
            SPS = 13; // 12
        }
        else if (WN.starts_with("WID_Assault_Heavy_")) {
            SPS = 8; // 5
        }
        else if (WN.starts_with("WID_Harvest_")) {
            SPS = 12; // 2
        }
        else if (WN.starts_with("WID_Hook_Gun_") || WN == "WID_Boss_Adventure_GH") {
            SPS = 3; // 1
        }
        else if (WN.starts_with("WID_Assault_LMG_Athena_")) {
            SPS = 27;
        }
        else if (WN.starts_with("WID_Pistol_AutoHeavyPDW_Athena")) {
            SPS = 17;
        }*/
        if (Shots[PC] > SPS && GetGameState()->GamePhase > EAthenaGamePhase::Aircraft) {
            //GameSession_KickPlayer(GetGameMode()->GameSession, PC);
            log_info("AC: Killed %s for rapid fire\n", PC->MyFortPawn->GetName().c_str());
            PC->MyFortPawn->BalloonActiveCount = 255;
            PC->MyFortPawn->ForceKill({ Conv_StringToName(L"Meowscles.Cheating") }, nullptr, nullptr);
            //PC->ClientReturnToMainMenu(L"You have been kicked for cheating! Please remember to play fair on Meowscles.");
        }
    }
    log_debug("ApplyCost %s\n", ((UObject*)arg1)->GetName().c_str());
    return ApplyCostOG(arg1, arg2, arg3, arg4);
}

void (*InternalServerTryActivateAbilityOG)(UFortAbilitySystemComponentAthena* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, FPredictionKey& PredictionKey, FGameplayEventData* TriggerEventData);
void InternalServerTryActivateAbilityHook(UFortAbilitySystemComponentAthena* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, FPredictionKey& PredictionKey, FGameplayEventData* TriggerEventData)
{
    FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(AbilitySystemComponent, Handle);
    if (!Spec)
        return AbilitySystemComponent->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
    log_info("Ability activated: %s\n", Spec->Ability->GetName().c_str());

    // Consume any pending target info, to clear out cancels from old executions
    //ConsumeAllReplicatedData(Handle, PredictionKey);

    UGameplayAbility* AbilityToActivate = Spec->Ability;

    UGameplayAbility* InstancedAbility = nullptr;
    Spec->InputPressed = true;

    /*if (Spec->Ability->GetName() == "Default__GA_Lotus_Athena_ApplyHeal_C") {
        AFortPlayerStateAthena *PS = (AFortPlayerStateAthena *) AbilitySystemComponent->GetOwner();
        auto Pawn = PS->GetCurrentPawn();
        auto NewHealth = (Pawn->GetHealth() + 7.5f) >= 100 ? 100 : (Pawn->GetHealth() + 7.5f);
        Pawn->SetHealth(NewHealth);
    } else {}*/
    AFortPlayerStateAthena* PS = (AFortPlayerStateAthena*)AbilitySystemComponent->GetOwner();
    auto Pawn = PS->GetCurrentPawn();
    AFortPlayerController* PC = nullptr;
    if (!Pawn) goto activate;
    PC = (AFortPlayerController *) Pawn->GetOwner();
    if (Spec->Ability->GetName() == "Default__GAB_InterrogatePlayer_Reveal_C") {
		log_debug("InterrogatePlayer_Reveal, %s\n", TriggerEventData->Target->GetName().c_str());
        if ((Pawn->GetHealth() + 20.f) >= 100) {
            auto NewShield = (Pawn->GetShield() + 20.f) >= 100 ? 100 : (Pawn->GetShield() + 20.f);
            Pawn->SetShield(NewShield);
        }
        else {
            auto NewHealth = Pawn->GetHealth() + 20.f;
            Pawn->SetHealth(NewHealth);
        }
    }
    else if (Spec->Ability->GetName() == "Default__GAT_Athena_c4_Detonate_C") {
        auto Def = StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/C4/Athena_C4.Athena_C4");
        float MaxStackSize = GetMaxStackSize(Def);
        FFortItemEntry* FoundEntry = nullptr;

        for (int32 /*size_t*/ i = 0; i < PC->WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
        {
            FFortItemEntry& Entry = PC->WorldInventory->Inventory.ReplicatedEntries[i];

            if (Entry.ItemDefinition == Def && (Entry.Count < MaxStackSize))
            {
                FoundEntry = &PC->WorldInventory->Inventory.ReplicatedEntries[i];
            }
        }

        if (FoundEntry && FoundEntry->Count == 0) {
            Remove(PC, Def);
        }
    }
    activate:
    // Attempt to activate the ability (server side) and tell the client if it succeeded or failed.
    if (!InternalTryActivateAbility(AbilitySystemComponent, Handle, PredictionKey, &InstancedAbility, nullptr, TriggerEventData))
    {
        log_debug("Ability failed! %s\n", Spec->Ability->GetName().c_str());
        AbilitySystemComponent->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
        Spec->InputPressed = false;
        AbilitySystemComponent->ActivatableAbilities.MarkItemDirty(*Spec);
    }
    //MarkAbilitySpecDirty(*Spec);
}

void GiveAbilitySet(AFortPlayerControllerAthena* PC, UFortAbilitySet* Set)
{
    if (Set)
    {
        for (int32 /*size_t*/ i = 0; i < Set->GameplayAbilities.Num(); i++)
        {
            UClass* Ability = Set->GameplayAbilities[i].Get();
            if (Ability) { GiveAbility(PC, Ability); }
        }
    }
}

void RemoveAbility(AFortPlayerController* PC, UClass* AbilityClass)
{
    for (int32 /*size_t*/ i = 0; i < PC->MyFortPawn->AbilitySystemComponent->ActivatableAbilities.Items.Num(); i++)
    {
        if (PC->MyFortPawn->AbilitySystemComponent->ActivatableAbilities.Items[i].Ability->Class == AbilityClass)
        {
            auto& Item = PC->MyFortPawn->AbilitySystemComponent->ActivatableAbilities.Items[i];
            PC->MyFortPawn->AbilitySystemComponent->ServerEndAbility(Item.Handle, Item.ActivationInfo, Item.ActivationInfo.PredictionKeyWhenActivated);
            PC->MyFortPawn->AbilitySystemComponent->ServerCancelAbility(Item.Handle, Item.ActivationInfo);
            PC->MyFortPawn->AbilitySystemComponent->ClientCancelAbility(Item.Handle, Item.ActivationInfo);
            break;
        }
    }
}