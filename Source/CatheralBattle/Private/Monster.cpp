// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "PlayerCharacter.h"

// Sets default values
AMonster::AMonster()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true; // Tick Ȱ��ȭ
	bCanAttack = true; // �ʱ� ���� ���� ����

}

//// Called when the game starts or when spawned
void AMonster::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;
	OnHpMonster.Broadcast((float)CurrentHP, (float)MaxHP);
	GetWorldTimerManager().SetTimer(AttackTimerHandle, 
		this, &AMonster::PerformAttack, 
		0.1f, true, 0.1f);
}

void AMonster::ResetAttackCooldown()
{
	bCanAttack = true;
}

float AMonster::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageAmount <= 0.f) return 0.f;

	CurrentHP -= (int32)DamageAmount;
	if (CurrentHP < 0) CurrentHP = 0;

	OnHpMonster.Broadcast((float)CurrentHP, (float)MaxHP);

	if (CurrentHP <= 0)
	{
		OnDeath();
	}

	return DamageAmount;
}

void AMonster::PerformAttack()
{
	//if (!bCanAttack)
	//	return;

	//APlayerCharacter* PlayerPawn = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	//if (!PlayerPawn)
	//	return;

	//float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), GetActorLocation());

	//// ���� ���� ���� ���� ���� ���� ����
	//if (Distance <= AttackRange)
	//{
	//
	//	FDamageEvent DamageEvent; // ��ü�� ��������� ����
	//	PlayerPawn->TakeDamage(AttackPoint);

	//	// ���� ��Ÿ�� ����
	//	bCanAttack = false;
	//	PlayAttackAnimation();
	//	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AMonster::ResetAttackCooldown, AttackCooldown, false);

	//	// ���� �ִϸ��̼� ��� �� ���⼭ �߰� ���� ����
	//}
}

void AMonster::OnDeath()
{
	OnMonsterDeath.Broadcast(this);

	GetWorldTimerManager().ClearTimer(AttackTimerHandle); // ���� Ÿ�̸� ����

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


 //Called every frame
void AMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

