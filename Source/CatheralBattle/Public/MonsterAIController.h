// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MonsterAIController.generated.h"


UCLASS()
class CATHERALBATTLE_API AMonsterAIController : public AAIController
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override; // 몬스터가 소유될 때 호출

	UPROPERTY(EditAnywhere, Category = "AI")
	float AcceptanceRadius = 100.f;

	UPROPERTY(EditAnywhere, Category = "AI")
	class UBehaviorTree* AIBehavior; // 몬스터의 행동 트리

};
