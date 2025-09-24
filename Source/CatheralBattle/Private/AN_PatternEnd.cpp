#include "AN_PatternEnd.h"
#include "Boss_Sevarog.h"

void UAN_PatternEnd::Notify(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;
    if (ABoss_Sevarog* Boss = Cast<ABoss_Sevarog>(MeshComp->GetOwner()))
    {
        Boss->NotifyPatternEnd();
    }
}
