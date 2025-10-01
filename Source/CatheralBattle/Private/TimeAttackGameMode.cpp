<<<<<<< HEAD
﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "TimeAttackGameMode.h"

// Fill out your copyright notice in the Description page of Project Settings.
=======
// TimeAttackGameMode.cpp

>>>>>>> origin/develop
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
<<<<<<< HEAD
	TimeRemaining = InitialTime;
=======

	// ���� ī��Ʈ�ٿ��� ���� ����
	TimeRemaining = InitialTime;

	// 10�� �ڿ� Ÿ�Ӿ��� ����
	GetWorldTimerManager().SetTimer(StartDelayHandle, this, &ATimeAttackGameMode::StartTimer, 10.0f, false);
}

void ATimeAttackGameMode::StartTimer()
{
	bIsTimerActive = true; // ���� ī��Ʈ�ٿ� ����
>>>>>>> origin/develop
}

void ATimeAttackGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
<<<<<<< HEAD
	if (TimeRemaining > 0.f)
=======

	if (bIsTimerActive && TimeRemaining > 0.f)
>>>>>>> origin/develop
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