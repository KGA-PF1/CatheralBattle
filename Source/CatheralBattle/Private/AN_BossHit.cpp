#include "AN_BossHit.h"
#include "Boss_Sevarog.h"
#include "BattleManager.h"
#include "BattleHUDWidget.h"                   // ★ 추가
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

void UAN_BossHit::Notify(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    if (ABoss_Sevarog* Boss = Cast<ABoss_Sevarog>(MeshComp->GetOwner()))
    {
        ABattleManager* BM = Cast<ABattleManager>(
            UGameplayStatics::GetActorOfClass(Boss->GetWorld(), ABattleManager::StaticClass()));
        if (!BM) return;

        APlayerCharacter* Target = BM->GetPlayerRef();               // ★ Boss->CurrentTarget 쓰지 말고 BM에서!
        if (!Target) return;

        const bool bParried = Boss->SucceededHits.Contains(HitIndex);

        if (bParried)
        {
            // 패링 성공 → 데미지 스킵 + 보상/팝업/연출
            Target->Stats.AP = FMath::Clamp(Target->Stats.AP + 1, 0, 7);
            Target->AddUltGauge(5.f);

            BM->SpawnUltPopup(Target, 5.f);
            BM->PlayParrySuccessEffectImmediate();
            BM->UpdateHUDSnapshot();
        }
        else
        {
            // 실패 → 즉시 데미지 + 팝업
            Boss->ApplyHitIfNotParried(Target, HitIndex, DamageMultiplier);
            BM->SpawnFloatingText(Target, Boss->AtkPoint * FMath::Max(0.f, DamageMultiplier));
        }
    }
}
