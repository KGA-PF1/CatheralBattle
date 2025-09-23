#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Boss_Sevarog.generated.h"

class APlayerCharacter;

/** 히트별 패링/데미지 스펙 */
USTRUCT(BlueprintType)
struct FBossHitSpec
{
	GENERATED_BODY()

	/** 최종피해 = AtkPoint * DamageMultiplier (예: 1.0f) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Hit")
	float DamageMultiplier = 1.0f;

	/** 모션 시작 후 이 히트가 들어가는 시점(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Hit")
	float QTEOffset = 0.6f;

	/** 패링 가능 시간(초). 예: 0.15f */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Hit")
	float QTEWindow = 0.15f;
};

/** 보스가 패링창을 여는 순간을 알림(플레이어가 이 이벤트를 받아 패링창 활성) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBossArmParry, APlayerCharacter*, Target, int32, HitIndex, float, WindowSec);

/**
 * Paragon Sevarog 느낌의 보스.
 * - HP 200, AtkPoint 10
 * - 다단히트 패턴: 히트 직전에 Arm → 창 내 입력 성공 시 피해 0 & AP+1
 * - 패턴의 모든 히트 성공 시 플레이어 Ult +10
 * - “항상 패링 가능” 설계 지원: Arm은 단지 ‘정답 타이밍’ 창을 알려줌
 */
UCLASS()
class CATHERALBATTLE_API ABoss_Sevarog : public ACharacter
{
	GENERATED_BODY()

public:
	ABoss_Sevarog();

	/** ====== 스탯 ====== */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat")
	float MaxHp = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat")
	float Hp = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat")
	float AtkPoint = 10.f;

	/** ====== 패턴 정의(다단히트) ====== */
	/** 이번에 재생할 패턴(히트 배열). BP에서 세팅: 2~3연타 등 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	TArray<FBossHitSpec> CurrentPattern;

	/** 보스 패턴 재생(애니/카메라는 BP에서; 이 함수는 Arm/판정 예약) */
	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	void PlayCurrentPattern(APlayerCharacter* Target);

	/** 플레이어가 패링 성공 시 보스에 보고(히트별) */
	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	void NotifyParrySuccess(APlayerCharacter* Target, int32 HitIndex);

	/** 보스 피해(플레이어 턴 등에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Boss|Stat")
	void ApplyDamageToBoss(float Damage);

	/** 보스가 Arm을 알리는 델리게이트(플레이어가 바인딩해서 패링창 활성) */
	UPROPERTY(BlueprintAssignable, Category = "Boss|Event")
	FOnBossArmParry OnBossArmParry;

protected:
	/** 히트 직전: 패링창 Arm 신호 */
	void FireHitCue(APlayerCharacter* Target, int32 HitIndex);

	/** 히트 시점: 실패 히트만 피해 적용 */
	void ResolveHit(APlayerCharacter* Target, int32 HitIndex);

private:
	/** 이번 패턴에서 성공한 히트 인덱스 집합 */
	TSet<int32> SucceededHits;

	/** 예약 타이머 핸들(정리용) */
	TArray<FTimerHandle> PendingTimers;
};
