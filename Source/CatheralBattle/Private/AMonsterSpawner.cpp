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

void AAMonsterSpawner::SetMaxMonsterCount(int32 Count)
{
	MaxMonsterCount = Count;
}

void AAMonsterSpawner::CullAllMonsters()
{
	// 뒤에서 앞으로 순회(인덱스 무너짐 방지)
	for (int32 i = SpawnedMonsters.Num() - 1; i >= 0; --i)
	{
		AMonster* M = SpawnedMonsters[i];
		if (IsValid(M))
		{
			// 사망 델리게이트 언바인드(예상치 못한 재호출 방지)
			M->OnMonsterDeath.RemoveDynamic(this, &AAMonsterSpawner::OnMonsterDeath);

			// 액터 파괴
			M->Destroy();
		}
	}

	// 내부 리스트 비우기
	SpawnedMonsters.Empty();
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
		if (bAllowRespawn)
		{
			FVector RespawnLoc = GetRandomSpawnLocation();
			SpawnMonsterAt(RespawnLoc); // 몬스터 재소환
		}
	}
}