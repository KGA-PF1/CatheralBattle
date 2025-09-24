#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Boss_Sevarog.generated.h"

class UAnimMontage;
class APlayerCharacter;

/** 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPatternFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBossArmParry, APlayerCharacter*, Target, int32, HitIndex, float, WindowSec);


/** 보스(Notify 구동형) */
UCLASS()
class CATHERALBATTLE_API ABoss_Sevarog : public ACharacter
{
	GENERATED_BODY()

public:
	ABoss_Sevarog();

	/** 스탯 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat") float MaxHp = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat") float Hp = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat") float AtkPoint = 10.f;

	/** 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Boss|Event") FOnBossArmParry    OnBossArmParry;
	UPROPERTY(BlueprintAssignable, Category = "Boss|Event") FOnPatternFinished OnPatternFinished;

	/** 패턴 재생(몽타주) */
	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	void PlayPatternMontage(UAnimMontage* Montage, APlayerCharacter* Target);

	/** 패턴 라이프사이클/히트 처리(Notify에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern") void NotifyPatternBegin(APlayerCharacter* Target, int32 InExpectedHits);
	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern") void NotifyPatternEnd();
	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern") void ApplyHitIfNotParried(APlayerCharacter* Target, int32 HitIndex, float DamageMultiplier);
	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern") void NotifyParrySuccess(APlayerCharacter* Target, int32 HitIndex);

	/** 보스 피해(플레이어 턴 등) */
	UFUNCTION(BlueprintCallable, Category = "Boss|Stat") void ApplyDamageToBoss(float Damage);

private:
	/** 이번 패턴 대상/성공기록 */
	UPROPERTY() TWeakObjectPtr<APlayerCharacter> CurrentTarget;
	TSet<int32> SucceededHits;
	int32 ExpectedHits = 0;
};
