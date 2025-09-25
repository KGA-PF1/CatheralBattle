#include "NPC.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"


void ANPC::BeginPlay()
{
    Super::BeginPlay();
}

void ANPC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// === Dialogue Functions ===
FText ANPC::GetCurrentDialogue() const
{
    if (DialogueLines.IsValidIndex(CurrentLineIndex))
    {
        return DialogueLines[CurrentLineIndex];
    }
    return FText::FromString(TEXT(""));
}

bool ANPC::NextDialogue()
{
    if (CurrentLineIndex + 1 < DialogueLines.Num())
    {
        CurrentLineIndex++;
        return true;
    }
    return false;
}

// === Overlap ===
void ANPC::OnRangeBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // C++ → BP 이벤트 호출
    OnRangeBeginBP(OtherActor);
}

void ANPC::OnRangeEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // C++ → BP 이벤트 호출
    OnRangeEndBP(OtherActor);
}