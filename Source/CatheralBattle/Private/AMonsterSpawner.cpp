#include "AMonsterSpawner.h"
#include "Engine/World.h"
#include "AIController.h"
#include "MonsterAIController.h"

AAMonsterSpawner::AAMonsterSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	SpawnRadius = 1000.f;
	MaxMonsterCount = 6;
	SpawnCenter = FVector::ZeroVector;
}

void AAMonsterSpawner::BeginPlay()
{
	Super::BeginPlay();
	SpawnInitialMonsters();
}

void AAMonsterSpawner::SpawnInitialMonsters()
{
	for (int32 i = 0; i < MaxMonsterCount; i++)
	{
		FVector SpawnLocation = GetRandomSpawnLocation();
		SpawnMonsterAt(SpawnLocation);
	}
}

FVector AAMonsterSpawner::GetRandomSpawnLocation() const
{
	FVector Offset(FMath::RandRange(-SpawnRadius, SpawnRadius), FMath::RandRange(-SpawnRadius, SpawnRadius), 0.f);
	return SpawnCenter + Offset;
}

void AAMonsterSpawner::SpawnMonsterAt(const FVector& Location)
{
	if (!MonsterClass) return;

	FActorSpawnParameters SpawnParams;
	AMonster* NewMonster = GetWorld()->SpawnActor<AMonster>(MonsterClass, Location, FRotator::ZeroRotator, SpawnParams);

	if (NewMonster)
	{
		SpawnedMonsters.Add(NewMonster);
		NewMonster->OnMonsterDeath.AddDynamic(this, &AAMonsterSpawner::OnMonsterDeath);
		// AI 컨트롤러 등 추가 초기화
	}
}

void AAMonsterSpawner::OnMonsterDeath(AMonster* DeadMonster)
{
	if (SpawnedMonsters.Contains(DeadMonster))
	{
		SpawnedMonsters.Remove(DeadMonster);
		FVector RespawnLoc = GetRandomSpawnLocation();
		SpawnMonsterAt(RespawnLoc); // 몬스터 재소환
	}
}
