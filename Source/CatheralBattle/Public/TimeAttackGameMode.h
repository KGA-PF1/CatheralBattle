// TimeAttackGameMode.h

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
	// 시작 시간 (기본값 초)
	UPROPERTY(EditAnywhere, Category = "Timer")
	float InitialTime = 60.f;

	// 남은 시간
	float TimeRemaining;

	// 딜레이 후 카운트다운 시작
	void StartTimer();
	FTimerHandle StartDelayHandle;

	// 실행 여부
	bool bIsTimerActive = false;

public:
	// 남은 시간 가져오기 (UI에서 접근 가능)
	UFUNCTION(BlueprintCallable, Category = "Timer")
	float GetTimeRemaining() const { return TimeRemaining; }
};
