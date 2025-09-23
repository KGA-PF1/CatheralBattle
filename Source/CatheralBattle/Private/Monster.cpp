// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMonster::AMonster()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

//// Called when the game starts or when spawned
void AMonster::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;
}

void AMonster::ResetAttackCooldown()
{
	bCanAttack = true;
}



void AMonster::TakeDamageCustom(int32 DamageAmount)
{
	if (CurrentHP <= 0) return; // �̹� ��� ���¶�� ����

	CurrentHP -= DamageAmount;

	if (CurrentHP <= 0)
	{
		OnDeath();
	}
}

void AMonster::OnDeath()
{
	// �׾��ٰ� �˸��� ���� �� Spawner/GameMode���� ���ε� ����
	OnMonsterDeath.Broadcast(this);

	// 1�� �� Destroy ó��
	GetWorldTimerManager().SetTimer(
		DeathTimerHandle,
		this,
		&AMonster::DestroyMonster,
		1.0f,
		false
	);
}

void AMonster::DestroyMonster()
{
	Destroy();
}

void AMonster::PerformAttack()
{
	if (!bCanAttack)
		return;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
		return;

	float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), GetActorLocation());

	// ���� ���� ���� ���� ���� ���� ����
	if (Distance <= AttackRange)
	{
		// ������ ���� (�÷��̾ ������ ����)
		UGameplayStatics::ApplyDamage(PlayerPawn, AttackPoint, GetController(), this, nullptr);

		// ���� ��Ÿ�� ����
		bCanAttack = false;
		GetWorldTimerManager().SetTimer(
			AttackTimerHandle, this, &AMonster::ResetAttackCooldown, AttackCooldown, false);

		// ���� �ִϸ��̼� ��� �� ���⼭ �߰� ���� ����
	}
}

// Called every frame
//void AMonster::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}
//
//// Called to bind functionality to input
//void AMonster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
//{
//	Super::SetupPlayerInputComponent(PlayerInputComponent);
//
//}