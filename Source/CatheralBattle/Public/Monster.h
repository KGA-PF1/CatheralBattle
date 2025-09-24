// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Monster.generated.h"

//몬스터 사망시 브로드캐스트 이벤트 (스포너/게임모드에서 사용)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterDeath, class AMonster*, Monster);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHpMonster, float, CurrentHP, float, MaxHP);

UCLASS()

class CATHERALBATTLE_API AMonster : public ACharacter
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
public:
    AMonster();
public:
        // ==== 스탯 ====
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		int32 MaxHP = 25;// 기본 체력 25
    
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
		int32 CurrentHP;// 현재 체력
    
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		int32 AttackPoint = 5;// 공격력
    
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		int32 UltGaugeReward = 5;// 처치시 주는 얼티 게이지

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		int32 Speed = 300;// 이동 속도

        // 공격 범위 및 공격력 변수 추가
        UPROPERTY(EditAnywhere, Category = "Combat")
		float AttackRange = 150.f;// 공격 범위 변수

        // 공격 쿨타임 변수 (초 단위)
        UPROPERTY(EditAnywhere, Category = "Combat")
		float AttackCooldown = 5.f;// 공격 쿨타임 변수

		bool bCanAttack = true; // 공격 가능 여부 변수

        void OnSpawnAnimationFinished();

        // 공격 함수 선언
      
        // 공격 쿨타임 리셋 함수
		void ResetAttackCooldown();// 공격 쿨타임 리셋 함수
        FTimerHandle AttackTimerHandle;

        // ==== 이벤트 ====
        // 몬스터가 죽을 때 알림
        UPROPERTY(BlueprintAssignable, Category = "Monster")
		FOnMonsterDeath OnMonsterDeath; // 몬스터가 죽을 때 알림 이벤트
        
        UPROPERTY(BlueprintAssignable, Category = "Monster")
        FOnHpMonster OnHpMonster;

        // ==== 함수 ====
        UFUNCTION(BlueprintCallable, Category = "Monster")
		void TakeDamageCustom(int32 DamageAmount);// 커스텀 데미지 함수
      
        

    protected:
		void OnDeath(); // 몬스터가 죽었을 때 처리 함수
		void DestroyMonster(); // 몬스터 제거 함수
        
        UFUNCTION(BlueprintCallable, Category = "Monster")
		void PerformAttack();// 공격 함수
		FTimerHandle DeathTimerHandle; // 죽음 후 제거 타이머 핸들
};