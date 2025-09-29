#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ParryComponent.generated.h"

class ABoss_Sevarog;
class APlayerCharacter;
class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnParrySuccess);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class CATHERALBATTLE_API UParryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UParryComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry") float RecoveryTime = 0.6f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry") float BufferBeforeOpen = 0.10f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry|Anim") UAnimMontage* ParryMontage = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Parry") void OnParryPressed();
	UPROPERTY(BlueprintAssignable, Category = "Parry") FOnParrySuccess OnParrySuccess;
	UFUNCTION() void OnBossArm(APlayerCharacter* Target, int32 HitIndex, float WindowSec);

protected:
	virtual void BeginPlay() override;

private:
	// 상태
	bool   bArmed = false;
	bool   bLocked = false;
	int32  ArmedHitIndex = -1;
	double WindowEndTime = 0.0;
	double LastPressTime = -100000.0;

	TWeakObjectPtr<ABoss_Sevarog> ArmedBoss;

	FTimerHandle TimerExpire;
	FTimerHandle TimerRecovery;
	FTimerHandle TimerRebind;

	// 내부
	void TryBindBoss();
	void ExpireArm();
	void BeginRecovery();
	void EndRecovery();
	void HandleParrySuccess();
	void PlayParryMontage() const;
	APlayerCharacter* GetOwnerPlayer() const;
};
