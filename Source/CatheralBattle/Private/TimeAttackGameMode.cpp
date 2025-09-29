// Fill out your copyright notice in the Description page of Project Settings.


#include "TimeAttackGameMode.h"

// Fill out your copyright notice in the Description page of Project Settings.

#include "TimeAttackGameMode.h"
#include "Kismet/GameplayStatics.h"

ATimeAttackGameMode::ATimeAttackGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATimeAttackGameMode::BeginPlay()
{
	Super::BeginPlay();
	TimeRemaining = InitialTime; // 시작할 때 남은 시간을 초기값으로 세팅 오류방지~ 
}

void ATimeAttackGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (TimeRemaining != -1.f)
	{
		TimeRemaining -= DeltaSeconds;

		if (TimeRemaining <= 0.f)
		{
			TimeRemaining = -1.f;
			UE_LOG(LogTemp, Warning, TEXT("The time has come"));
			// 때가 되었다 .. 라는 의미의 문구 썼어요
		}
	}
}
