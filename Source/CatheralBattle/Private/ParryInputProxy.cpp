#include "ParryInputProxy.h"
#include "ParryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/InputComponent.h"

AParryInputProxy::AParryInputProxy()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AParryInputProxy::BeginPlay()
{
	Super::BeginPlay();
	CacheParryComponent();
	BindKeys();
}

void AParryInputProxy::CacheParryComponent()
{
	APawn* P = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!P) return;

	if (UActorComponent* C = P->FindComponentByClass(UParryComponent::StaticClass()))
	{
		ParryComp = Cast<UParryComponent>(C);
	}
}

void AParryInputProxy::BindKeys()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	EnableInput(PC);

	if (!InputComponent)
	{
		InputComponent = NewObject<UInputComponent>(this, TEXT("ParryInputProxyIC"));
		InputComponent->RegisterComponent();
	}
	InputComponent->BindKey(PrimaryKey, IE_Pressed, this, &AParryInputProxy::OnParryPressed);
	if (SecondaryKey.IsValid())
		InputComponent->BindKey(SecondaryKey, IE_Pressed, this, &AParryInputProxy::OnParryPressed);
}

void AParryInputProxy::OnParryPressed()
{
	if (!ParryComp.IsValid())
	{
		CacheParryComponent();
	}
	if (ParryComp.IsValid())
	{
		ParryComp->OnParryPressed();
	}
}
