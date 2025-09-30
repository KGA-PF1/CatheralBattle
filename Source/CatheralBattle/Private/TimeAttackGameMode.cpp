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

	// ���� ī��Ʈ�ٿ��� ���� ����
	TimeRemaining = InitialTime;

	// 10�� �ڿ� Ÿ�Ӿ��� ����
	GetWorldTimerManager().SetTimer(StartDelayHandle, this, &ATimeAttackGameMode::StartTimer, 10.0f, false);
}

void ATimeAttackGameMode::StartTimer()
{
	bIsTimerActive = true; // ���� ī��Ʈ�ٿ� ����
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
			bIsTimerActive = false; // ����� ������

			TArray<AActor*> Found;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAMonsterSpawner::StaticClass(), Found);

			for (AActor* Actor : Found)
			{
				if (AAMonsterSpawner* Spawner = Cast<AAMonsterSpawner>(Actor))
				{
					Spawner->SetSpawningEnabled(false);  // ���ȯ ���� (bAllowRespawn = false)
					Spawner->SetMaxMonsterCount(0);      // �ʱ� ������ 0���� ����
					Spawner->CullAllMonsters();          // �̹� ����ִ� ���� ���� (���ϸ� ���� ����)
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("The time has come"));
		}
	}
}
