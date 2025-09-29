#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PlayerCharacter.h"
#include "AN_PlayerHit.generated.h"

UCLASS()
class CATHERALBATTLE_API UAN_PlayerHit : public UAnimNotify
{
    GENERATED_BODY()
public:
    // 어떤 스킬인지 (Attack, Skill_Q, Skill_E, Ult_R)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit")
    ESkillInput SkillInput = ESkillInput::Attack;

    virtual void Notify(USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;
};