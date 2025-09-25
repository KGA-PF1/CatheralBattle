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
	PrimaryActorTick.bCanEverTick = true; // Tick 활성화
	bCanAttack = true; // 초기 공격 가능 상태

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

	//// 공격 범위 내에 있을 때만 공격 수행
	//if (Distance <= AttackRange)
	//{
	//
	//	FDamageEvent DamageEvent; // 객체를 명시적으로 생성
	//	PlayerPawn->TakeDamage(AttackPoint);

	//	// 공격 쿨타임 시작
	//	bCanAttack = false;
	//	PlayAttackAnimation();
	//	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AMonster::ResetAttackCooldown, AttackCooldown, false);

	//	// 공격 애니메이션 재생 등 여기서 추가 구현 가능
	//}
}

void AMonster::OnDeath()
{
	OnMonsterDeath.Broadcast(this);

	GetWorldTimerManager().ClearTimer(AttackTimerHandle); // 공격 타이머 종료

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

