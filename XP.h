#pragma once
#include "Engine.h"
#include "Backend.h"

FFortUpdatedObjectiveStat& GetStat(AFortPlayerControllerAthena* PC, UFortQuestItemDefinition* QuestDef, bool& NeedsToAdd)
{
    FFortUpdatedObjectiveStat Stat{};
    for (int32 /*size_t*/ i = 0; i < PC->UpdatedObjectiveStats.Num(); i++)
    {
        if (PC->UpdatedObjectiveStats[i].Quest == QuestDef)
        {
            NeedsToAdd = false;
            return PC->UpdatedObjectiveStats[i];
        }
    }
    NeedsToAdd = true;
    return Stat;
}

void ProgressQuest(AFortPlayerControllerAthena* PC, UFortQuestItemDefinition* QuestDef, FName Primary_BackendName)
{
    auto QuestManager = PC->GetQuestManager(ESubGame::Athena);
    UFortQuestItem* QuestItem = QuestManager->GetQuestWithDefinition(QuestDef);
    log_info("Quest: %s\n", QuestDef->GetName().c_str());

    if (!QuestItem)
        return;

    log_info("Objective count: %d\n", QuestItem->GetNumObjectivesComplete());
    log_info("Percentage: %f%%\n", QuestItem->GetPercentageComplete());
    log_info("Stage: %d\n", QuestItem->CurrentStage);

    QuestManager->SelfCompletedUpdatedQuest(PC, QuestDef, Primary_BackendName, 1, QuestItem->GetNumObjectivesComplete(), nullptr, true, false);
    
    bool FinishedQuest = QuestItem->GetPercentageComplete() >= 100.f;
    if (FinishedQuest)
    {
        QuestManager->ClaimQuestReward(QuestItem);
    }
}

int AddXP(AFortPlayerControllerAthena* PC, int CombatXP, int SurvivalXP, int BonusMedalXP, int ChallengeXP, int MatchXP)
{
    UFortPlayerControllerAthenaXPComponent* XPComponent = *(UFortPlayerControllerAthenaXPComponent**)(__int64(PC) + 0x3560);

    if (XPComponent)
    {
        XPComponent->CombatXp += CombatXP;
        XPComponent->SurvivalXp += SurvivalXP;
        XPComponent->MedalBonusXP += BonusMedalXP;
        XPComponent->ChallengeXp += ChallengeXP;
        XPComponent->MatchXp += MatchXP;
        XPComponent->OnXpUpdated(CombatXP, SurvivalXP, BonusMedalXP, ChallengeXP, MatchXP, XPComponent->TotalXpEarned);
        int TotalXpEarned = (XPComponent->CombatXp + XPComponent->SurvivalXp + XPComponent->MedalBonusXP + XPComponent->ChallengeXp + XPComponent->MatchXp);
        XPComponent->TotalXpEarned += TotalXpEarned;
        XPComponent->CachedLevelInfo.LevelXp += /*XPComponent->*/TotalXpEarned;
        XPComponent->CachedLevelInfo.BookLevelXp += /*XPComponent->*/TotalXpEarned;
        XPComponent->OnXpUpdated(XPComponent->CombatXp, XPComponent->SurvivalXp, XPComponent->MedalBonusXP, XPComponent->ChallengeXp, XPComponent->MatchXp, XPComponent->TotalXpEarned);
        PlooshFNAPI::AddXP(GetAccountID(PC).ToString(), TotalXpEarned);

        return TotalXpEarned;
    }

    return 9;
}
