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
	
	UPROPERTY(EditAnywhere, Category = "Parry")
	float ParryCooldown = 0.7f;          // 패링 재입력 금지 시간

	bool bParryOnCooldown = false;       // 쿨다운 플래그 (이미 있으면 중복 선언 금지)
	FTimerHandle TimerParryCooldown;     // 쿨다운 타이머

	void ResetParryCooldown();           // 쿨다운 해제


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
