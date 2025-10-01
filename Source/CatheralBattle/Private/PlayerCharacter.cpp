// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Monster.h"

APlayerCharacter::APlayerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->GravityScale = 1.75f;


	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	//무기 히트박스
	Weapon = CreateDefaultSubobject<UBoxComponent>(TEXT("Sword"));
	Weapon->SetupAttachment(GetMesh());
	Weapon->InitBoxExtent(FVector(10, 10, 80));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Weapon->SetCollisionObjectType(ECC_WorldDynamic);
	Weapon->SetCollisionResponseToAllChannels(ECR_Ignore);
	Weapon->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Weapon->SetGenerateOverlapEvents(true);

	Stats.Hp = Stats.MaxHp;
}

void APlayerCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	SyncMovementSpeed();
	OnHpChanged.Broadcast(Stats.Hp, Stats.MaxHp);

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	//무기 히트박스
	if (Weapon && GetMesh())
	{
		Weapon->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			WeaponSocketName
		);
		Weapon->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnWeaponBeginOverlap);
	}
	//if(QSphere)
	//	QSphere->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnWeaponBeginOverlap);
	//if(ESphere)
	//	ESphere->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnWeaponBeginOverlap);

	//초기 쿨다운 0
	for (const auto& Pair : SkillTable)
	{
		CooldownTimers.Add(Pair.Key, 0.f);
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateCooldowns(DeltaTime);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

		//Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &APlayerCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopSprint);

		//공격 관련
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &APlayerCharacter::Input_Attack);
		EnhancedInputComponent->BindAction(SkillQAction, ETriggerEvent::Started, this, &APlayerCharacter::Input_SkillQ);
		EnhancedInputComponent->BindAction(SkillEAction, ETriggerEvent::Started, this, &APlayerCharacter::Input_SkillE);
		EnhancedInputComponent->BindAction(UltimateAction, ETriggerEvent::Started, this, &APlayerCharacter::Input_Ult);
		//EnhancedInputComponent->BindAction(ShieldAction, ETriggerEvent::Started, this, &APlayerCharacter::)
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APlayerCharacter::StartSprint()
{
	bIsSprinting = true;
	SyncMovementSpeed();
}

void APlayerCharacter::StopSprint()
{
	bIsSprinting = false;
	SyncMovementSpeed();
}

void APlayerCharacter::SyncMovementSpeed()
{
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		const float Base = Stats.Speed;
		const float Desired = bIsSprinting ? Base * SprintMultiplier : Base;
		Move->MaxWalkSpeed = Desired;
	}
}

float APlayerCharacter::GetCooldownRemaining(ESkillInput Input) const
{
	if (const float* P = CooldownTimers.Find(Input))
	{
		return *P;
	}
	return 0.0f;
}

float APlayerCharacter::GetCooldownDuration(ESkillInput Input) const
{
	if (const FSkillSpec* Spec = SkillTable.Find(Input))
	{
		return FMath::Max(0.f, Spec->CooldownSec);
	}
	return 0.0f;
}

float APlayerCharacter::GetCooldownPercent(ESkillInput Input) const
{
	const float Duration = GetCooldownDuration(Input);
	const float Remaining = GetCooldownRemaining(Input);
	return (Duration > 0.f) ? (Remaining / Duration) : 0.f;
}

float APlayerCharacter::TakeDamage(float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	if (bInvincibleByHit || bInvincibleBySkill || DamageAmount <= 0) return 0.f;

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage <= 0.f) return 0.f;

	const int32 OldHp = Stats.Hp;
	Stats.Hp = FMath::Clamp(Stats.Hp - ActualDamage, 0, Stats.MaxHp);

	if (Stats.Hp != OldHp)
	{
		OnHpChanged.Broadcast(Stats.Hp, Stats.MaxHp);

		if (!IsDead() && HitMontage)
		{
			if (UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
			{
				const bool bSkillPlaying = (ActiveSkillMontage && Anim->Montage_IsPlaying(ActiveSkillMontage));
				const bool bInSkill = (CurrentSkill != ESkillInput::None && CurrentSkill != ESkillInput::Attack);

				if (bSkillPlaying || bInSkill)
					return ActualDamage;

				Anim->StopAllMontages(0.1f);
				AN_Sword_Off();
				BP_ResetAttackState();

				float Len = Anim->Montage_Play(HitMontage);
				if (Len > 0.f)
				{
					bInvincibleByHit = true;
					//SetCanBeDamaged(false);
					bCanAttack = false;

					//잠금 해제 이벤트 바인딩(끝, 중단)
					FOnMontageBlendingOutStarted BlendOut;
					BlendOut.BindUObject(this, &APlayerCharacter::OnMontageBlendOut);
					Anim->Montage_SetBlendingOutDelegate(BlendOut, HitMontage);

					FOnMontageEnded Ended;
					Ended.BindUObject(this, &APlayerCharacter::OnMontageEnded);
					Anim->Montage_SetEndDelegate(Ended, HitMontage);
				}
				else
				{
					bCanAttack = true;
				}
			}
		}
	}

	if (IsDead())
	{
		OnDeath();
	}

	return ActualDamage;
}

void APlayerCharacter::TakeDamage(float Damage)
{
	if (bInvincibleByHit) return;
	if (Damage <= 0) return;

	const int32 OldHp = Stats.Hp;
	Stats.Hp = FMath::Clamp(Stats.Hp - Damage, 0, Stats.MaxHp);

	if (Stats.Hp != OldHp)
	{
		OnHpChanged.Broadcast(Stats.Hp, Stats.MaxHp);

		if (!IsDead() && HitMontage)
		{
			if (UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
			{
				const bool bSkillPlaying = (ActiveSkillMontage && Anim->Montage_IsPlaying(ActiveSkillMontage));
				const bool bInSkill = (CurrentSkill != ESkillInput::None && CurrentSkill != ESkillInput::Attack);

				if (bSkillPlaying || bInSkill)
					return;

				Anim->StopAllMontages(0.1f);
				AN_Sword_Off();
				BP_ResetAttackState();
				float Len = Anim->Montage_Play(HitMontage);
				if (Len > 0.f)
				{
					bInvincibleByHit = true;
					//SetCanBeDamaged(false);
					bCanAttack = false;

					//잠금 해제 이벤트 바인딩(끝, 중단)
					FOnMontageBlendingOutStarted BlendOut;
					BlendOut.BindUObject(this, &APlayerCharacter::OnMontageBlendOut);
					Anim->Montage_SetBlendingOutDelegate(BlendOut, HitMontage);

					FOnMontageEnded Ended;
					Ended.BindUObject(this, &APlayerCharacter::OnMontageEnded);
					Anim->Montage_SetEndDelegate(Ended, HitMontage);
				}
				else
				{
					bCanAttack = true;
				}
			}
		}
	}

	if (IsDead())
	{
		OnDeath();
	}
}

void APlayerCharacter::Respawn()
{
	if (Stats.Hp != Stats.MaxHp)
	{
		Stats.Hp = Stats.MaxHp;
		OnHpChanged.Broadcast(Stats.Hp, Stats.MaxHp);
	}
}

void APlayerCharacter::OnDeath()
{
	SetCanBeDamaged(false); //데미지 무시

	if (DeathMontage)
	{
		if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
		{
			UE_LOG(LogTemp, Warning, TEXT("데스몽타주"));

			//델리게이트를 변수로 만들어서
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &APlayerCharacter::OnMontageEnded);

			//lvalue로 전달함
			Anim->Montage_SetEndDelegate(EndDelegate, DeathMontage);

			//재생
			Anim->Montage_Play(DeathMontage);
		}
	}
	else
	{
		if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
		{
			Anim->StopAllMontages(0.2f);
		}
	}

	//이동, 입력 정지
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->DisableMovement();
		Move->SetMovementMode(MOVE_None);
		//Move->Velocity = FVector::ZeroVector;
		//Move->bOrientRotationToMovement = false;
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}



	//충돌 처리
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//컨트롤러 분리(시점 유지 필요없을 경우)
	//DetachFromControllerPendingDestroy();

	OnPlayerDeath.Broadcast(this);

}

void APlayerCharacter::AddUltGauge(float Amount)
{
	Stats.UltGauge = FMath::Clamp(Stats.UltGauge + Amount, 0.f, Stats.MaxUltGauge);
	OnUltGaugeChanged.Broadcast(Stats.UltGauge, Stats.MaxUltGauge);
}

bool APlayerCharacter::TryUseSkill(ESkillInput InputKind)
{
	if (const FSkillSpec* Spec = SkillTable.Find(InputKind))
	{
		return InternalUseSkill(*Spec, InputKind);
	}
	return false;
}

void APlayerCharacter::Input_Attack()
{
	CurrentSkill = ESkillInput::Attack;
	TryUseAttack();
}

void APlayerCharacter::Input_SkillQ()
{
	CurrentSkill = ESkillInput::Skill_Q;
	TryUseSkillQ();
}

void APlayerCharacter::Input_SkillE()
{
	CurrentSkill = ESkillInput::Skill_E;
	TryUseSkillE();
}

void APlayerCharacter::Input_Ult()
{
	CurrentSkill = ESkillInput::Ult_R;
	if (Stats.UltGauge)
		TryUseUlt();
}

void APlayerCharacter::AN_Sword_On()
{
	if (Weapon)
	{
		CurrentSkill = ESkillInput::Attack;
		HitActorsThisSwing.Reset();
		Weapon->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void APlayerCharacter::AN_Sword_Off()
{
	if (Weapon)
	{
		CurrentSkill = ESkillInput::None;
		Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

bool APlayerCharacter::InternalUseSkill(const FSkillSpec& Spec, ESkillInput InputKind)
{
	//몽타주 없는 스킬 무시
	if (!Spec.Montage) return false;

	//중복 시전 방지
	if (UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		if (Anim->IsAnyMontagePlaying())
		{
			const bool bHitPlaying = (HitMontage && Anim->Montage_IsPlaying(HitMontage));
			if (bHitPlaying)
			{
				Anim->Montage_Stop(0.1f, HitMontage);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Attack blocked: IsAnyMontagePlaying() = true"));
				return false;
			}
		}
	}

	//간단 쿨타임 체크
	if (float* Timer = CooldownTimers.Find(InputKind))
	{
		if (*Timer > 0.f) return false;
	}

	//턴제: AP 체크
	if (bTurnBased && Spec.APCost > 0)
	{
		if (Stats.AP < Spec.APCost) return false;
		Stats.AP -= Spec.APCost;
	}

	//히트박스 모양 업데이트
	if (Weapon && Spec.bUseWeaponHitBox)
	{
		Weapon->SetBoxExtent(Spec.BoxExtent, true);
		Weapon->SetRelativeLocation(Spec.BoxRelLocation);
		Weapon->SetRelativeRotation(Spec.BoxRelRotation);
	}
	bCanAttack = false;

	//몽타주 재생
	if (InputKind == ESkillInput::Skill_Q)
	{
		Jump();
		LaunchCharacter(FVector(0.f, 0.f, 700.f), true, true);

		PlaySkillMontage(Spec);
		GetWorldTimerManager().ClearTimer(MovementLockStartHandle);
		GetWorldTimerManager().SetTimer(
			MovementLockStartHandle,
			this,
			//	&APlayerCharacter::LockMoveInputDelayed,
			&APlayerCharacter::LockMoveInput,
			1.0f,
			false
		);
		//LockMoveInput();
	}
	else if (InputKind == ESkillInput::Ult_R)
	{
		LockMoveInput();
		PlaySkillMontage(Spec);
	}
	else
	{
		PlaySkillMontage(Spec);
	}
	//쿨다운 스타트
	if (float* Timer = CooldownTimers.Find(InputKind))
	{
		const float Duration = FMath::Max(0.f, Spec.CooldownSec);
		*Timer = Duration;
		BroadcastCooldown(InputKind, *Timer, Duration);
	}

	return true;
}

void APlayerCharacter::UpdateCooldowns(float DeltaTime)
{
	for (auto& Pair : CooldownTimers)
	{
		const ESkillInput Input = Pair.Key;
		float& Remaining = Pair.Value;

		if (Remaining > 0.f)
		{
			const float Old = Remaining;
			Remaining = FMath::Max(0.f, Remaining - DeltaTime);

			//총 지속시간은 스킬테이블에서 조회
			const FSkillSpec* Spec = SkillTable.Find(Input);
			const float Duration = Spec ? FMath::Max(0.f, Spec->CooldownSec) : 0.f;

			//값 변동 시에만 브로드캐스트
			if (!FMath::IsNearlyEqual(Old, Remaining))
			{
				BroadcastCooldown(Input, Remaining, Duration);
			}
			if (Remaining <= 0.f)
			{
				OnCooldownEnded.Broadcast(Input);
			}
		}
	}
}

void APlayerCharacter::PlaySkillMontage(const FSkillSpec& Spec)
{
	if (!Spec.Montage) return;
	if (UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		float Len = Anim->Montage_Play(Spec.Montage);
		if (Len > 0.f)
		{
			ActiveSkillMontage = Spec.Montage;
			bCanAttack = false;

			bInvincibleBySkill = true;
			//SetCanBeDamaged(false);

			//잠금 해제 이벤트 바인딩(끝, 중단)
			FOnMontageBlendingOutStarted BlendOut;
			BlendOut.BindUObject(this, &APlayerCharacter::OnMontageBlendOut);
			Anim->Montage_SetBlendingOutDelegate(BlendOut, Spec.Montage);

			FOnMontageEnded Ended;
			Ended.BindUObject(this, &APlayerCharacter::OnMontageEnded);
			Anim->Montage_SetEndDelegate(Ended, Spec.Montage);
		}
	}
}

void APlayerCharacter::OnWeaponBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (!OtherActor || OtherActor == this || IsDead()) return;

	if (HitActorsThisSwing.Contains(OtherActor)) return; //중복 타격 방지
	AMonster* DamagedActor = Cast<AMonster>(OtherActor);
	if (DamagedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Successfully damaged actor: %s"), *DamagedActor->GetName());

		const float Damage = CalcAttackDamage();
		if (Damage <= 0.f) return;

		//Instigator, DamageCauser 설정
		AController* InstigatorCtrl = GetController();

		UGameplayStatics::ApplyDamage(DamagedActor, Damage, InstigatorCtrl, this, UDamageType::StaticClass());

		HitActorsThisSwing.Add(DamagedActor);
	}
}

void APlayerCharacter::ExecuteAOEAttack()
{
	const float Damage = CalcAttackDamage();
	const float Range = CalcAttackRange();

	UWorld* World = GetWorld();
	if (!World)
		return;

	FVector AttackCenter;
	if (CurrentSkill == ESkillInput::Skill_Q)
	{
		FName SkillWeaponSocketName = TEXT("FX_Sword_Top");
		if (GetMesh() && GetMesh()->DoesSocketExist(SkillWeaponSocketName))
		{
			AttackCenter = GetMesh()->GetSocketLocation(SkillWeaponSocketName);
		}
	}
	// 공격 중심 위치 (캐릭터 위치)
	else
	{
		AttackCenter = GetActorLocation();
	}

	// 오버랩할 액터들 저장할 배열
	TArray<FOverlapResult> OverlapResults;

	// 충돌 쿼리 파라미터 설정
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // 자기 자신 제외

	// 충돌 체크할 오브젝트 타입 설정
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	// 구체 범위 내의 액터들 검색 (즉발)
	bool bHasHit = World->OverlapMultiByObjectType(
		OverlapResults,
		AttackCenter,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(Range),
		QueryParams
	);

	// 히트한 액터들 처리
	if (bHasHit)
	{
		//액터 중복 제거
		TSet<AActor*> UniqueActors;
		for (const FOverlapResult& Result : OverlapResults)
		{
			if (AActor* HitActor = Result.GetActor())
			{
				UniqueActors.Add(HitActor);
			}
		}

		for (AActor* Actor : UniqueActors)
		{
			// Monster 타입인지 확인
			AMonster* Monster = Cast<AMonster>(Actor);
			if (Monster)
			{
				UE_LOG(LogTemp, Warning, TEXT("Successfully damaged actor: %s"), *Monster->GetName());
				// 데미지 적용 (ApplyDamage만 사용)
				UGameplayStatics::ApplyDamage(
					Monster,
					Damage,
					GetController(),
					this,
					UDamageType::StaticClass()
				);
			}
		}


	}

	// 디버그 시각화
	if (bShowDebugSphere)
	{
		DrawDebugSphere(
			World,
			AttackCenter,
			Range,
			24,
			FColor::Red,
			false,
			5.0f
		);
	}
}

void APlayerCharacter::BroadcastCooldown(ESkillInput Input, float Remaining, float Duration)
{
	OnCooldownUpdated.Broadcast(Input, Remaining, Duration);
}

void APlayerCharacter::OnMontageBlendOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == ActiveSkillMontage)
	{
		ActiveSkillMontage = nullptr;
		bInvincibleBySkill = false;
		CurrentSkill = ESkillInput::None;
	}
	if (Montage == HitMontage)
	{
		bInvincibleByHit = false;
	}
	UnLockMoveInput();
	bCanAttack = true;
}
void APlayerCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == DeathMontage)
	{
		//사망용처리
		GetCharacterMovement()->DisableMovement();   // 다시 안 움직이게
		bCanAttack = false;
		GetMesh()->bPauseAnims = true;
		return;
	}

	if (Montage == ActiveSkillMontage)
	{
		ActiveSkillMontage = nullptr;
		bInvincibleBySkill = false;
		CurrentSkill = ESkillInput::None;
	}
	// HitMontage 종료 디버그 로그
	if (Montage == HitMontage)
	{
		bInvincibleByHit = false;
		AN_Sword_Off();
	}

	UnLockMoveInput();
	bCanAttack = true;
}

void APlayerCharacter::LockMoveInput()
{
	if (bMoveInputLocked) return;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetIgnoreMoveInput(true);
	}
	bMoveInputLocked = true;
}

void APlayerCharacter::UnLockMoveInput()
{
	if (!bMoveInputLocked) return;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetIgnoreMoveInput(false);
	}
	bMoveInputLocked = false;
}

void APlayerCharacter::MirrorAllTo(APlayerCharacter* Dest, bool bCopyCooldowns) const
{
	if (!Dest) return;

	// 스탯 통째 복사
	Dest->Stats = this->Stats; // Hp/MaxHp/AtkPoint/Speed/UltGauge/MaxUltGauge/AP 전부 포함

	// 런타임 상태
	Dest->SprintMultiplier = this->SprintMultiplier;
	Dest->bIsSprinting = this->bIsSprinting;
	Dest->bCanAttack = this->bCanAttack;

	// 이동속도 반영
	Dest->SyncMovementSpeed();

	// UI 이벤트 재브로드캐스트(즉시 HUD 반영)
	Dest->OnHpChanged.Broadcast(Dest->Stats.Hp, Dest->Stats.MaxHp);
	Dest->OnUltGaugeChanged.Broadcast(Dest->Stats.UltGauge, Dest->Stats.MaxUltGauge);
}


float APlayerCharacter::GetSkillDamage(ESkillInput Input) const
{
	if (const FSkillSpec* Found = SkillTable.Find(Input))
	{
		return Stats.AtkPoint * Found->DamageMultiplier;
	}
	return 0.f;
}
float APlayerCharacter::CalcAttackDamage() const
{
	//실제 데미지 계산
	const FSkillSpec* Basic = SkillTable.Find(CurrentSkill);
	const float Mult = Basic ? FMath::Max(1.f, Basic->DamageMultiplier) : 1.f;
	return FMath::Max(0.f, Stats.AtkPoint * Mult);
}

float APlayerCharacter::CalcAttackRange() const
{

	switch (CurrentSkill)
	{
	case ESkillInput::Skill_Q: return QSkillRange;
	case ESkillInput::Skill_E: return ESkillRange;
	default: return 0.f;
	}
}