// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
void AMonsterAIController::BeginPlay()
{
	Super::BeginPlay();


	if (AIBehavior != nullptr)
	{
		RunBehaviorTree(AIBehavior);
		//APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(),0);
		UBlackboardComponent* BBComp = GetBlackboardComponent();
		//처음 스폰된 지점,.
		APawn* myPawn = GetPawn();
		if (myPawn != nullptr)
			BBComp->SetValueAsVector(TEXT("SpawnLocation"), myPawn->GetActorLocation());
	}
}

void AMonsterAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn) return;

	APawn* MyPawn = GetPawn();
	if (!MyPawn) return;

	float Dist = FVector::Dist(PlayerPawn->GetActorLocation(), MyPawn->GetActorLocation());

	if (Dist > AcceptanceRadius)
	{
		MoveToActor(PlayerPawn, AcceptanceRadius);
	}
	else
	{
		StopMovement();
	}
}

void AMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

}
