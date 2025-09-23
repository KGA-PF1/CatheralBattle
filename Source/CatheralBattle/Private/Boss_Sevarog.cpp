#include "Boss_Sevarog.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerCharacter.h"

ABoss_Sevarog::ABoss_Sevarog()
{
	PrimaryActorTick.bCanEverTick = false;
	Hp = MaxHp = 200.f;
	AtkPoint = 10.f;
}

void ABoss_Sevarog::PlayCurrentPattern(APlayerCharacter* Target)
{
	if (!Target || CurrentPattern.Num() <= 0) return;

	// 진행 정리
	SucceededHits.Empty();
	for (FTimerHandle& H : PendingTimers) { GetWorldTimerManager().ClearTimer(H); }
	PendingTimers.Reset();

	// 애니/카메라 연출은 BP에서 재생
	// 타이밍 예약: Arm(Offset-Window) → Resolve(Offset)
	for (int32 i = 0; i < CurrentPattern.Num(); ++i)
	{
		const FBossHitSpec& Hit = CurrentPattern[i];

		// Arm: 히트 직전 창 열기 알림
		const float ArmTime = FMath::Max(0.f, Hit.QTEOffset - Hit.QTEWindow);
		FTimerHandle HArm;
		GetWorldTimerManager().SetTimer(
			HArm,
			FTimerDelegate::CreateUObject(this, &ABoss_Sevarog::FireHitCue, Target, i),
			ArmTime, false
		);
		PendingTimers.Add(HArm);

		// Resolve: 실제 히트 판정
		FTimerHandle HResolve;
		GetWorldTimerManager().SetTimer(
			HResolve,
			FTimerDelegate::CreateUObject(this, &ABoss_Sevarog::ResolveHit, Target, i),
			Hit.QTEOffset, false
		);
		PendingTimers.Add(HResolve);
	}
}

void ABoss_Sevarog::FireHitCue(APlayerCharacter* Target, int32 HitIndex)
{
	if (!Target || !CurrentPattern.IsValidIndex(HitIndex)) return;

	// 항상 패링 가능, Arm은 정답 타이밍만 알림.
	// 플레이어는 이 이벤트를 받아 자신의 패링창/버퍼를 활성화
	OnBossArmParry.Broadcast(Target, HitIndex, CurrentPattern[HitIndex].QTEWindow);
}

void ABoss_Sevarog::ResolveHit(APlayerCharacter* Target, int32 HitIndex)
{
	if (!Target || !CurrentPattern.IsValidIndex(HitIndex)) return;

	// 성공했으면 피해 0
	if (SucceededHits.Contains(HitIndex))
	{
		return;
	}

	// 실패 히트만 데미지 적용
	const FBossHitSpec& Hit = CurrentPattern[HitIndex];
	const float Damage = AtkPoint * Hit.DamageMultiplier;
	Target->TakeDamage(Damage); // PlayerCharacter의 HP 처리/브로드캐스트 사용 
}

void ABoss_Sevarog::NotifyParrySuccess(APlayerCharacter* Target, int32 HitIndex)
{
	if (!Target || !CurrentPattern.IsValidIndex(HitIndex)) return;

	SucceededHits.Add(HitIndex);

	// 모든 히트 성공 → Ult +10
	if (SucceededHits.Num() == CurrentPattern.Num())
	{
		Target->AddUltGauge(10.f); // Ult 게이지 반영(이미 구현) 
	}
}

void ABoss_Sevarog::ApplyDamageToBoss(float Damage)
{
	if (Damage <= 0.f || Hp <= 0.f) return;
	Hp = FMath::Clamp(Hp - Damage, 0.f, MaxHp);
	// HP 0 연출/파괴는 BP에서 처리
}
