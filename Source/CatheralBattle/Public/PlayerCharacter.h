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

//��ų �Է�
UENUM(BlueprintType)
enum class ESkillInput : uint8
{
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
struct FPlayerStats //�÷��̾� ���� ����ü
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

	//TODO: ���� ����(�ñر� ������, AP) ����
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

	//���� �б� �ʿ��ϸ� �ּ� ����
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	//FName MontageSection;

	//���� �� ���� ��
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Skill")
	float DamageMultiplier = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Skill")
	float CooldownSec = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Skill")
	int32 APCost = 0;

	//���� ��Ʈ�ڽ� ����� �� (AnimNotifyState���� On, Off)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|HitBox")
	bool bUseWeaponHitBox = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|HitBox", meta = (EditCondition = "bUseWeaponHitBox"))
	FVector BoxExtent = FVector(10, 10, 80);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|HitBox", meta = (EditCondition = "bUseWeaponHitBox"))
	FVector BoxRelLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|HitBox", meta = (EditCondition = "bUseWeaponHitBox"))
	FRotator BoxRelRotation = FRotator::ZeroRotator;

	//Ʈ���̽� ���(���� ��) ����Ϸ��� �ּ� ����
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace")
	//bool bUseTrace = false;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace", meta = (EditCondition = "bUseTrace"))
	//float TraceRadius = 120.f;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace", meta = (EditCondition = "bUseTrace"))
	//float TraceRange = 300.f;
};

//TODO: Weapon ���Ͽ� �ݸ��� �ڽ� �Ҵ�

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


	//Event ������
#pragma region Event
	//Hp - OnDeath?�ʿ�
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnHpChanged OnHpChanged;
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnUltGaugeChanged OnUltGaugeChanged;
	UPROPERTY(BlueprintAssignable, Category = "Event|Cooldown")
	FOnCooldownUpdated OnCooldownUpdated;
	UPROPERTY(BlueprintAssignable, Category = "Event|Cooldown")
	FOnCooldownEnded OnCooldownEnded;
#pragma endregion

	//UI�� ���� Getter
#pragma region UI Binding Function
	//HP
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetHp() { return Stats.Hp; }
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetMaxHp() { return Stats.MaxHp; }
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetHpPercent() { return Stats.MaxHp > 0 ? Stats.Hp / Stats.MaxHp : 0.f; }
	
	//�ñر�
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetUltGauge() { return Stats.UltGauge; }
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetMaxUltGauge() { return Stats.MaxUltGauge; }
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetUltGaugePercent() { return Stats.MaxUltGauge > 0 ? Stats.UltGauge / Stats.MaxUltGauge : 0.f; }

	//��Ÿ��
	UFUNCTION(BlueprintPure, Category="Skill|Cooldown")
	float GetCooldownRemaining(ESkillInput Input) const;
	UFUNCTION(BlueprintPure, Category="Skill|Cooldown")
	float GetCooldownDuration(ESkillInput Input) const;
	UFUNCTION(BlueprintPure, Category="Skill|Cooldown")
	float GetCooldownPercent(ESkillInput Input) const;
#pragma endregion
	
	UFUNCTION(BlueprintCallable, Category = "Stats")
	bool IsDead() const { return Stats.Hp <= 0; }
	//TODO: ApplyDamage �ʿ��ϸ� ����
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void TakeDamage(float Damage);
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void Respawn();
	//UltGauge
	//TODO: ���� ���� �� AddUltGauge�ϰԲ�
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddUltGauge(float Amount);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Table")
	TMap<ESkillInput, FSkillSpec> SkillTable;


	//TODO: UltGauge�� MaxUltGauge�̿��� �ñر� �ϵ���
	//TODO: UltGauge 0���� �����
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

	//Input ���ε�
	UFUNCTION() void Input_Attack();
	UFUNCTION() void Input_SkillQ();
	UFUNCTION() void Input_SkillE();
	UFUNCTION() void Input_Ult();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bTurnBased = false;

	//���� ��Ʈ�ڽ�
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Weapon")
	UBoxComponent* Sword;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Weapon")
	FName WeaponSocketName = TEXT("sword_top"); //TODO: ���� �̸� �˾ƿ���

	//AnimNotify���� �� On, Off
	UFUNCTION(BlueprintCallable, Category = "Combat|Notify")
	void AN_Sword_On();
	UFUNCTION(BlueprintCallable, Category = "Combat|Notify")
	void AN_Sword_Off();

protected:
	//�Է� -> ��ų���� ��ȸ -> (��/AP üũ) -> ��Ÿ�� ���
	bool InternalUseSkill(const FSkillSpec& Spec, ESkillInput InputKind);

	//��ٿ� ����
	void UpdateCooldowns(float DeltaSeconds);

	//��Ÿ�� ��� ��ƿ
	void PlaySkillMontage(const FSkillSpec& Spec);

	//Overlap��
	UFUNCTION()
	void OnWeaponBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& Sweep);

	//��Ÿ��
#pragma region
	TMap<ESkillInput, float> CooldownTimers;
	void BroadcastCooldown(ESkillInput Input, float Remaining, float Duration);
#pragma endregion


	
private: //��ų Q �Է¸���
	FTimerHandle MovementLockStartHandle;
	UPROPERTY() UAnimMontage* ActiveSkillMontage = nullptr;
	bool bMoveInputLocked = false;

	UFUNCTION()
	void OnMontageBlendOut(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void LockMoveInput();
	void UnLockMoveInput();
};
