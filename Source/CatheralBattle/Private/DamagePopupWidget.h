#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamagePopupWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

UCLASS()
class CATHERALBATTLE_API UDamagePopupWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget)) UTextBlock* PopupText;

    UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
    UWidgetAnimation* Anim_Popup;

    UFUNCTION(BlueprintCallable)
    void InitPopup(const FString& InText, const FLinearColor& InColor);
};
