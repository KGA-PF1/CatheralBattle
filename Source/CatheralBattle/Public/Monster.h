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
    virtual float TakeDamage(float DamageAmount,
        struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator,
        AActor* DamageCauser) override;

public:
    AMonster();
public:
        // ==== ���� ====
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float MaxHP = 25.f; //�ִ�ü��

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float CurrentHP = 25.f; //���� ä��

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float AttackPoint = 5.f;//���ݷ�

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float UltGaugeReward = 5.f;//�ñ��� ����Ʈ

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float Speed = 300.f;//�ӵ�

        UFUNCTION(BlueprintCallable, Category = "Stats")
        float GetHp() { return CurrentHP; }
        UFUNCTION(BlueprintCallable, Category = "Stats")
        float GetMaxHp() { return MaxHP; }
        UFUNCTION(BlueprintCallable, Category = "Stats")
        float GetHpPercent() { return MaxHP > 0 ? CurrentHP / MaxHP : 0.f; }

        // ���� ���� �� ���ݷ� ���� �߰�
        UPROPERTY(EditAnywhere, Category = "Combat")
		float AttackRange = 150.f;// ���� ���� ����

        // ���� ��Ÿ�� ���� (�� ����)
        UPROPERTY(EditAnywhere, Category = "Combat")
		float AttackCooldown = 5.f;// ���� ��Ÿ�� ����

        bool bCanAttack; // ���� ���� ���� ����

        void ResetAttackCooldown();// ���� ��Ÿ�� ���� �Լ�

        void PerformAttack(); // ���� �Լ�

        void OnDeath(); // ���Ͱ� �׾��� �� ó�� �Լ�

        void DestroyMonster(); // ���� ���� �Լ�
        
        void OnSpawnAnimationFinished();

        FTimerHandle AttackTimerHandle;
        FTimerHandle DeathTimerHandle;

        // ==== �̺�Ʈ ====
        // ���Ͱ� ���� �� �˸�
        UPROPERTY(BlueprintAssignable, Category = "Monster")
        FOnMonsterDeath OnMonsterDeath; // ���Ͱ� ���� �� �˸� �̺�Ʈ
		// ������ HP�� ����� �� �˸� (������ ��ε�ĳ��Ʈ)
        UPROPERTY(BlueprintAssignable, Category = "Monster")
        FOnHpMonster OnHpMonster;

    protected:	
        void PlayAttackAnimation();
};