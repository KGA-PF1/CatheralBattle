

// Fill out your copyright notice in the Description page of Project Settings.


#include "AMonsterSpawner.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Components/CapsuleComponent.h"
#include "Monster.h"

// Sets default values
AAMonsterSpawner::AAMonsterSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAMonsterSpawner::BeginPlay()
{
	Super::BeginPlay();
	SpawnInitialMonsters();
	
}

void AAMonsterSpawner::SpawnMonsterAt(const FVector& Location)
{

	if (!MonsterClass) return;

	FActorSpawnParameters SpawnParams;
	AMonster* NewMonster = GetWorld()->SpawnActor<AMonster>(MonsterClass, Location, FRotator::ZeroRotator, SpawnParams);
	if (NewMonster)
	{
		SpawnedMonsters.Add(NewMonster);
        NewMonster->OnMonsterDeath.AddDynamic(this, &AAMonsterSpawner::OnMonsterKilled);
		
	}

}

FVector AAMonsterSpawner::GetRandomPointInRadius()
{
	FVector RandomOffset = FVector(
		FMath::RandRange(-SpawnRadius, SpawnRadius),
		FMath::RandRange(-SpawnRadius, SpawnRadius),
		0.f);

	return SpawnCenter + RandomOffset;
}

FVector AAMonsterSpawner::FindSpawnLocationAwayFromPlayer()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	FVector SpawnLoc;
	do
	{
		SpawnLoc = GetRandomPointInRadius();
	} while (PlayerPawn && FVector::Dist(PlayerPawn->GetActorLocation(), SpawnLoc) < MinDistanceFromPlayer);

	return SpawnLoc;
}

void AAMonsterSpawner::OnMonsterKilled(AMonster* DeadMonster)
{

	SpawnedMonsters.Remove(DeadMonster);

	FVector NewSpawnLoc = FindSpawnLocationAwayFromPlayer();

	SpawnMonsterAt(NewSpawnLoc);

}

void AAMonsterSpawner::SpawnInitialMonsters()
{

	for (int i = 0; i < MaxMonsterCount; i++)
	{
		FVector SpawnLocation = GetRandomPointInRadius();
		SpawnMonsterAt(SpawnLocation);
	}

}

//// Called every frame
//void AAMonsterSpawner::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

