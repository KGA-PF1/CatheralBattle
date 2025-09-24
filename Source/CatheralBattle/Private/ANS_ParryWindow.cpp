#include "ANS_ParryWindow.h"
#include "Boss_Sevarog.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

void UANS_ParryWindow::NotifyBegin(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    float TotalDuration,
    const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;
    if (ABoss_Sevarog* Boss = Cast<ABoss_Sevarog>(MeshComp->GetOwner()))
    {
        if (APawn* P = UGameplayStatics::GetPlayerPawn(Boss, 0))
        {
            if (APlayerCharacter* Target = Cast<APlayerCharacter>(P))
            {
                // 창 열기: 이 구간(Duration) 동안만 성공
                Boss->OnBossArmParry.Broadcast(Target, HitIndex, TotalDuration);
            }
        }
    }
}

void UANS_ParryWindow::NotifyEnd(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    // 필요 시 여기서 창 강제 종료 로직 추가 가능(현재는 Duration 타이머로 닫힘)
}
