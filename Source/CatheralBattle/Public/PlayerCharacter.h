// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
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

UCLASS()
class CATHERALBATTLE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	/** Camera boom positioning the camera behind the character */
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Sprint")
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
	UPROPERTY(BlueprintAssignable, Category="Event")
	FOnHpChanged OnHpChanged;
	UPROPERTY(BlueprintAssignable, Category="Event")
	FOnUltGaugeChanged OnUltGaugeChanged;

	UFUNCTION(BlueprintCallable, Category="Stats")
	float GetHp() { return Stats.Hp; }
	UFUNCTION(BlueprintCallable, Category="Stats")
	float GetMaxHp() { return Stats.MaxHp; }
	UFUNCTION(BlueprintCallable, Category="Stats")
	float GetHpPercent() { return Stats.MaxHp > 0 ? Stats.Hp / Stats.MaxHp : 0.f; }
	UFUNCTION(BlueprintCallable, Category="Stats")
	bool IsDead() const { return Stats.Hp <= 0; }
	//TODO: ApplyDamage 필요하면 변경
	UFUNCTION(BlueprintCallable, Category="Stats")
	void TakeDamage(float Damage);
	UFUNCTION(BlueprintCallable, Category="Stats")
	void Respawn();
	
	//UltGauge
	//TODO: 몬스터 죽을 때 AddUltGauge하게끔
	UFUNCTION(BlueprintCallable, Category="Stats")
	void AddUltGauge(float Amount);


	//Combat 관련
	//몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Montage")
	UAnimMontage* Montage_BaseAttack;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Montage")
	UAnimMontage* Montage_Skill1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Montage")
	UAnimMontage* Montage_Skill2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Montage")
	UAnimMontage* Montage_Ult;

	//스킬 호출
	//UFUNCTION(BlueprintCallable, Category="Combat|Skill")
	//bool TryUseBaseAttack(AActor* Target);

};
