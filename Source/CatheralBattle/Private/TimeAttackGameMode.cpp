// TimeAttackGameMode.cpp

#include "TimeAttackGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "AMonsterSpawner.h"


ATimeAttackGameMode::ATimeAttackGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATimeAttackGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 아직 카운트다운은 멈춘 상태
	TimeRemaining = InitialTime;

	// 10초 뒤에 타임어택 시작
	GetWorldTimerManager().SetTimer(StartDelayHandle, this, &ATimeAttackGameMode::StartTimer, 10.0f, false);
}

void ATimeAttackGameMode::StartTimer()
{
	bIsTimerActive = true; // 이제 카운트다운 시작
}

void ATimeAttackGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsTimerActive && TimeRemaining > 0.f)
	{
		TimeRemaining -= DeltaSeconds;

		if (TimeRemaining <= 0.f)
		{
			TimeRemaining = 0.f;
			bIsTimerActive = false; // 재실행 방지용

			TArray<AActor*> Found;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAMonsterSpawner::StaticClass(), Found);

			for (AActor* Actor : Found)
			{
				if (AAMonsterSpawner* Spawner = Cast<AAMonsterSpawner>(Actor))
				{
					Spawner->SetSpawningEnabled(false);  // 재소환 끄기 (bAllowRespawn = false)
					Spawner->SetMaxMonsterCount(0);      // 초기 스폰도 0으로 고정
					Spawner->CullAllMonsters();          // 이미 살아있는 몬스터 정리 (원하면 유지 가능)
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("The time has come"));
		}
	}
}
