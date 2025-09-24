#include "BattleManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "PlayerCharacter.h"
#include "Boss_Sevarog.h"
#include "ParryInputProxy.h"
#include "Animation/AnimMontage.h"

ABattleManager::ABattleManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABattleManager::BeginPlay()
{
	Super::BeginPlay();
	if (!ParryProxy) { TryAutoWireProxy(); }
	if (bAutoStart) { StartBattle(); }
}

void ABattleManager::Initialize(APlayerCharacter* InPlayer, ABoss_Sevarog* InBoss, AParryInputProxy* InProxy)
{
	PlayerRef = InPlayer;
	BossRef = InBoss;
	if (InProxy) { ParryProxy = InProxy; }
	else if (!ParryProxy) { TryAutoWireProxy(); }

	// 플레이어 사망 감지
	if (PlayerRef)
	{
		PlayerRef->OnHpChanged.AddDynamic(this, &ABattleManager::OnPlayerHpChanged);
	}
	// 보스 패턴 종료 감지
	if (BossRef)
	{
		BossRef->OnPatternFinished.AddDynamic(this, &ABattleManager::OnBossPatternFinished);
	}
}

void ABattleManager::StartBattle()
{
	if (bRunning || !PlayerRef || !BossRef) return;
	bRunning = true;
	LastPatternIdx = -1;
	EnterPlayerTurn();
}

void ABattleManager::EndBattle()
{
	if (!bRunning) return;
	bRunning = false;

	SetParryEnabled(false);

	if (PlayerRef) { PlayerRef->OnHpChanged.RemoveDynamic(this, &ABattleManager::OnPlayerHpChanged); }
	if (BossRef) { BossRef->OnPatternFinished.RemoveDynamic(this, &ABattleManager::OnBossPatternFinished); }
}

void ABattleManager::NotifyPlayerTurnDone()
{
	if (!bRunning) return;
	CheckEnd();
	if (!bRunning) return;
	EnterBossTurn();
}

void ABattleManager::EnterPlayerTurn()
{
	CurrentTurn = ETurn::Player;
	SetParryEnabled(false); // 보스턴 아님 → 패링 비활성
	// 플레이어 커맨드 입력/UI는 게임 쪽에서 처리 후 NotifyPlayerTurnDone 호출!
}

void ABattleManager::EnterBossTurn()
{
	CurrentTurn = ETurn::Boss;
	SetParryEnabled(true); // 보스턴 → 패링 활성

	// 몽타주 선택
	UAnimMontage* Chosen = nullptr;
	if (BossPatternMontages.Num() > 0)
	{
		const int32 Idx = PickMontageIndex();
		Chosen = BossPatternMontages[Idx];
		LastPatternIdx = Idx;
	}
	// 패턴 재생(Notify들이 패턴 Begin/Window/Hit/End를 처리)
	BossRef->PlayPatternMontage(Chosen, PlayerRef);
}

int32 ABattleManager::PickMontageIndex() const
{
	if (BossPatternMontages.Num() <= 1 || !bDisallowRepeat || LastPatternIdx < 0)
	{
		return FMath::Clamp(FMath::RandRange(0, BossPatternMontages.Num() - 1), 0, BossPatternMontages.Num() - 1);
	}
	int32 Idx;
	do { Idx = FMath::RandRange(0, BossPatternMontages.Num() - 1); } while (Idx == LastPatternIdx);
	return Idx;
}

void ABattleManager::SetParryEnabled(bool bEnable)
{
	if (!ParryProxy)
	{
		TryAutoWireProxy();
		if (!ParryProxy) return;
	}
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (bEnable) { ParryProxy->EnableInput(PC); }
		else { ParryProxy->DisableInput(PC); }
	}
}

void ABattleManager::TryAutoWireProxy()
{
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AParryInputProxy::StaticClass(), Found);
	if (Found.Num() > 0) { ParryProxy = Cast<AParryInputProxy>(Found[0]); }
}

void ABattleManager::OnPlayerHpChanged(float NewHp, float MaxHp)
{
	if (NewHp <= 0.f) { EndBattle(); }
}

void ABattleManager::OnBossPatternFinished()
{
	if (!bRunning) return;
	CheckEnd();
	if (!bRunning) return;
	EnterPlayerTurn();
}

void ABattleManager::CheckEnd()
{
	if (!PlayerRef || !BossRef) { EndBattle(); return; }
	if (PlayerRef->IsDead()) { EndBattle(); return; }
	if (BossRef->Hp <= 0.f) { EndBattle(); return; }
}
