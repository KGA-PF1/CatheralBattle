#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_BossHit.generated.h"

UCLASS()
class CATHERALBATTLE_API UAN_BossHit : public UAnimNotify
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 HitIndex = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DamageMultiplier = 1.0f;

    virtual void Notify(USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};