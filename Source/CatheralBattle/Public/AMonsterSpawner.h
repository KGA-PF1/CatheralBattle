// Fill out your copyright notice in the Description page of Project Settings.

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
	// Sets default values for this actor's properties
	AAMonsterSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TSubclassOf<AMonster> MonsterClass;  // 스폰할 몬스터 클래스

    UPROPERTY(EditAnywhere, Category = "Spawn")
    FVector SpawnCenter;  // 스폰 중심 좌표

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float SpawnRadius = 1000.f;  // 스폰 범위 반경

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float MinDistanceFromPlayer = 1000.f;  // 플레이어로부터 최소 거리

    UPROPERTY(EditAnywhere, Category = "Spawn")
    int32 MaxMonsterCount = 6;  // 최대 몬스터 수

private:
    TArray<AMonster*> SpawnedMonsters;  // 현재 스폰된 몬스터 배열

	void SpawnMonsterAt(const FVector& Location); // 특정 위치에 몬스터 스폰

	FVector GetRandomPointInRadius(); // 반경 내 랜덤 위치 반환

	FVector FindSpawnLocationAwayFromPlayer(); // 플레이어로부터 일정 거리 떨어진 위치 찾기

    UFUNCTION()
	void OnMonsterKilled(AMonster* DeadMonster); // 몬스터 사망시 호출되는 함수

public:
	void SpawnInitialMonsters(); // 초기 몬스터 스폰
};
