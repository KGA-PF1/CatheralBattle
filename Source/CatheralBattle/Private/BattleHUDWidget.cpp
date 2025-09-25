#include "BattleHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"     // 토스트 메시지용
#include "Blueprint/WidgetTree.h"

void UBattleHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APImages.Reset();
	if (AP0) APImages.Add(AP0); if (AP1) APImages.Add(AP1); if (AP2) APImages.Add(AP2);
	if (AP3) APImages.Add(AP3); if (AP4) APImages.Add(AP4); if (AP5) APImages.Add(AP5);

	if (APImages.Num() == 0 && APBox) // 이름 안 붙였으면 APBox 자식에서 자동 수집
	{
		const int32 N = APBox->GetChildrenCount();
		for (int32 i = 0; i < N && i < 6; i++)
			if (UImage* Img = Cast<UImage>(APBox->GetChildAt(i))) APImages.Add(Img);
	}
	UpdateAPPips(0);
}

void UBattleHUDWidget::SetPlayerHP(float C, float M) { if (PlayerHpBar && M > 0) PlayerHpBar->SetPercent(C / M); }
void UBattleHUDWidget::SetBossHP(float C, float M) { if (BossHpBar && M > 0)   BossHpBar->SetPercent(C / M); }
void UBattleHUDWidget::SetUlt(float C, float M) { if (UltBar && M > 0)      UltBar->SetPercent(C / M); }

void UBattleHUDWidget::SetAP(float AP)
{
	const int32 PipCount = FMath::Clamp(FMath::RoundToInt(AP), 0, 6);
	UpdateAPPips(PipCount);
}

void UBattleHUDWidget::UpdateAPPips(int32 FilledCount)
{
	for (int32 i = 0; i < APImages.Num() && i < 6; i++)
	{
		if (!APImages[i]) continue;
		UTexture2D* Tex = (i < FilledCount) ? PipFilledTexture : PipEmptyTexture;
		if (Tex) APImages[i]->SetBrushFromTexture(Tex, false);
		APImages[i]->SetRenderScale(i < FilledCount ? FVector2D(1.05f, 1.05f) : FVector2D(1.f, 1.f));
	}
}

void UBattleHUDWidget::ShowToast(const FString& Msg, float Life)
{
	if (!ToastBox) return;
	UTextBlock* T = NewObject<UTextBlock>(this);
	T->SetText(FText::FromString(Msg));
	ToastBox->AddChild(T);
	FTimerHandle H; GetWorld()->GetTimerManager().SetTimer(H, FTimerDelegate::CreateWeakLambda(this, [=]() {
		if (T && T->IsValidLowLevelFast()) { T->RemoveFromParent(); }
		}), Life, false);
}
