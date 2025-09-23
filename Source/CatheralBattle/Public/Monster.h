// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Monster.generated.h"

//���� ����� ��ε�ĳ��Ʈ �̺�Ʈ (������/���Ӹ�忡�� ���)
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
        // ==== ���� ====
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

        // ���� ���� �� ���ݷ� ���� �߰�
        UPROPERTY(EditAnywhere, Category = "Combat")
        float AttackRange = 150.f;

        // ���� ��Ÿ�� ���� (�� ����)
        UPROPERTY(EditAnywhere, Category = "Combat")
        float AttackCooldown = 5.f;

        bool bCanAttack = true;

        // ���� �Լ� ����
      

        // ���� ��Ÿ�� ���� �Լ�
        void ResetAttackCooldown();
        FTimerHandle AttackTimerHandle;

        // ==== �̺�Ʈ ====
        // ���Ͱ� ���� �� �˸�
        UPROPERTY(BlueprintAssignable, Category = "Monster")
        FOnMonsterDeath OnMonsterDeath;
    
        // ==== �Լ� ====
        UFUNCTION(BlueprintCallable, Category = "Monster")
        void TakeDamageCustom(int32 DamageAmount);
      
    protected:
        void OnDeath();
        void DestroyMonster();
        UFUNCTION(BlueprintCallable, Category = "Monster")
        void PerformAttack();
        FTimerHandle DeathTimerHandle;
};