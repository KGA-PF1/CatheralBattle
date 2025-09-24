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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHpChanged, float, NewHp, float, MaxHp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUltGaugeChanged, float, NewGauge, float, MaxGauge);

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

//스킬 입력
UENUM(BlueprintType)
enum class ESkillInput : uint8
{
	Attack UMETA(DisplayName = "Attack"),
	Skill_Q UMETA(DisplayName = "Skill_Q"),
	Skill_E UMETA(DisplayName = "Skill_E"),
	Ult_R UMETA(DisplayName = "Ult_R")
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

	//Hp - OnDeath?필요
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnHpChanged OnHpChanged;
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnUltGaugeChanged OnUltGaugeChanged;

	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetHp() { return Stats.Hp; }
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetMaxHp() { return Stats.MaxHp; }
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetHpPercent() { return Stats.MaxHp > 0 ? Stats.Hp / Stats.MaxHp : 0.f; }
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
	UBoxComponent* WeaponHitBox;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Weapon")
	FName WeaponSocketName = TEXT("hand_r_socket"); //TODO: 소켓 이름 알아오기

	//AnimNotify에서 쓸 On, Off
	UFUNCTION(BlueprintCallable, Category = "Combat|Notify")
	void AN_WeaponHitbox_On();
	UFUNCTION(BlueprintCallable, Category = "Combat|Notify")
	void AN_WeaponHitbox_Off();

protected:
	//입력 -> 스킬스펙 조회 -> (쿨/AP 체크) -> 몽타주 재생
	bool InternalUseSkill(const FSkillSpec& Spec, ESkillInput InputKind);

	//간단 쿨타임 타이머
	TMap<ESkillInput, float> CooldownTimers;

	//쿨다운 감소
	void UpdateCooldowns(float DeltaSeconds);

	//몽타주 재생 유틸
	void PlaySkillMontage(const FSkillSpec& Spec);

	//Overlap용
	UFUNCTION()
	void OnWeaponBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& Sweep);

	//스킬 Q 입력막기
private:
	FTimerHandle MovementLockStartHandle;
	UPROPERTY() UAnimMontage* ActiveSkillMontage = nullptr;
	void LockMoveInputDelayed();

	bool bMoveInputLocked = false;

	UFUNCTION()
	void OnMontageBlendOut(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void LockMoveInput();
	void UnLockMoveInput();
};
