#include "DamagePopupWidget.h"
#include "Components/TextBlock.h"

void UDamagePopupWidget::InitPopup(const FString& InText, const FLinearColor& InColor)
{
    if (PopupText)
    {
        PopupText->SetText(FText::FromString(InText));
        PopupText->SetColorAndOpacity(FSlateColor(InColor));
    }

    if (Anim_Popup)
    {
        PlayAnimation(Anim_Popup, 0.f, 1);
    }
}
