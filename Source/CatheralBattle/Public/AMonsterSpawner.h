#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Monster.h"
#include "AMonsterSpawner.generated.h"

UCLASS()
class CATHERALBATTLE_API AAMonsterSpawner : public AActor
{
	GENERATED_BODY()

public:

	AAMonsterSpawner();

	// ������ ���� �������Ʈ Ŭ���� ����
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<AMonster> MonsterClass;

	// ���� �߽� ��ġ
	UPROPERTY(EditAnywhere, Category = "Spawn")
	FVector SpawnCenter;

	// ���� �ݰ�
	UPROPERTY(EditAnywhere, Category = "Spawn")
	float SpawnRadius;

	// �ִ� ���� ��
	UPROPERTY(EditAnywhere, Category = "Spawn")
	int32 MaxMonsterCount;

protected:
	virtual void BeginPlay() override;

private:
	TArray<AMonster*> SpawnedMonsters;

	// Ư�� ��ġ�� ���� ����
	void SpawnMonsterAt(const FVector& Location);

	// ���� ���� ��ġ ��ȯ (SpawnCenter ���� �ݰ� ��)
	FVector GetRandomSpawnLocation() const;

	// ���� ��� �� ȣ��
	void OnMonsterDeath(AMonster* DeadMonster);

public:

	// �ʱ� ���� �ټ� ���� �Լ�
	void SpawnInitialMonsters();
};
