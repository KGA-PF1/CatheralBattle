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
	TimeRemaining = InitialTime;
}

void ATimeAttackGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (TimeRemaining > 0.f)
	{
		TimeRemaining -= DeltaSeconds;

		if (TimeRemaining <= 0.f)
		{
			TimeRemaining = 0.f;
			UE_LOG(LogTemp, Warning, TEXT("The time has come"));
		}
	}
}