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

		// AIController가 없으면 생성 후 몬스터 소유
		AAIController* AIController = Cast<AAIController>(NewMonster->GetController());
		if (!AIController)
		{
			AIController = GetWorld()->SpawnActor<AMonsterAIController>(AMonsterAIController::StaticClass());
			if (AIController)
				AIController->Possess(NewMonster);
		}

		// 몬스터 사망 이벤트 바인딩 대응하면 재생성 가능 (선택적)
		// NewMonster->OnDeath.AddDynamic(this, &AAMonsterSpawner::OnMonsterDeath);
	}
}

void AAMonsterSpawner::OnMonsterDeath(AMonster* DeadMonster)
{
	if (SpawnedMonsters.Contains(DeadMonster))
	{
		SpawnedMonsters.Remove(DeadMonster);

		FVector RespawnLoc = GetRandomSpawnLocation();
		SpawnMonsterAt(RespawnLoc);
	}
}
