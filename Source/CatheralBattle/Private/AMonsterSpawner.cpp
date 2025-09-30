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
	// �ڿ��� ������ ��ȸ(�ε��� ������ ����)
	for (int32 i = SpawnedMonsters.Num() - 1; i >= 0; --i)
	{
		AMonster* M = SpawnedMonsters[i];
		if (IsValid(M))
		{
			// ��� ��������Ʈ ����ε�(����ġ ���� ��ȣ�� ����)
			M->OnMonsterDeath.RemoveDynamic(this, &AAMonsterSpawner::OnMonsterDeath);

			// ���� �ı�
			M->Destroy();
		}
	}

	// ���� ����Ʈ ����
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
		// AI ��Ʈ�ѷ� �� �߰� �ʱ�ȭ
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
			SpawnMonsterAt(RespawnLoc); // ���� ���ȯ
		}
	}
}