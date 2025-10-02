#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NPC.generated.h"

class USphereComponent;
class UCameraComponent;

UCLASS()
class CATHERALBATTLE_API ANPC : public ACharacter
{
    GENERATED_BODY()



protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === Components ===
    UPROPERTY(VisibleAnywhere, Category = "NPC")
    USphereComponent* TalkRange;


    // === Dialogue ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Dialogue")
    TArray<FText> DialogueLines;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Dialogue")
    int32 CurrentLineIndex;

    // === Overlap ===
    UFUNCTION()
    void OnRangeBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnRangeEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // === Blueprint Events ===
    UFUNCTION(BlueprintImplementableEvent, Category = "NPC|Overlap")
    void OnRangeBeginBP(AActor* OtherActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC|Overlap")
    void OnRangeEndBP(AActor* OtherActor);

    // === Dialogue Flow ===
    UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
    FText GetCurrentDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
    bool NextDialogue();
};
