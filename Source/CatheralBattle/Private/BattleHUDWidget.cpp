#include "BattleHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void UBattleHUDWidget::SetPlayerHP(float C, float M) { if (PlayerHpBar && M > 0) PlayerHpBar->SetPercent(C / M); }
void UBattleHUDWidget::SetBossHP(float C, float M) { if (BossHpBar && M > 0) BossHpBar->SetPercent(C / M); }
void UBattleHUDWidget::SetUlt(float C, float M) { if (UltBar && M > 0) UltBar->SetPercent(C / M); }
void UBattleHUDWidget::SetAP(float AP) { if (APText) APText->SetText(FText::FromString(FString::Printf(TEXT("AP: %.0f"), AP))); }

void UBattleHUDWidget::ShowToast(const FString& Msg, float Life)
{
	if (!ToastBox) return;
	UTextBlock* T = NewObject<UTextBlock>(this);
	T->SetText(FText::FromString(Msg));
	ToastBox->AddChildToVerticalBox(T);
	FTimerHandle H; GetWorld()->GetTimerManager().SetTimer(H, FTimerDelegate::CreateWeakLambda(this, [=]() {
		if (T && T->IsValidLowLevelFast()) { T->RemoveFromParent(); }
		}), Life, false);
}
