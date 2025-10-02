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

static bool IsUltReady(APlayerCharacter* P) {
	return (P && P->GetUltGauge() >= 100.f);
}

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



	if (BossRef) { BossRef->OnBossDealtDamage.AddDynamic(this, &ABattleManager::OnBossDealDmg); }

	// 플레이어 사망 감지
	if (PlayerRef)
	{
		PlayerRef->OnHpChanged.AddDynamic(this, &ABattleManager::OnPlayerHpChanged);
		PlayerRef->OnUltGaugeChanged.AddDynamic(this, &ABattleManager::OnPlayerUltChanged);
	}
	// 보스 패턴 종료 감지
	if (BossRef)
	{
		BossRef->OnPatternFinished.AddDynamic(this, &ABattleManager::OnBossPatternFinished);
		BossRef->OnPatternPerfect.AddDynamic(this, &ABattleManager::OnBossPatternPerfect);
		BossRef->OnBossHpChanged.AddDynamic(this, &ABattleManager::UpdateHUDSnapshot);
	}

	if (BossRef && PlayerRef)
	{
		if (UParryComponent* PC = PlayerRef->FindComponentByClass<UParryComponent>())
		{
			PC->OnParrySuccess.Clear(); // 중복 방지
			PC->OnParrySuccess.AddDynamic(this, &ABattleManager::OnParryToast);
			BossRef->OnBossArmParry.AddDynamic(PC, &UParryComponent::OnBossArm);
		}
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

	if (OriginalPawn)
	{
		if (APlayerCharacter* OldP = Cast<APlayerCharacter>(OriginalPawn))
		{
			if (UParryComponent* OldPC = OldP->FindComponentByClass<UParryComponent>())
			{
				if (BossRef)
				{
					BossRef->OnBossArmParry.RemoveDynamic(OldPC, &UParryComponent::OnBossArm);
				}
				OldPC->OnParrySuccess.RemoveDynamic(this, &ABattleManager::OnParryToast);
			}
		}
	}

	// 2) TB 플레이어의 Parry 바인딩 추가(중복 없이)
	if (UParryComponent* NewPC = PlayerRef->FindComponentByClass<UParryComponent>())
	{
		if (BossRef)
		{
			BossRef->OnBossArmParry.AddUniqueDynamic(NewPC, &UParryComponent::OnBossArm);
			BossRef->OnPatternPerfect.AddUniqueDynamic(this, &ABattleManager::OnBossPatternPerfect);
		}
		NewPC->OnParrySuccess.RemoveDynamic(this, &ABattleManager::OnParryToast); // 안전차단
		NewPC->OnParrySuccess.AddUniqueDynamic(this, &ABattleManager::OnParryToast);
	}

	// 3) 프록시도 TB로 다시 캐싱(이미 있음)
	if (ParryProxy) { ParryProxy->CacheParryComponent(); }

	if (ATBPlayerCharacter* TB = Cast<ATBPlayerCharacter>(PlayerRef))
	{
		TB->bTurnBased = true; // 스킬 사용 시 AP 체크 발동
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

	// ★ 3초 대기 후 보스 턴
	GetWorldTimerManager().ClearTimer(Timer_NextTurn);
	GetWorldTimerManager().SetTimer(Timer_NextTurn, this, &ABattleManager::EnterBossTurn, 1.0f, false);
}

void ABattleManager::EnterPlayerTurn()
{
	CurrentTurn = ETurn::Player;
	SetParryEnabled(false);
	ShowPlayerUI(true);
	UpdateHUDSnapshot();

	if (HUD) {
		HUD->ShowPlayerTurn(true);
	}
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

	if (HUD) {
		HUD->ShowBossTurn(true);
	}
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

void ABattleManager::OnPlayerUltChanged(float Current, float Max)
{
	if (HUD) HUD->SetUlt(Current, Max);
	if (TurnMenu)
	{
		TurnMenu->UpdateUltReady(Current >= Max);
	}
}

void ABattleManager::OnBossPatternFinished()
{
	if (!bRunning) return;
	CheckEnd();
	if (!bRunning) return;

	// ★ 3초 대기 후 플레이어 턴
	GetWorldTimerManager().ClearTimer(Timer_NextTurn);
	GetWorldTimerManager().SetTimer(Timer_NextTurn, this, &ABattleManager::EnterPlayerTurn, 1.0f, false);
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
				TurnMenu->AddToViewport(100);        

				const bool bReady = IsUltReady(PlayerRef);
				TurnMenu->SetUltimateEnabled(bReady);
				TurnMenu->UpdateUltReady(bReady);
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
	if (TurnMenu) { TurnMenu->UpdateUltReady(IsUltReady(PlayerRef)); }
}

void ABattleManager::PlayParrySuccessEffectImmediate()
{
	if (HUD) HUD->PlayParrySuccessEffect();
	UpdateHUDSnapshot();
}

// 선택지 확정 시 플레이어 연출 + 데미지 처리
void ABattleManager::HandleMenuConfirm(EPlayerCommand Command)
{
	if (!PlayerRef) return;

	auto UseAP = [&](int32 Cost) -> bool {
		if (PlayerRef->Stats.AP < Cost) {
			if (HUD) HUD->ShowToast(TEXT("Not enough AP"), 0.7f);
			return false;
		}
		PlayerRef->Stats.AP -= Cost;
		HUD->SetAP(PlayerRef->Stats.AP);
		return true;
	};

	bool bCanExecute = true;
	switch (Command) {
	case EPlayerCommand::Attack:
		break; // Attack은 AP 소모 없음
	case EPlayerCommand::Skill1:
		if (!UseAP(2)) bCanExecute = false;
		break;
	case EPlayerCommand::Skill2:
		if (!UseAP(3)) bCanExecute = false;
		break;
	case EPlayerCommand::Ultimate:
		if (PlayerRef->GetUltGauge() < 100.f) {
			HUD->ShowToast(TEXT("Not enough Ult"), 0.7f);
			bCanExecute = false;
		}
		if (bCanExecute) {
			PlayerRef->AddUltGauge(-100.f);
			HUD->SetUlt(PlayerRef->GetUltGauge(), 100.f);
		}
		break;
	}

	if (!bCanExecute) return; // 조건 안 맞으면 스킬 취소

	PendingCommand = Command;

	if (ATBPlayerCharacter* TB = Cast<ATBPlayerCharacter>(PlayerRef))
	{
		if (UAnimMontage* M = TB->GetMontageByCommand(Command))
			PlayPlayerMontage(M);
	}
}

void ABattleManager::OnBossDealDmg(float Amount)
{
	if (!PlayerRef) return;

	// ★ 체력 감소
	PlayerRef->TakeDamage(Amount);

	// 토스트 출력
	if (HUD)
	{
		HUD->ShowToast(FString::Printf(TEXT("Player -%.0f"), Amount), 0.6f);
	}

	PlayPlayerHitReact();

	// HUD 업데이트 (사실 TakeDamage에서 Broadcast로 갱신됨)
	UpdateHUDSnapshot();
}



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

// 패링 성공시 호출
void ABattleManager::OnParryToast()
{
	/*if (!HUD) return;

	HUD->SetAP(PlayerRef ? PlayerRef->Stats.AP : 0.f);
	HUD->PlayParrySuccessEffect();

	if (TurnMenu) { TurnMenu->UpdateUltReady(IsUltReady(PlayerRef)); }*/
	return;
}

void ABattleManager::OnBossPatternPerfect()
{
	/*if (!HUD) return;

	if (PlayerRef) { PlayerRef->AddUltGauge(10.f); }
	HUD->PlayParryPerfectEffect();

	UpdateHUDSnapshot();*/
}

void ABattleManager::PlayPlayerHitReact()
{
	if (ATBPlayerCharacter* TB = Cast<ATBPlayerCharacter>(PlayerRef))
	{
		TB->PlayHitReact();
	}
}

void ABattleManager::PlayBossHitReact()
{
	if (BossRef) BossRef->PlayHitReact();
}


void ABattleManager::SpawnFloatingText(AActor* Target, float Amount)
{
	if (HUD) HUD->ShowDamagePopup(Target, Amount);
}
void ABattleManager::SpawnUltPopup(AActor* Target, float Amount)
{
	if (HUD) HUD->ShowUltGainPopup_HUD(Amount);
}

void ABattleManager::SpawnAPPopup(float Amount)
{
	if (HUD) HUD->ShowAPGainPopup(Amount);
}