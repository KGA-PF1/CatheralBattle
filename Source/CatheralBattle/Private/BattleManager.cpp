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
#include "GameFramework/CharacterMovementComponent.h"
#include "ParryComponent.h"


FTimerHandle Timer_NextTurn;

ABattleManager::ABattleManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABattleManager::BeginPlay()
{
	Super::BeginPlay();

	//APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);

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

	//if (bAutoStart) StartBattle();
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
		BossRef->OnBossHpChanged.AddDynamic(this, &ABattleManager::UpdateHUDSnapshot);       // ★
	}

	BindRuntimeSignals();
	UpdateHUDSnapshot();
}

// StartBattle: HUD 보장 + 교체 + 시작
void ABattleManager::StartBattle()
{
	if (bRunning || !PlayerRef /*|| !BossRef*/) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	// ★ 1) 현재 폰을 가장 먼저 저장
	this->OriginalPawn = PC->GetPawn();
	if (OriginalPawn)
	{
		OriginalPawnLoc = OriginalPawn->GetActorLocation();
		OriginalPawnRot = OriginalPawn->GetActorRotation();
	}

	// ★ 2) TB 스폰(앵커 적용)
	const FTransform TBXf = TBPlayerSpawnAnchor ? TBPlayerSpawnAnchor->GetActorTransform()
		: (OriginalPawn ? OriginalPawn->GetActorTransform() : FTransform::Identity);
	if (TBPlayerClass && !TBPlayer)
	{
		TBPlayer = GetWorld()->SpawnActor<ATBPlayerCharacter>(TBPlayerClass, TBXf);
	}
	if (!TBPlayer) { UE_LOG(LogTemp, Error, TEXT("[BM] TB spawn failed")); return; }

	// ★ 3) 스탯 복제(원본 → TB)
	if (APlayerCharacter* O = Cast<APlayerCharacter>(OriginalPawn))
	{
		O->MirrorAllTo(TBPlayer);
	}

	// ★ 4) 포제션 → 원본 숨김 (섀도잉 금지! 지역변수 만들지 말 것)
	PC->Possess(TBPlayer);
	PlayerRef = TBPlayer;
	SetOriginalPawnVisible(false);            // ← 원본 가시/충돌/틱/애님 OFF

	// (옵션) 원본 바닥 아래로 내려 클리핑 방지
	// if (OriginalPawn) OriginalPawn->SetActorLocation(OriginalPawnLoc + FVector(0,0,-5000));

	// ★ 5) 카메라 고정 + HUD 보장
	TBPlayer->EnsureParryMontageInjected();
	TBPlayer->ActivateTurnCamera(true);
	if ((!HUD || !HUD->IsInViewport()) && HUDClass)
	{
		HUD = CreateWidget<UBattleHUDWidget>(PC, HUDClass);
		if (HUD) { HUD->AddToViewport(50); HUD->SetVisibility(ESlateVisibility::Visible); }
	}

	bRunning = true;
	LastPatternIdx = -1;
	EnterPlayerTurn();
}


void ABattleManager::EndBattle()
{
	if (!bRunning) return;
	bRunning = false;

	SetParryEnabled(false);
	ShowPlayerUI(false);

	// 턴제 카메라 해제
	if (ATBPlayerCharacter* TB = Cast<ATBPlayerCharacter>(PlayerRef))
	{
		TB->ActivateTurnCamera(false);
	}

	// ★ 원래 플레이어 되살리고 재포제션
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (OriginalPawn)
		{
			// 위치/방향 복귀(원하면 전투장 빠져나온 위치로 갱신해도 됨)
			OriginalPawn->SetActorLocationAndRotation(OriginalPawnLoc, OriginalPawnRot);
			SetOriginalPawnVisible(true);
			PC->Possess(Cast<APawn>(OriginalPawn));
		}
	}

	// HUD 정리(원하면 유지 가능)
	if (HUD) { HUD->RemoveFromParent(); HUD = nullptr; }

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

	// ★ 3초 대기 후 보스 턴
	GetWorldTimerManager().ClearTimer(Timer_NextTurn);
	GetWorldTimerManager().SetTimer(Timer_NextTurn, this, &ABattleManager::EnterBossTurn, 3.0f, false);

	// 턴 배너 끄기
	if (HUD) HUD->ShowPlayerTurn(false);
}

void ABattleManager::EnterPlayerTurn()
{
	CurrentTurn = ETurn::Player;
	SetParryEnabled(false);
	ShowPlayerUI(true);       // 메뉴 ON
	UpdateHUDSnapshot();
	if (HUD) HUD->ShowPlayerTurn(true);   // 배너 ON
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
	if (BossRef) { BossRef->PlayPatternMontage(Chosen, PlayerRef); }

	if (HUD) HUD->ShowBossTurn(true);     // ★ 배너 ON
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
	if (HUD) HUD->SetPlayerHP(NewHp, MaxHp); // ★ 즉시 반영
	if (NewHp <= 0.f) { EndBattle(); }
}

void ABattleManager::OnBossPatternFinished()
{
	if (!bRunning) return;
	CheckEnd();
	if (!bRunning) return;
	EnterPlayerTurn();

	// ★ 3초 대기 후 플레이어 턴
	GetWorldTimerManager().ClearTimer(Timer_NextTurn);
	GetWorldTimerManager().SetTimer(Timer_NextTurn, this, &ABattleManager::EnterPlayerTurn, 3.0f, false);

	if (HUD) HUD->ShowBossTurn(false);
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

// 선택지 확정 시 플레이어 연출 + 데미지 처리
void ABattleManager::HandleMenuConfirm(EPlayerCommand Command)
{
	// 플레이어 연출(공/스/스)
	if (ATBPlayerCharacter* TB = Cast<ATBPlayerCharacter>(PlayerRef))
	{
		if (UAnimMontage* M = TB->GetMontageByCommand(Command))
		{
			PlayPlayerMontage(M); // 종료 콜백에서 턴 진행
		}
	}

	// (기존) 데미지/게이지 처리
	switch (Command) {
	case EPlayerCommand::Attack:
		if (BossRef) { BossRef->ApplyDamageToBoss(20.f); }
		if (HUD) { HUD->ShowToast(TEXT("Attack -20"), 0.7f); }
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
	// 턴 진행은 몽타주 끝(OnPlayerMontageEnded)에서.
}

void ABattleManager::OnBossDealDmg(float Amount)
{
	// 토스트
	if (HUD) { HUD->ShowToast(FString::Printf(TEXT("-%.0f"), Amount), 0.6f); }
	// ★ 히트리액트 재생(턴제 전용일 때만)
	if (ATBPlayerCharacter* TB = Cast<ATBPlayerCharacter>(PlayerRef))
	{
		TB->PlayHitReact();
	}
	UpdateHUDSnapshot();
}


// 스켈레톤 비교(컴파일 에러 수정)
void ABattleManager::PlayPlayerMontage(UAnimMontage* MontageToPlay)
{
	if (!PlayerRef) { UE_LOG(LogTemp, Error, TEXT("[BM] PlayerRef nullptr")); NotifyPlayerTurnDone(); return; }
	if (!MontageToPlay) { UE_LOG(LogTemp, Warning, TEXT("[BM] Montage null"));    NotifyPlayerTurnDone(); return; }

	USkeletalMeshComponent* Mesh = PlayerRef->GetMesh();
	if (!Mesh) { UE_LOG(LogTemp, Error, TEXT("[BM] Player Mesh nullptr")); NotifyPlayerTurnDone(); return; }

	if (!Mesh->GetSkeletalMeshAsset() || MontageToPlay->GetSkeleton() != Mesh->GetSkeletalMeshAsset()->GetSkeleton())
	{
		UE_LOG(LogTemp, Error, TEXT("[BM] Skeleton mismatch: Montage=%s  Player=%s"),
			*GetNameSafe(MontageToPlay->GetSkeleton()),
			*GetNameSafe(Mesh->GetSkeletalMeshAsset() ? Mesh->GetSkeletalMeshAsset()->GetSkeleton() : nullptr));
		NotifyPlayerTurnDone(); return;
	}

	UAnimInstance* Anim = Mesh->GetAnimInstance();
	if (!Anim) { UE_LOG(LogTemp, Error, TEXT("[BM] AnimInstance nullptr")); NotifyPlayerTurnDone(); return; }

	ShowPlayerUI(false);

	Anim->OnMontageEnded.RemoveDynamic(this, &ABattleManager::OnPlayerMontageEnded);
	Anim->OnMontageEnded.AddDynamic(this, &ABattleManager::OnPlayerMontageEnded);

	const float Len = Anim->Montage_Play(MontageToPlay, 1.0f);
	if (Len <= 0.f)
	{
		UE_LOG(LogTemp, Error, TEXT("[BM] Montage_Play failed"));
		NotifyPlayerTurnDone();
		return;
	}
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

void ABattleManager::SetOriginalPawnVisible(bool bVisible)
{
	if (!OriginalPawn) return;
	OriginalPawn->SetActorHiddenInGame(!bVisible);
	OriginalPawn->SetActorEnableCollision(bVisible);
	OriginalPawn->SetActorTickEnabled(bVisible);

	if (ACharacter* Ch = Cast<ACharacter>(OriginalPawn))
	{
		if (USkeletalMeshComponent* Sk = Ch->GetMesh())
		{
			Sk->bPauseAnims = !bVisible;
			Sk->SetVisibility(bVisible, true); // 자식 포함
		}
		if (UCharacterMovementComponent* Move = Ch->GetCharacterMovement())
		{
			if (!bVisible) Move->DisableMovement();
			else           Move->SetMovementMode(MOVE_Walking);
		}
	}
}

void ABattleManager::OnParryToast()
{
	if (HUD) {
		HUD->ShowToast(TEXT("Block!  AP +1"), 0.8f);
		HUD->SetAP(PlayerRef ? PlayerRef->Stats.AP : 0.f);
	}
}
