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

	// 스폰할 몬스터 블루프린트 클래스 지정
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<AMonster> MonsterClass;

	// 스폰 중심 위치
	UPROPERTY(EditAnywhere, Category = "Spawn")
	FVector SpawnCenter;

	// 스폰 반경
	UPROPERTY(EditAnywhere, Category = "Spawn")
	float SpawnRadius;

	// 최대 몬스터 수
	UPROPERTY(EditAnywhere, Category = "Spawn")
	int32 MaxMonsterCount;

	UFUNCTION()
	void OnMonsterDeath(AMonster* DeadMonster);

protected:
	virtual void BeginPlay() override;

private:
	TArray<AMonster*> SpawnedMonsters;

	// 특정 위치에 몬스터 생성
	void SpawnMonsterAt(const FVector& Location);

	// 랜덤 스폰 위치 반환 (SpawnCenter 기준 반경 내)
	FVector GetRandomSpawnLocation() const;

	// 몬스터 사망 시 호출


public:

	// 초기 몬스터 다수 생성 함수
	void SpawnInitialMonsters();
};
