#include "AN_PlayerHit.h"
#include "ATBPlayerCharacter.h"
#include "Boss_Sevarog.h"
#include "BattleManager.h"
#include "Kismet/GameplayStatics.h"


void UAN_PlayerHit::Notify(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    if (ATBPlayerCharacter* Player = Cast<ATBPlayerCharacter>(MeshComp->GetOwner()))
    {
        float Damage = Player->GetSkillDamage(SkillInput);
        if (Damage <= 0.f && SkillInput == ESkillInput::Attack)
        {
            Damage = Player->Stats.AtkPoint;
        }

        if (ABoss_Sevarog* Boss = Cast<ABoss_Sevarog>(
            UGameplayStatics::GetActorOfClass(Player->GetWorld(), ABoss_Sevarog::StaticClass())))
        {
            if (Damage > 0.f) Boss->ApplyDamageToBoss(Damage);
        }

        // ★ 기본공격일 경우 여기서 즉시 AP +1
        if (SkillInput == ESkillInput::Attack)
        {
            Player->Stats.AP = FMath::Clamp(Player->Stats.AP + 1, 0, 7);

            if (ABattleManager* BM = Cast<ABattleManager>(
                UGameplayStatics::GetActorOfClass(Player->GetWorld(), ABattleManager::StaticClass())))
            {
                BM->UpdateHUDSnapshot();
            }
        }
    }
}
