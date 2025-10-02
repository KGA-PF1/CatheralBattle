#include "AN_PlayerHit.h"
#include "ATBPlayerCharacter.h"
#include "Boss_Sevarog.h"
#include "BattleManager.h"
#include "BattleHUDWidget.h"                   // ★ 추가

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

        ABattleManager* BM = Cast<ABattleManager>(
            UGameplayStatics::GetActorOfClass(Player->GetWorld(), ABattleManager::StaticClass()));

        if (ABoss_Sevarog* Boss = Cast<ABoss_Sevarog>(
            UGameplayStatics::GetActorOfClass(Player->GetWorld(), ABoss_Sevarog::StaticClass())))
        {
            if (Damage > 0.f)
            {
                Boss->ApplyDamageToBoss(Damage);
                if (BM) BM->SpawnFloatingText(Boss, Damage);

                if (UWorld* World = Player->GetWorld())
                {
                    // ★ 히트스톱
                    UGameplayStatics::SetGlobalTimeDilation(World, 0.2f);

                    // ★ 카메라 쉐이크 (경로 대신 UPROPERTY로 할당된 BP 사용)
                    if (HitStopShakeClass)
                    {
                        if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
                        {
                            if (PC->PlayerCameraManager)
                            {
                                PC->PlayerCameraManager->StartCameraShake(HitStopShakeClass);
                            }
                        }
                    }

                    // 시간 복구
                    FTimerHandle HS;
                    World->GetTimerManager().SetTimer(HS, [World]()
                        {
                            UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
                        }, 0.04f, false);
                }
            }
        }

        // ★ AP+1 / Ult+5
        Player->Stats.AP = FMath::Clamp(Player->Stats.AP + 1, 0, 7);
		//플레이어 스킬 별 ultgauge 증가량 조정, 그 증가량을 변수로 저장
		float UltIncrease = 0.f;
        if (SkillInput == ESkillInput::Skill_Q) UltIncrease = 5.f;
        else if (SkillInput == ESkillInput::Skill_E) UltIncrease = 7.f;
        else if (SkillInput == ESkillInput::Attack) UltIncrease = 3.f;
		Player->AddUltGauge(UltIncrease); // 궁극기 게이지 증가

        if (BM)
        {
            BM->SpawnUltPopup(Player, UltIncrease);                          // ★ 플레이어 머리 위 +게이지 팝업
			BM->SpawnAPPopup(1.f);                             // ★ AP 팝업
            BM->UpdateHUDSnapshot();                                 // HUD 동기화
        }
    }
}
