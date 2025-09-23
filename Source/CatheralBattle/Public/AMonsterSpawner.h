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

    void SpawnMonsterAt(const FVector& Location);

    FVector GetRandomPointInRadius();

    FVector FindSpawnLocationAwayFromPlayer();

    UFUNCTION()
    void OnMonsterKilled(AMonster* DeadMonster);

public:
    void SpawnInitialMonsters();
};
