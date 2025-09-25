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
#include "Kismet/GameplayStatics.h"
#include "Monster.h"
#include "Components/SphereComponent.h"

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
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Weapon->SetGenerateOverlapEvents(true);
	Weapon->SetCollisionObjectType(ECC_WorldDynamic);
	Weapon->SetCollisionResponseToAllChannels(ECR_Ignore);
	Weapon->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
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

void APlayerCharacter::TakeDamage(float Damage)
{
	if (Damage <= 0 || IsDead()) return;
	const int32 OldHp = Stats.Hp;
	Stats.Hp = FMath::Clamp(Stats.Hp - Damage, 0, Stats.MaxHp);
	if (Stats.Hp != OldHp)
	{
		OnHpChanged.Broadcast(Stats.Hp, Stats.MaxHp);
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
	TryUseAttack();
}

void APlayerCharacter::Input_SkillQ()
{
	TryUseSkillQ();
}

void APlayerCharacter::Input_SkillE()
{
	TryUseSkillE();
}

void APlayerCharacter::Input_Ult()
{
	if (Stats.UltGauge)
		TryUseUlt();
}

void APlayerCharacter::AN_Sword_On()
{
	if (Weapon)
	{
		HitActorsThisSwing.Reset();
		Weapon->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void APlayerCharacter::AN_Sword_Off()
{
	if (Weapon)
	{
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
		if (Anim->IsAnyMontagePlaying()) return false;
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
			//잠금 해제 이벤트 바인딩(끝, 중단)
			FOnMontageBlendingOutStarted BlendOut;
			BlendOut.BindUObject(this, &APlayerCharacter::OnMontageBlendOut);
			Anim->Montage_SetBlendingOutDelegate(BlendOut, Spec.Montage);

			FOnMontageEnded Ended;
			Ended.BindUObject(this, &APlayerCharacter::OnMontageEnded);
			Anim->Montage_SetEndDelegate(Ended, Spec.Montage);
		}
		//if (Len > 0.f && Spec.MontageSection != NAME_None)
		//{
		//	Anim->Montage_JumpToSection(Spec.MontageSection, Spec.Montage);
		//}
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

void APlayerCharacter::BroadcastCooldown(ESkillInput Input, float Remaining, float Duration)
{
	OnCooldownUpdated.Broadcast(Input, Remaining, Duration);
}

void APlayerCharacter::OnMontageBlendOut(UAnimMontage* Montage, bool bInterrupted)
{
	UnLockMoveInput();
	bCanAttack = true;
}
void APlayerCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
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

float APlayerCharacter::CalcAttackDamage() const
{
	//실제 데미지 계산
	const FSkillSpec* Basic = SkillTable.Find(ESkillInput::Attack);
	const float Mult = Basic ? FMath::Max(0.f, Basic->DamageMultiplier) : 1.f;
	return FMath::Max(0.f, Stats.AtkPoint * Mult);
}

void APlayerCharacter::AN_QSkillCircle(float Radius, float Damage)
{
	if (Radius <= 0.f || Damage <= 0.f || !GetMesh()) return;

	UWorld* World = GetWorld();
	if (!World) return;
	
	//AMonster만 뽑아오는 클래스 필터
	TArray<TSubclassOf<AActor>> ClassFilter;
	ClassFilter.Add(AMonster::StaticClass());

	TArray<AActor*> Ignore;
	Ignore.Add(this); //플레이어 무시

	TArray<AActor*> OutActors;
	const FVector Center = GetActorLocation();

	const bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		World, Center, Radius,
		TArray<TEnumAsByte<EObjectTypeQuery>>(),
		nullptr,
		Ignore, OutActors
	);

	if (!bHit) return;

	AController* InstigatorCtrl = GetController();
	for (AActor* A : OutActors)
	{
		if (AMonster* M = Cast<AMonster>(A))
		{
			UGameplayStatics::ApplyDamage(M, Damage, InstigatorCtrl, this, UDamageType::StaticClass());
			UE_LOG(LogTemp, Warning, TEXT("QQQQQQQQQSuccessfully damaged actor: %s"), *M->GetName());
		}
	}
}

void APlayerCharacter::AN_ESkillCircle(float Radius, float Damage)
{
	//Radius랑 Damage만 다름
	AN_QSkillCircle(Radius, Damage);
}

void APlayerCharacter::AN_StartPersistentAoE(float Radius, float DamagePerSecond, float TickInterval, float LifetimeSec)
{
	if (Radius <= 0.f || DamagePerSecond <= 0.f || TickInterval <= 0.f) return;

	UWorld* World = GetWorld();
	if (!World) return;

	AN_EndPersistentAoE();

	PersistentAoEComp = NewObject<USphereComponent>(this, TEXT("PersistentAoE"));
	PersistentAoEComp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	PersistentAoEComp->SetSphereRadius(Radius);
	PersistentAoEComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PersistentAoEComp->SetCollisionObjectType(ECC_WorldDynamic);
	PersistentAoEComp->SetGenerateOverlapEvents(true);

	//Pawn(몬스터 캡슐)과 Overlap 되게
	PersistentAoEComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	PersistentAoEComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	PersistentAoEComp->RegisterComponent();

	PersistentAoEComp->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnPersistentAoEBeginOverlap);
	PersistentAoEComp->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnPersistentAoEEndOverlap);

	//파라미터 저장
	Persistent_Damage = DamagePerSecond;
	Persistent_TickInterval = TickInterval;

	//Tick 타이머 시작
	GetWorldTimerManager().SetTimer(
		PersistentAoE_TickTimer,
		this,
		&APlayerCharacter::DealPersistentAoEDamage,
		TickInterval, true, TickInterval
	);

	//수명 타이머
	if (LifetimeSec > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			PersistentAoE_LifeTimer,
			this,
			&APlayerCharacter::AN_EndPersistentAoE,
			LifetimeSec, false
		);
	}
}

void APlayerCharacter::AN_EndPersistentAoE()
{
	//타이머 정리
	GetWorldTimerManager().ClearTimer(PersistentAoE_TickTimer);
	GetWorldTimerManager().ClearTimer(PersistentAoE_LifeTimer);

	//컴포넌트, 집합 정리
	if (PersistentAoEComp)
	{
		PersistentAoEComp->OnComponentBeginOverlap.RemoveAll(this);
		PersistentAoEComp->OnComponentEndOverlap.RemoveAll(this);
		PersistentAoEComp->DestroyComponent();
		PersistentAoEComp = nullptr;
	}

	PersistentAoEActors.Reset();
	Persistent_Damage = 0.f;
	Persistent_TickInterval = 1.f;
}

void APlayerCharacter::OnPersistentAoEBeginOverlap(UPrimitiveComponent* Comp, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (AMonster* M = Cast<AMonster>(Other))
	{
		PersistentAoEActors.Add(M);
	}
}

void APlayerCharacter::OnPersistentAoEEndOverlap(UPrimitiveComponent* Comp, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex)
{
	if (AMonster* M = Cast<AMonster>(Other))
	{
		PersistentAoEActors.Remove(M);
	}
}

void APlayerCharacter::DealPersistentAoEDamage()
{
	if (Persistent_Damage <= 0.f || Persistent_TickInterval <= 0.f) return;
	const float DamagePerTick = Persistent_Damage * Persistent_TickInterval;
	if (DamagePerTick <= 0.f) return;

	AController* InstigatorCtrl = GetController();

	//집합 순회하며 유효한 몬스터만 타격
	TArray<TWeakObjectPtr<AMonster>> ToRemove;
	for (const TWeakObjectPtr<AMonster>& WeakM : PersistentAoEActors)
	{
		AMonster* M = WeakM.Get();
		if (!IsValid(M)) { ToRemove.Add(WeakM); continue; }

		UGameplayStatics::ApplyDamage(M, DamagePerTick, InstigatorCtrl, this, UDamageType::StaticClass());
	}

	//무효 포인터 제거
	for (const auto& W : ToRemove) { PersistentAoEActors.Remove(W); }
}
