// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TimeAttackGameMode.generated.h"

UCLASS()
class CATHERALBATTLE_API ATimeAttackGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATimeAttackGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	// ���� �ð� (�⺻�� ��)
	UPROPERTY(EditAnywhere, Category = "Timer")
	float InitialTime = 60.f;

	// ���� �ð�
	float TimeRemaining;

public:
	// ���� �ð� �������� (UI���� ���� ����)
	UFUNCTION(BlueprintCallable, Category = "Timer")
	float GetTimeRemaining() const { return TimeRemaining; }
};
