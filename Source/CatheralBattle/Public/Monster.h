// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Monster.generated.h"

//���� ����� ��ε�ĳ��Ʈ �̺�Ʈ (������/���Ӹ�忡�� ���)
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
        // ==== ���� ====
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		int32 MaxHP = 25;// �⺻ ü�� 25
    
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
		int32 CurrentHP;// ���� ü��
    
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		int32 AttackPoint = 5;// ���ݷ�
    
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		int32 UltGaugeReward = 5;// óġ�� �ִ� ��Ƽ ������

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		int32 Speed = 300;// �̵� �ӵ�

        // ���� ���� �� ���ݷ� ���� �߰�
        UPROPERTY(EditAnywhere, Category = "Combat")
		float AttackRange = 150.f;// ���� ���� ����

        // ���� ��Ÿ�� ���� (�� ����)
        UPROPERTY(EditAnywhere, Category = "Combat")
		float AttackCooldown = 5.f;// ���� ��Ÿ�� ����

		bool bCanAttack = true; // ���� ���� ���� ����

        void OnSpawnAnimationFinished();

        // ���� �Լ� ����
      
        // ���� ��Ÿ�� ���� �Լ�
		void ResetAttackCooldown();// ���� ��Ÿ�� ���� �Լ�
        FTimerHandle AttackTimerHandle;

        // ==== �̺�Ʈ ====
        // ���Ͱ� ���� �� �˸�
        UPROPERTY(BlueprintAssignable, Category = "Monster")
		FOnMonsterDeath OnMonsterDeath; // ���Ͱ� ���� �� �˸� �̺�Ʈ
        
        UPROPERTY(BlueprintAssignable, Category = "Monster")
        FOnHpMonster OnHpMonster;

        // ==== �Լ� ====
        UFUNCTION(BlueprintCallable, Category = "Monster")
		void TakeDamageCustom(int32 DamageAmount);// Ŀ���� ������ �Լ�
      
        

    protected:
		void OnDeath(); // ���Ͱ� �׾��� �� ó�� �Լ�
		void DestroyMonster(); // ���� ���� �Լ�
        
        UFUNCTION(BlueprintCallable, Category = "Monster")
		void PerformAttack();// ���� �Լ�
		FTimerHandle DeathTimerHandle; // ���� �� ���� Ÿ�̸� �ڵ�
};