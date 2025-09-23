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
    TSubclassOf<AMonster> MonsterClass;  // ������ ���� Ŭ����

    UPROPERTY(EditAnywhere, Category = "Spawn")
    FVector SpawnCenter;  // ���� �߽� ��ǥ

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float SpawnRadius = 1000.f;  // ���� ���� �ݰ�

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float MinDistanceFromPlayer = 1000.f;  // �÷��̾�κ��� �ּ� �Ÿ�

    UPROPERTY(EditAnywhere, Category = "Spawn")
    int32 MaxMonsterCount = 6;  // �ִ� ���� ��

private:
    TArray<AMonster*> SpawnedMonsters;  // ���� ������ ���� �迭

    void SpawnMonsterAt(const FVector& Location);

    FVector GetRandomPointInRadius();

    FVector FindSpawnLocationAwayFromPlayer();

    UFUNCTION()
    void OnMonsterKilled(AMonster* DeadMonster);

public:
    void SpawnInitialMonsters();
};
