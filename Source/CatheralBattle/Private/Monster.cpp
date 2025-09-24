// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
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
	OnHpMonster.Broadcast(CurrentHP, MaxHP);
	
}

void AMonster::ResetAttackCooldown()
{
	bCanAttack = true;
}


void AMonster::TakeDamageCustom(int32 DamageAmount)
{
	CurrentHP -= DamageAmount;
	if (CurrentHP < 0) CurrentHP = 0;

	// HP 변경 이벤트 발생 (위젯에 브로드캐스트)
	OnHpMonster.Broadcast((float)CurrentHP, (float)MaxHP);

	if (CurrentHP <= 0)
	{
		OnDeath();
	}
}

void AMonster::OnDeath()
{
	// 죽었다고 알림을 보냄 → Spawner/GameMode에서 바인딩 가능
	OnMonsterDeath.Broadcast(this);

	// 1초 뒤 Destroy 처리
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

	APlayerCharacter* PlayerPawn = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (!PlayerPawn)
		return;

	float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), GetActorLocation());

	// 공격 범위 내에 있을 때만 공격 수행
	if (Distance <= AttackRange)
	{
		// 데미지 적용 (플레이어에 데미지 전달)
		////////////////UGameplayStatics::ApplyDamage(PlayerPawn, AttackPoint, GetController(), this, nullptr);
		PlayerPawn->TakeDamage(AttackPoint);

		// 공격 쿨타임 시작
		bCanAttack = false;
		GetWorldTimerManager().SetTimer(
			AttackTimerHandle, this, &AMonster::ResetAttackCooldown, AttackCooldown, false);

		// 공격 애니메이션 재생 등 여기서 추가 구현 가능
	}
}

 //Called every frame
void AMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 플레이어와 거리를 체크해 공격 범위 이내면 자동 공격 시도
	PerformAttack();

}
//
//// Called to bind functionality to input
//void AMonster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
//{
//	Super::SetupPlayerInputComponent(PlayerInputComponent);
//
//}