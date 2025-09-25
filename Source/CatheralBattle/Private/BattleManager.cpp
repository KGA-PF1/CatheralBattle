#include "BattleManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "PlayerCharacter.h"
#include "Boss_Sevarog.h"
#include "ParryInputProxy.h"
#include "Animation/AnimMontage.h"
#include "TurnMenuWidget.h"
#include "BattleHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "ParryComponent.h"


ABattleManager::ABattleManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABattleManager::BeginPlay()
{
	Super::BeginPlay();
	if (!ParryProxy) { TryAutoWireProxy(); }

	// UI 생성
	if (HUDClass)
	{
		HUD = CreateWidget<UBattleHUDWidget>(GetWorld(), HUDClass);
		if (HUD) { HUD->AddToViewport(0); }
	}

	if (bAutoStart) { StartBattle(); }
}

void ABattleManager::Initialize(APlayerCharacter* InPlayer, ABoss_Sevarog* InBoss, AParryInputProxy* InProxy)
{
	PlayerRef = InPlayer;
	BossRef = InBoss;
	if (InProxy) { ParryProxy = InProxy; }
	else if (!ParryProxy) { TryAutoWireProxy(); }

	// Parry 성공 토스트
	if (UParryComponent* PCmp = PlayerRef->FindComponentByClass<UParryComponent>()) {
		PCmp->OnParrySuccess.AddDynamic(this, &ABattleManager::OnParryToast);
	}
	if (BossRef) { BossRef->OnBossDealtDamage.AddDynamic(this, &ABattleManager::OnBossDealDmg); }


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

	BindRuntimeSignals();
	UpdateHUDSnapshot();
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
	// 메뉴 OFF
	ShowPlayerUI(false);
	CheckEnd();
	if (!bRunning) return;
	EnterBossTurn();
}

void ABattleManager::EnterPlayerTurn()
{
	CurrentTurn = ETurn::Player;
	SetParryEnabled(false);
	ShowPlayerUI(true);       // 메뉴 ON
	UpdateHUDSnapshot();
}

void ABattleManager::EnterBossTurn() {
	CurrentTurn = ETurn::Boss;
	SetParryEnabled(true);
	ShowPlayerUI(false);      // 메뉴 OFF

	// ★ 패턴 선택 + 실제 재생
	UAnimMontage* Chosen = nullptr;
	if (BossPatternMontages.Num() > 0)
	{
		const int32 Idx = PickMontageIndex();
		Chosen = BossPatternMontages[Idx];
		LastPatternIdx = Idx;
	}
	if (BossRef) { BossRef->PlayPatternMontage(Chosen, PlayerRef); } // ← 몽타주 GO}
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

void ABattleManager::BindRuntimeSignals() {
	if (PlayerRef) {
		PlayerRef->OnHpChanged.AddDynamic(this, &ABattleManager::OnPlayerHpChanged);
		// 플레이어 Ult/AP 이벤트가 있다면 여기에 추가 바인딩(없으면 액션 시 직접 HUD 갱신)
	}
	if (BossRef) {
		BossRef->OnPatternFinished.AddDynamic(this, &ABattleManager::OnBossPatternFinished);
		// 보스 HP 변경/피해 딜리게이트가 있으면 바인딩해서 HUD/토스트 표기
	}
	// Parry 성공 토스트: 플레이어의 ParryComponent 찾아 바인딩(있으면)
}


void ABattleManager::CheckEnd()
{
	if (!PlayerRef || !BossRef) { EndBattle(); return; }
	if (PlayerRef->IsDead()) { EndBattle(); return; }
	if (BossRef->Hp <= 0.f) { EndBattle(); return; }
}

void ABattleManager::ShowPlayerUI(bool bShowMenu) {
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	// HUD는 항상 보이게(이미 Add됨)
	if (bShowMenu) {
		if (!TurnMenu && TurnMenuClass) {
			TurnMenu = CreateWidget<UTurnMenuWidget>(GetWorld(), TurnMenuClass);
			if (TurnMenu) {
				TurnMenu->AddToViewport(100);
				// 궁극기 가능여부 세팅
				bool bUlt = false;
				if (PlayerRef) {
					// PlayerCharacter에 게터가 있으면 사용
					if constexpr (true) { /* 그냥 예시 블록 */ }
					// 1) GetUltGauge()가 있으면:
					if (PlayerRef->GetUltGauge() >= 100.f) bUlt = true;
					// (만약 GetUltGauge가 없다면, 아래 3) 참고해서 게터 추가)
				}
				TurnMenu->SetUltimateEnabled(bUlt);

				// Confirm → 실제 실행 → 토스트 → NotifyPlayerTurnDone
				TurnMenu->OnConfirm.AddDynamic(this, &ABattleManager::HandleMenuConfirm);
			}
		}
		// 입력모드 UIOnly
		FInputModeUIOnly M; M.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(M); PC->bShowMouseCursor = false;
	}
	else {
		if (TurnMenu) { TurnMenu->RemoveFromParent(); TurnMenu = nullptr; }
		// 입력모드 GameOnly (보스턴에 패링키만)
		FInputModeGameOnly M; PC->SetInputMode(M); PC->bShowMouseCursor = false;
	}
}

void ABattleManager::UpdateHUDSnapshot() {
	if (!HUD) return;
	if (PlayerRef) { HUD->SetPlayerHP(PlayerRef->GetHp(), PlayerRef->GetMaxHp()); HUD->SetAP(PlayerRef->Stats.AP); HUD->SetUlt(PlayerRef->GetUltGauge(), 100.f); }
	if (BossRef) { HUD->SetBossHP(BossRef->Hp, BossRef->MaxHp); }
}

void ABattleManager::HandleMenuConfirm(EPlayerCommand Command)
{
	// 아주 간단한 실행(연출은 BP에서)
	switch (Command) {
	case EPlayerCommand::Attack:
		if (BossRef) { BossRef->ApplyDamageToBoss(20.f); }
		if (HUD) { HUD->ShowToast(TEXT("Attack -20"), 0.7f); }
		// 일반공격 AP+1
		if (PlayerRef) { PlayerRef->Stats.AP = FMath::Clamp(PlayerRef->Stats.AP + 1.f, 0.f, 6.f); if (HUD) HUD->SetAP(PlayerRef->Stats.AP); HUD->ShowToast(TEXT("AP +1"), 0.7f); }
		break;
	case EPlayerCommand::Skill1:
		if (BossRef) { BossRef->ApplyDamageToBoss(30.f); }
		if (HUD) { HUD->ShowToast(TEXT("Skill1 -30"), 0.7f); }
		break;
	case EPlayerCommand::Skill2:
		if (BossRef) { BossRef->ApplyDamageToBoss(30.f); }
		if (HUD) { HUD->ShowToast(TEXT("Skill2 -30"), 0.7f); }
		break;
	case EPlayerCommand::Ultimate:
		if (PlayerRef && PlayerRef->GetUltGauge() >= 100.f) {
			if (BossRef) { BossRef->ApplyDamageToBoss(120.f); }
			PlayerRef->AddUltGauge(-100.f);
			if (HUD) { HUD->SetUlt(PlayerRef->GetUltGauge(), 100.f); HUD->ShowToast(TEXT("ULT! -120"), 0.9f); }
		}
		else {
			if (HUD) { HUD->ShowToast(TEXT("Not enough ULT"), 0.6f); }
			return;
		}
		break;
	}
	UpdateHUDSnapshot();
	NotifyPlayerTurnDone();
}

void ABattleManager::OnParryToast()
{
	if (HUD) { HUD->ShowToast(TEXT("Block!  AP +1"), 0.8f); HUD->SetAP(PlayerRef ? PlayerRef->Stats.AP : 0.f); }
}

void ABattleManager::OnBossDealDmg(float Amount)
{
	if (HUD) { HUD->ShowToast(FString::Printf(TEXT("-%.0f"), Amount), 0.6f); }
	UpdateHUDSnapshot();
}