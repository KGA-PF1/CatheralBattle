#include "ParryComponent.h"
#include "Boss_Sevarog.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"

UParryComponent::UParryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UParryComponent::BeginPlay()
{
	Super::BeginPlay();
	// 보스가 나중에 스폰될 수 있으니 주기적으로 Arm 델리게이트 바인딩 시도
	GetWorld()->GetTimerManager().SetTimer(TimerRebind, this, &UParryComponent::TryBindBoss, 0.5f, true, 0.0f);
}

void UParryComponent::TryBindBoss()
{
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABoss_Sevarog::StaticClass(), Found);
	for (AActor* A : Found)
	{
		if (ABoss_Sevarog* B = Cast<ABoss_Sevarog>(A))
		{
			B->OnBossArmParry.AddDynamic(this, &UParryComponent::OnBossArm);
		}
	}
	if (Found.Num() > 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerRebind);
	}
}

APlayerCharacter* UParryComponent::GetOwnerPlayer() const
{
	return Cast<APlayerCharacter>(GetOwner());
}

void UParryComponent::OnBossArm(APlayerCharacter* Target, int32 HitIndex, float WindowSec)
{
	APlayerCharacter* OwnerPC = GetOwnerPlayer();
	if (!OwnerPC || OwnerPC != Target || WindowSec <= 0.f) return;

	// Arm: 창 ON
	bArmed = true;
	ArmedHitIndex = HitIndex;
	WindowEndTime = FPlatformTime::Seconds() + (double)WindowSec;

	// 가장 가까운 보스 탐색 (TActorIterator → GetAllActorsOfClass 대체)
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABoss_Sevarog::StaticClass(), Found);

	float BestDist = TNumericLimits<float>::Max();
	ABoss_Sevarog* Closest = nullptr;
	for (AActor* A : Found)
	{
		if (ABoss_Sevarog* B = Cast<ABoss_Sevarog>(A))
		{
			const float Dist = FVector::Dist(B->GetActorLocation(), OwnerPC->GetActorLocation());
			if (Dist < BestDist) { BestDist = Dist; Closest = B; }
		}
	}
	ArmedBoss = Closest;

	// 버퍼: 창 직전 입력 있었다면 즉시 성공
	const double Now = FPlatformTime::Seconds();
	if (!bLocked && (Now - LastPressTime) <= (double)BufferBeforeOpen)
	{
		HandleParrySuccess();
		return;
	}

	// 창 만료 타이머
	GetWorld()->GetTimerManager().ClearTimer(TimerExpire);
	GetWorld()->GetTimerManager().SetTimer(TimerExpire, this, &UParryComponent::ExpireArm, WindowSec, false);
}


void UParryComponent::ExpireArm()
{
	bArmed = false;
	ArmedHitIndex = -1;
	ArmedBoss = nullptr;
	WindowEndTime = 0.0;
}

void UParryComponent::OnParryPressed()
{
	const double Now = FPlatformTime::Seconds();
	LastPressTime = Now;

	// 모션은 항상 재생(성공/실패 공통 피드백)
	PlayParryMontage();

	// 리커버리 중이면 판정 불가
	if (bLocked) return;

	// 창 안이면 성공
	if (bArmed && Now <= WindowEndTime)
	{
		HandleParrySuccess();
		return;
	}

	// 실패 → 리커버리 진입
	BeginRecovery();
}

void UParryComponent::HandleParrySuccess()
{
	APlayerCharacter* OwnerPC = GetOwnerPlayer();
	if (!OwnerPC || !ArmedBoss.IsValid() || ArmedHitIndex < 0) { ExpireArm(); return; }

	// AP +1 (최대 6)
	OwnerPC->Stats.AP = FMath::Clamp(OwnerPC->Stats.AP + 1.f, 0.f, 6.f);

	// 보스에 히트 성공 보고(모든 히트 성공 시 Ult+10은 보스가 처리)
	ArmedBoss->NotifyParrySuccess(OwnerPC, ArmedHitIndex);

	ExpireArm();
}

void UParryComponent::BeginRecovery()
{
	bLocked = true;
	GetWorld()->GetTimerManager().ClearTimer(TimerRecovery);
	GetWorld()->GetTimerManager().SetTimer(TimerRecovery, this, &UParryComponent::EndRecovery, RecoveryTime, false);
}

void UParryComponent::EndRecovery()
{
	bLocked = false;
}

void UParryComponent::PlayParryMontage() const
{
	if (!ParryMontage) return;
	if (ACharacter* Ch = Cast<ACharacter>(GetOwner()))
	{
		if (UAnimInstance* Anim = Ch->GetMesh() ? Ch->GetMesh()->GetAnimInstance() : nullptr)
		{
			Anim->Montage_Play(ParryMontage, 1.0f);
		}
	}
}
