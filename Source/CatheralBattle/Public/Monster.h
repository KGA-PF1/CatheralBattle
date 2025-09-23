// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Monster.generated.h"

//몬스터 사망시 브로드캐스트 이벤트 (스포너/게임모드에서 사용)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterDeath, class AMonster*, Monster);

UCLASS()

class CATHERALBATTLE_API AMonster : public ACharacter
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

public:
    AMonster();
public:
        // ==== 스탯 ====
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        int32 MaxHP = 25;
    
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
        int32 CurrentHP;
    
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        int32 AttackPoint = 5;
    
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        int32 UltGaugeReward = 5;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        int32 Speed = 300;

        // 공격 범위 및 공격력 변수 추가
        UPROPERTY(EditAnywhere, Category = "Combat")
        float AttackRange = 150.f;

        // 공격 쿨타임 변수 (초 단위)
        UPROPERTY(EditAnywhere, Category = "Combat")
        float AttackCooldown = 5.f;

        bool bCanAttack = true;

        // 공격 함수 선언
      

        // 공격 쿨타임 리셋 함수
        void ResetAttackCooldown();
        FTimerHandle AttackTimerHandle;

        // ==== 이벤트 ====
        // 몬스터가 죽을 때 알림
        UPROPERTY(BlueprintAssignable, Category = "Monster")
        FOnMonsterDeath OnMonsterDeath;
    
        // ==== 함수 ====
        UFUNCTION(BlueprintCallable, Category = "Monster")
        void TakeDamageCustom(int32 DamageAmount);
      
    protected:
        void OnDeath();
        void DestroyMonster();
        UFUNCTION(BlueprintCallable, Category = "Monster")
        void PerformAttack();
        FTimerHandle DeathTimerHandle;
};