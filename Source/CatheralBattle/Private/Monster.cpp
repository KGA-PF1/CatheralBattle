// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h" // 추가
#include "GameFramework/CharacterMovementComponent.h" // 추가
#include "PlayerCharacter.h"


// Sets default values
AMonster::AMonster()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true; // Tick 활성화
	bCanAttack = true; // 초기 공격 가능 상태
	bIsAttacking = false;

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
	if (GetMesh() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->OnMontageEnded.AddDynamic(this, &AMonster::OnAttackMontageEnded);
		GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	}
}

void AMonster::ResetAttackCooldown()
{

	bCanAttack = true;
}

float AMonster::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageAmount <= 0.f || bIsDead)  // 이미 죽은 상태면 무시
		return 0.f;

	CurrentHP -= (int32)DamageAmount;
	if (CurrentHP < 0) CurrentHP = 0;

	if (HitMontage && !bIsDead)
	{
		PlayAnimMontage(HitMontage);
	}

	OnHpMonster.Broadcast((float)CurrentHP, (float)MaxHP);

	if (CurrentHP <= 0 && !bIsDead)  // 죽었을 때만 OnDeath 호출
	{
		OnDeath();
	}

	return DamageAmount;
}

void AMonster::PerformAttack()
{
	if (!bCanAttack || bIsDead)
		return;

	AActor* TargetActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	if (!TargetActor)
		return;

	float Distance = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());

	if (Distance <= AttackRange)  // 공격 거리 내면 공격
	{
		// 공격 로직 (데미지 전달 등) 여기에 구현
		bCanAttack = false;

		// 5초 후 공격 가능 상태로 복구
		GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AMonster::ResetAttackCooldown, 5.f, false);
	}
}

void AMonster::OnDeath()
{
	bIsDead = true;
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		AICon->StopMovement();
		if (AICon->GetBrainComponent())
		{
			AICon->GetBrainComponent()->StopLogic("Dead");
		}
		UBlackboardComponent* BlackboardComp = AICon->GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsBool("IsDead", true);
		}
	}

	if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)))
	{
		PlayerChar->AddUltGauge(UltGaugeReward);
	}

	GetMesh()->SetSimulatePhysics(true); // 물리 시뮬레이션 활성화
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll_NoCamera"));
	GetCharacterMovement()->DisableMovement();
	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	OnMonsterDeath.Broadcast(this);

	if (DeathMontage && GetMesh()->GetAnimInstance())
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->GetAnimInstance()->Montage_Play(DeathMontage);

	}

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

void AMonster::StartAttack()
{
	if (bIsAttacking) return;

	bIsAttacking = true;

	if (AttackMontage && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(AttackMontage);
	}
}

void AMonster::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == AttackMontage)
	{
		bIsAttacking = false;
		OnAttackEnded();  // 블루프린트에서 이동 재개 등 추가 처리할 수 있음
	}
}



 //Called every frame
void AMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

