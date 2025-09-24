#include "AN_PatternBegin.h"
#include "Boss_Sevarog.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

void UAN_PatternBegin::Notify(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;
    if (ABoss_Sevarog* Boss = Cast<ABoss_Sevarog>(MeshComp->GetOwner()))
    {
        if (APawn* P = UGameplayStatics::GetPlayerPawn(Boss, 0))
        {
            if (APlayerCharacter* Target = Cast<APlayerCharacter>(P))
            {
                Boss->NotifyPatternBegin(Target, ExpectedHits);
            }
        }
    }
}
