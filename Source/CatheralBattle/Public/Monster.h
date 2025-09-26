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
    virtual float TakeDamage(float DamageAmount,
        struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator,
        AActor* DamageCauser) override;

public:
    AMonster();
public:
        // ==== 스탯 ====
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float MaxHP = 25.f; //최대체력

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float CurrentHP = 25.f; //현제 채력

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float AttackPoint = 5.f;//공격력

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float UltGaugeReward = 5.f;//궁국기 포인트

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float Speed = 300.f;//속도

        UFUNCTION(BlueprintCallable, Category = "Stats")
        float GetHp() { return CurrentHP; }
        UFUNCTION(BlueprintCallable, Category = "Stats")
        float GetMaxHp() { return MaxHP; }
        UFUNCTION(BlueprintCallable, Category = "Stats")
        float GetHpPercent() { return MaxHP > 0 ? CurrentHP / MaxHP : 0.f; }

        // 공격 범위 및 공격력 변수 추가
        UPROPERTY(EditAnywhere, Category = "Combat")
		float AttackRange = 150.f;// 공격 범위 변수

        // 공격 쿨타임 변수 (초 단위)
        UPROPERTY(EditAnywhere, Category = "Combat")
		float AttackCooldown = 5.f;// 공격 쿨타임 변수

        bool bCanAttack; // 공격 가능 여부 변수

        void ResetAttackCooldown();// 공격 쿨타임 리셋 함수

        void PerformAttack(); // 공격 함수

        void OnDeath(); // 몬스터가 죽었을 때 처리 함수

        void DestroyMonster(); // 몬스터 제거 함수
        
        void OnSpawnAnimationFinished();

        FTimerHandle AttackTimerHandle;
        FTimerHandle DeathTimerHandle;

        // ==== 이벤트 ====
        // 몬스터가 죽을 때 알림
        UPROPERTY(BlueprintAssignable, Category = "Monster")
        FOnMonsterDeath OnMonsterDeath; // 몬스터가 죽을 때 알림 이벤트
		// 몬스터의 HP가 변경될 때 알림 (위젯에 브로드캐스트)
        UPROPERTY(BlueprintAssignable, Category = "Monster")
        FOnHpMonster OnHpMonster;

    protected:	
        void PlayAttackAnimation();
};