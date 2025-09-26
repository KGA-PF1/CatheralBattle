// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;
class UBoxComponent;
struct FInputActionValue;
class USphereComponent;
class AMonster;

//스킬 입력
UENUM(BlueprintType)
enum class ESkillInput : uint8
{
	None,
	Attack UMETA(DisplayName = "Attack"),
	Skill_Q UMETA(DisplayName = "Skill_Q"),
	Skill_E UMETA(DisplayName = "Skill_E"),
	Ult_R UMETA(DisplayName = "Ult_R")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHpChanged, float, NewHp, float, MaxHp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUltGaugeChanged, float, NewGauge, float, MaxGauge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCooldownUpdated, ESkillInput, Input, float, Remaining, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCooldownEnded, ESkillInput, input);

USTRUCT(BlueprintType)
struct FPlayerStats //플레이어 스탯 구조체
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Hp = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHp = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AtkPoint = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Speed = 400.f;

	//TODO: 전투 관련(궁극기 게이지, AP) 구현
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float UltGauge = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxUltGauge = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0", ClampMax = "6"))
	float AP = 0.f;
};


USTRUCT(BlueprintType)
struct FSkillSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Skill")
	UAnimMontage* Montage = nullptr;

	//섹션 분기 필요하면 주석 해제
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	//FName MontageSection;

	//전투 때 쓰일 값
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Skill")
	float DamageMultiplier = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Skill")
	float CooldownSec = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Skill")
	int32 APCost = 0;

	//무기 히트박스 사용할 때 (AnimNotifyState에서 On, Off)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|HitBox")
	bool bUseWeaponHitBox = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|HitBox", meta = (EditCondition = "bUseWeaponHitBox"))
	FVector BoxExtent = FVector(10, 10, 80);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|HitBox", meta = (EditCondition = "bUseWeaponHitBox"))
	FVector BoxRelLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|HitBox", meta = (EditCondition = "bUseWeaponHitBox"))
	FRotator BoxRelRotation = FRotator::ZeroRotator;

	//트레이스 방식(광역 등) 사용하려면 주석 해제
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace")
	//bool bUseTrace = false;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace", meta = (EditCondition = "bUseTrace"))
	//float TraceRadius = 120.f;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace", meta = (EditCondition = "bUseTrace"))
	//float TraceRange = 300.f;
};

//TODO: Weapon 소켓에 콜리전 박스 할당

UCLASS()
class CATHERALBATTLE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

#pragma region Camera and Input
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SkillQAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SkillEAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* UltimateAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShieldAction;
#pragma endregion
protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);


public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FPlayerStats Stats;

	//Sprint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Sprint")
	float SprintMultiplier = 1.5f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|Sprint")
	bool bIsSprinting = false;

	UFUNCTION(BlueprintCallable)
	void StartSprint();
	UFUNCTION(BlueprintCallable)
	void StopSprint();
	UFUNCTION(BlueprintCallable)
	void SyncMovementSpeed();




	//Event 변수들
#pragma region Event
	//Hp - OnDeath?필요
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnHpChanged OnHpChanged;
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnUltGaugeChanged OnUltGaugeChanged;
	UPROPERTY(BlueprintAssignable, Category = "Event|Cooldown")
	FOnCooldownUpdated OnCooldownUpdated;
	UPROPERTY(BlueprintAssignable, Category = "Event|Cooldown")
	FOnCooldownEnded OnCooldownEnded;
#pragma endregion

	//UI를 위한 Getter
#pragma region UI Binding Function
	//HP
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetHp() { return Stats.Hp; }
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetMaxHp() { return Stats.MaxHp; }
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetHpPercent() { return Stats.MaxHp > 0 ? Stats.Hp / Stats.MaxHp : 0.f; }

	//궁극기
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetUltGauge() { return Stats.UltGauge; }
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetMaxUltGauge() { return Stats.MaxUltGauge; }
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetUltGaugePercent() { return Stats.MaxUltGauge > 0 ? Stats.UltGauge / Stats.MaxUltGauge : 0.f; }

	//쿨타임
	UFUNCTION(BlueprintPure, Category = "Skill|Cooldown")
	float GetCooldownRemaining(ESkillInput Input) const;
	UFUNCTION(BlueprintPure, Category = "Skill|Cooldown")
	float GetCooldownDuration(ESkillInput Input) const;
	UFUNCTION(BlueprintPure, Category = "Skill|Cooldown")
	float GetCooldownPercent(ESkillInput Input) const;
#pragma endregion
public:
	UFUNCTION(BlueprintCallable, Category = "Stats")
	bool IsDead() const { return Stats.Hp <= 0; }
	//TODO: ApplyDamage 필요하면 변경
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void TakeDamage(float Damage);
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void Respawn();
	//UltGauge
	//TODO: 몬스터 죽을 때 AddUltGauge하게끔
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddUltGauge(float Amount);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Table")
	TMap<ESkillInput, FSkillSpec> SkillTable;



	//스킬 관련 코드들
#pragma region Skill Code
	//TODO: UltGauge가 MaxUltGauge이여야 궁극기 하도록
	//TODO: UltGauge 0으로 만들기
public:
	UPROPERTY(BlueprintReadWrite, Category = "State")
	bool bCanAttack = true;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool TryUseSkill(ESkillInput InputKind);
	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool TryUseAttack() { return TryUseSkill(ESkillInput::Attack); }
	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool TryUseSkillQ() { return TryUseSkill(ESkillInput::Skill_Q); }
	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool TryUseSkillE() { return TryUseSkill(ESkillInput::Skill_E); }
	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool TryUseUlt() { return TryUseSkill(ESkillInput::Ult_R); }

	//Input 바인딩
	UFUNCTION() void Input_Attack();
	UFUNCTION() void Input_SkillQ();
	UFUNCTION() void Input_SkillE();
	UFUNCTION() void Input_Ult();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bTurnBased = false;

	//무기 히트박스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Weapon")
	UBoxComponent* Weapon;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Weapon")
	FName WeaponSocketName = TEXT("sword_top"); //TODO: 소켓 이름 알아오기

	//방패 히트박스
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Shield")
	//UBoxComponent* Shield;
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Shield")
	//FName ShieldSocketName = TEXT("shield_inner");


	//AnimNotify에서 쓸 On, Off
	UFUNCTION(BlueprintCallable, Category = "Combat|Notify")
	void AN_Sword_On();
	UFUNCTION(BlueprintCallable, Category = "Combat|Notify")
	void AN_Sword_Off();

protected:
	//입력 -> 스킬스펙 조회 -> (쿨/AP 체크) -> 몽타주 재생
	bool InternalUseSkill(const FSkillSpec& Spec, ESkillInput InputKind);

	//쿨다운 감소
	void UpdateCooldowns(float DeltaSeconds);

	//몽타주 재생 유틸
	void PlaySkillMontage(const FSkillSpec& Spec);

	//쿨타임
	TMap<ESkillInput, float> CooldownTimers;
	void BroadcastCooldown(ESkillInput Input, float Remaining, float Duration);

private: //스킬 Q 입력막기
	FTimerHandle MovementLockStartHandle;
	UPROPERTY() UAnimMontage* ActiveSkillMontage = nullptr;
	bool bMoveInputLocked = false;

	ESkillInput CurrentSkill = ESkillInput::None;

	UFUNCTION()
	void OnMontageBlendOut(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void LockMoveInput();
	void UnLockMoveInput();
#pragma endregion


	//오버랩 관련 코드들
#pragma region Overlap
private:
	//AN_Sword_On ~ Off동안 중복 타격 방지
	TSet<TWeakObjectPtr<AActor>> HitActorsThisSwing;

public:
	// 공격 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float QSkillRange = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float ESkillRange = 600.f;
	

	// 공격 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float SkillDamage = 0.f;

	// 디버그 표시 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugSphere = true;


	float CalcAttackDamage() const;
	float CalcAttackRange() const;

	//Attack, SkillQ, SkillE 오버랩
	UFUNCTION()
	void OnWeaponBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& Sweep);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ExecuteAOEAttack();


#pragma endregion
};
