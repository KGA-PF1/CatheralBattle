#include "Boss_Sevarog.h"
#include "Animation/AnimInstance.h"
#include "PlayerCharacter.h"
#include "BattleManager.h"
#include <Kismet/GameplayStatics.h>

ABoss_Sevarog::ABoss_Sevarog()
{
	PrimaryActorTick.bCanEverTick = false;
	Hp = MaxHp = 200.f;
	AtkPoint = 10.f;
}

void ABoss_Sevarog::PlayPatternMontage(UAnimMontage* Montage, APlayerCharacter* Target)
{
	CurrentTarget = Target;
	SucceededHits.Empty();
	if (Montage && GetMesh())
	{
		if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
		{
			Anim->Montage_Play(Montage, 1.0f);
		}
	}
}

void ABoss_Sevarog::NotifyPatternBegin(APlayerCharacter* Target, int32 InExpectedHits)
{
	CurrentTarget = Target;
	ExpectedHits = FMath::Max(0, InExpectedHits);
	SucceededHits.Empty();
}

void ABoss_Sevarog::NotifyPatternEnd()
{
	OnPatternFinished.Broadcast();
	ExpectedHits = 0;
	SucceededHits.Empty();
	CurrentTarget = nullptr;
}

void ABoss_Sevarog::ApplyHitIfNotParried(APlayerCharacter* Target, int32 HitIndex, float DamageMultiplier)
{
	if (!Target) return;

	if (SucceededHits.Contains(HitIndex)) return;

	const float Dmg = AtkPoint * FMath::Max(0.f, DamageMultiplier);
	Target->TakeDamage(Dmg);
	OnBossDealtDamage.Broadcast(Dmg);
}

void ABoss_Sevarog::NotifyParrySuccess(APlayerCharacter* Target, int32 HitIndex)
{
	if (!Target) return;

	//SucceededHits.Add(HitIndex);

	// ★ 모든 히트 성공 순간에 퍼펙트 신호
	if (ExpectedHits > 0 && SucceededHits.Num() == ExpectedHits)
	{
		Target->AddUltGauge(10.f);
		OnPatternPerfect.Broadcast();
	}
}

void ABoss_Sevarog::ApplyDamageToBoss(float Damage)
{
	if (Damage <= 0.f || Hp <= 0.f) return;

	const float Old = Hp;
	Hp = FMath::Clamp(Hp - Damage, 0.f, MaxHp);
	if (!FMath::IsNearlyEqual(Old, Hp))
	{
		OnBossHpChanged.Broadcast();
	}

	// ★ BM 통해 보스 HitReact 실행 (통합 관리)
	if (ABattleManager* BM = Cast<ABattleManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABattleManager::StaticClass())))
	{
		BM->PlayBossHitReact();
	}
}


void ABoss_Sevarog::PlayHitReact()
{
	if (UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		if (HitReactMontage)
		{
			Anim->Montage_Play(HitReactMontage);
		}
	}
}