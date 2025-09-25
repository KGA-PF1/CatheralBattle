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

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);

	//if (HUDClass && PC && !HUD)
	//{
	//	HUD = CreateWidget<UBattleHUDWidget>(PC, HUDClass);   // ★ Owning Player!
	//	if (HUD)
	//	{
	//		HUD->SetVisibility(ESlateVisibility::Visible);
	//		HUD->AddToViewport(5);                             // ★ Z 높임
	//		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("HUD SPAWNED"));
	//	}
	//}

	if (bAutoStart) StartBattle();
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

	if ((!HUD || !HUD->IsInViewport()) && HUDClass)
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
		{
			HUD = CreateWidget<UBattleHUDWidget>(PC, HUDClass);
			if (HUD) { HUD->AddToViewport(50); HUD->SetVisibility(ESlateVisibility::Visible); }
		}
	}
	bRunning = true;
	LastPatternIdx = -1;
	EnterPlayerTurn();

	if (!HUDClass) { UE_LOG(LogTemp, Error, TEXT("[BM] HUDClass not set!")); }
	if (!TurnMenuClass) { UE_LOG(LogTemp, Error, TEXT("[BM] TurnMenuClass not set!")); }
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

void ABattleManager::ShowPlayerUI(bool bShowMenu)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	if (bShowMenu)
	{
		if (!TurnMenu && TurnMenuClass)
		{
			TurnMenu = CreateWidget<UTurnMenuWidget>(PC, TurnMenuClass); // ★ Owning Player
			if (TurnMenu)
			{
				TurnMenu->OnConfirm.AddDynamic(this, &ABattleManager::HandleMenuConfirm);

				TurnMenu->AddToViewport(100);                             // 메뉴 최상단
				TurnMenu->SetUltimateEnabled(PlayerRef && PlayerRef->GetUltGauge() >= 100.f);
			}
		}
		if (TurnMenu)
		{
			FInputModeUIOnly M;
			M.SetWidgetToFocus(TurnMenu->TakeWidget());                  // ★ 포커스 대상 지정
			M.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(M);
			PC->bShowMouseCursor = false;

			TurnMenu->SetVisibility(ESlateVisibility::Visible);
			TurnMenu->SetIsEnabled(true);
			TurnMenu->SetKeyboardFocus();                                 // ★ 백업 포커스
			PC->SetShowMouseCursor(false);
		}
	}
	else
	{
		if (TurnMenu) { TurnMenu->RemoveFromParent(); TurnMenu = nullptr; }
		FInputModeGameOnly G; PC->SetInputMode(G); PC->bShowMouseCursor = false;
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

void ABattleManager::PlayPlayerMontage(UAnimMontage* MontageToPlay)
{
	if (!PlayerRef) { UE_LOG(LogTemp, Error, TEXT("[BM] PlayerRef nullptr")); NotifyPlayerTurnDone(); return; }
	if (!MontageToPlay) { UE_LOG(LogTemp, Warning, TEXT("[BM] Montage null")); NotifyPlayerTurnDone(); return; }

	USkeletalMeshComponent* Mesh = PlayerRef->GetMesh();
	if (!Mesh) { UE_LOG(LogTemp, Error, TEXT("[BM] Player Mesh nullptr")); NotifyPlayerTurnDone(); return; }

	// 스켈레톤 확인
	if (MontageToPlay->GetSkeleton() != Mesh->GetSkeletalMeshAsset()->GetSkeleton())
	{
		UE_LOG(LogTemp, Error, TEXT("[BM] Skeleton mismatch: Montage=%s  Player=%s"),
			*MontageToPlay->GetSkeleton()->GetName(),
			*Mesh->GetSkeletalMeshAsset()->GetSkeleton()->GetName());
		NotifyPlayerTurnDone();
		return;
	}

	// 애님 인스턴스 확보
	UAnimInstance* Anim = Mesh->GetAnimInstance();
	if (!Anim) { UE_LOG(LogTemp, Error, TEXT("[BM] AnimInstance nullptr (Anim Class 미지정?)")); NotifyPlayerTurnDone(); return; }

	// 슬롯 이름 로그(슬롯 불일치 디버그)
	if (MontageToPlay->SlotAnimTracks.Num() > 0)
	{
		const FName SlotName = MontageToPlay->SlotAnimTracks[0].SlotName;
		UE_LOG(LogTemp, Log, TEXT("[BM] Play Montage=%s Slot=%s"), *MontageToPlay->GetName(), *SlotName.ToString());
	}

	// 메뉴 숨기고 게임 입력으로
	ShowPlayerUI(false);

	// 중복 바인딩 방지
	Anim->OnMontageEnded.RemoveDynamic(this, &ABattleManager::OnPlayerMontageEnded);
	Anim->OnMontageEnded.AddDynamic(this, &ABattleManager::OnPlayerMontageEnded);

	// 재생!
	const float Len = Anim->Montage_Play(MontageToPlay, 1.0f);
	if (Len <= 0.f)
	{
		UE_LOG(LogTemp, Error, TEXT("[BM] Montage_Play failed! (슬롯 미연결 또는 블렌드 규칙 충돌)"));
		// 실패시 바로 턴 진행 막힘 방지
		NotifyPlayerTurnDone();
		return;
	}

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, TEXT("PLAYER MONTAGE PLAY"));
}


void ABattleManager::OnPlayerMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (UAnimInstance* Anim = PlayerRef && PlayerRef->GetMesh() ? PlayerRef->GetMesh()->GetAnimInstance() : nullptr)
	{
		Anim->OnMontageEnded.RemoveDynamic(this, &ABattleManager::OnPlayerMontageEnded);
	}
	UpdateHUDSnapshot();
	NotifyPlayerTurnDone();
}