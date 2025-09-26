// TBPlayerCharacter.cpp
#include "ATBPlayerCharacter.h"
#include "ParryComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "TurnMenuWidget.h"

ATBPlayerCharacter::ATBPlayerCharacter()
{
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->DisableMovement();
		Move->bOrientRotationToMovement = false;
	}
	AutoPossessPlayer = EAutoReceiveInput::Disabled;
}

void ATBPlayerCharacter::SetupPlayerInputComponent(UInputComponent* IC)
{
	Super::SetupPlayerInputComponent(IC);
}

void ATBPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	CacheParry();
	EnsureParryMontageInjected();
}

void ATBPlayerCharacter::CacheParry()
{
	if (!CachedParry.IsValid())
	{
		if (UActorComponent* C = FindComponentByClass(UParryComponent::StaticClass()))
		{
			CachedParry = Cast<UParryComponent>(C);
		}
	}
}

void ATBPlayerCharacter::EnsureParryMontageInjected()
{
	if (CachedParry.IsValid() && M_Parry)
	{
		CachedParry.Get()->ParryMontage = M_Parry; // ParryComponent가 재생
	}
}

UAnimMontage* ATBPlayerCharacter::GetMontageByCommand(EPlayerCommand Cmd) const
{
	switch (Cmd)
	{
	case EPlayerCommand::Attack:  return M_Attack;
	case EPlayerCommand::Skill1:  return M_Skill1;
	case EPlayerCommand::Skill2:  return M_Skill2;
	default: return nullptr;
	}
}

void ATBPlayerCharacter::PlayHitReact() const
{
	PlayMontage(M_HitReact);
}

void ATBPlayerCharacter::PlayMontage(UAnimMontage* M) const
{
	if (!M) return;
	if (USkeletalMeshComponent* SkelMesh = GetMesh())
	{
		if (UAnimInstance* Anim = SkelMesh->GetAnimInstance())
		{
			if (SkelMesh->GetSkeletalMeshAsset()
				&& M->GetSkeleton() == SkelMesh->GetSkeletalMeshAsset()->GetSkeleton())
			{
				Anim->Montage_Play(M, 1.0f);
			}
		}
	}
}


void ATBPlayerCharacter::ActivateTurnCamera(bool bActive)
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (bActive)
		{
			PC->SetViewTargetWithBlend(this, 0.2f);
			PC->SetIgnoreLookInput(true);   // ★ 마우스 시점 고정
			PC->SetIgnoreMoveInput(true);   // ★ WASD 봉인
		}
		else
		{
			PC->SetIgnoreLookInput(false);
			PC->SetIgnoreMoveInput(false);
		}
	}
}