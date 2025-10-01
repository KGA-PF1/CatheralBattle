// TBPlayerCharacter.h
#pragma once
#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ATBPlayerCharacter.generated.h"

// ★ EPlayerCommand 전방선언(실제 정의 헤더는 cpp에서 include)
enum class EPlayerCommand : uint8;

class UAnimMontage;
class UParryComponent;

UCLASS()
class ATBPlayerCharacter : public APlayerCharacter
{
	GENERATED_BODY()
public:
	ATBPlayerCharacter();

	// 선택지 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TB|Montage")
	UAnimMontage* M_Attack = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TB|Montage")
	UAnimMontage* M_Skill1 = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TB|Montage")
	UAnimMontage* M_Skill2 = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TB|Montage")
	UAnimMontage* M_Ultimate = nullptr;

	// 피격/패링 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TB|Montage")
	UAnimMontage* M_HitReact = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TB|Montage")
	UAnimMontage* M_Parry = nullptr;


	UFUNCTION(BlueprintCallable) void ActivateTurnCamera(bool bActive);

	// 입력 봉인
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;

	// 선택지 매핑
	UAnimMontage* GetMontageByCommand(EPlayerCommand Cmd) const;

	// BM에서 필요 시 호출
	void PlayHitReact() const;
	void EnsureParryMontageInjected();



private:
	UPROPERTY() TWeakObjectPtr<UParryComponent> CachedParry;
	void CacheParry();
	void PlayMontage(UAnimMontage* M) const;
};
