// TimeAttackGameMode.cpp
#include "TimeAttackGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "BattleManager.h"
#include "Boss_Sevarog.h"
#include "ParryInputProxy.h"
#include "PlayerCharacter.h"

ATimeAttackGameMode::ATimeAttackGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATimeAttackGameMode::BeginPlay()
{
	Super::BeginPlay();

	// ★ 실시간 UI 싹 제거(테스트 전투만 보기)
	//UWidgetBlueprintLibrary::RemoveAllWidgets(GetWorld());

	// ★ 플레이어 확보(없으면 DefaultPawnClass를 PlayerCharacter BP로!)
	APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Player) { UE_LOG(LogTemp, Error, TEXT("PlayerCharacter not possessed")); return; }

	// ★ 보스/프록시/BM 찾거나 스폰
	ABoss_Sevarog* Boss = nullptr;
	{
		TArray<AActor*> Found; UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABoss_Sevarog::StaticClass(), Found);
		Boss = Found.Num() > 0 ? Cast<ABoss_Sevarog>(Found[0]) : nullptr;
		if (!Boss)
		{
			const FVector Loc = Player->GetActorLocation() + Player->GetActorForwardVector() * 800.f;
			Boss = GetWorld()->SpawnActor<ABoss_Sevarog>(ABoss_Sevarog::StaticClass(), Loc, FRotator::ZeroRotator);
		}
	}
	AParryInputProxy* Proxy = nullptr;
	{
		TArray<AActor*> Found; UGameplayStatics::GetAllActorsOfClass(GetWorld(), AParryInputProxy::StaticClass(), Found);
		Proxy = Found.Num() > 0 ? Cast<AParryInputProxy>(Found[0]) : GetWorld()->SpawnActor<AParryInputProxy>();
	}
	ABattleManager* BM = nullptr;
	{
		TArray<AActor*> Found; UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABattleManager::StaticClass(), Found);
		BM = Found.Num() > 0 ? Cast<ABattleManager>(Found[0]) : GetWorld()->SpawnActor<ABattleManager>();
	}

	// ★ 초기화 + 즉시 시작
	if (BM && Boss && Player)
	{
		BM->Initialize(Player, Boss, Proxy);
		BM->StartBattle();
	}

	// 타임어택 타이머는 테스트에 영향 없지만, 완전 정지하려면:
	PrimaryActorTick.bCanEverTick = false;
}

void ATimeAttackGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// 테스트용: 필요없으면 비워두면 됨
}