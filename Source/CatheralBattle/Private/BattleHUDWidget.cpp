#include "BattleHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h" 
#include "Blueprint/WidgetTree.h"
#include <Kismet/GameplayStatics.h>
#include "Components/CanvasPanel.h"       
#include "Components/CanvasPanelSlot.h"    
#include "Blueprint/UserWidget.h"
#include "DamagePopupWidget.h"
#include "Blueprint/SlateBlueprintLibrary.h" 
#include "Blueprint/WidgetLayoutLibrary.h"  


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

	// 초기 배너 감추기(애니 시작 전 깜빡임 방지)
	if (Overlay_playerturn) Overlay_playerturn->SetVisibility(ESlateVisibility::Hidden);
	if (Overlay_bossturn)   Overlay_bossturn->SetVisibility(ESlateVisibility::Hidden);

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

void UBattleHUDWidget::ShowPlayerTurn(bool bShow)
{
	if (!Overlay_playerturn || !Overlay_bossturn) return;

	if (bShow)
	{
		// 플레이어 턴 시작 → 자기 배너 In, 보스 배너 Out
		Overlay_playerturn->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (PlayerTurn_In) PlayAnimation(PlayerTurn_In, 0.f, 1);
		if (BossTurn_Out && Overlay_bossturn->IsVisible()) PlayAnimation(BossTurn_Out, 0.f, 1);
	}
}

void UBattleHUDWidget::ShowBossTurn(bool bShow)
{
	if (!Overlay_bossturn || !Overlay_playerturn) return;

	if (bShow)
	{
		// 보스 턴 시작 → 자기 배너 In, 플레이어 배너 Out
		Overlay_bossturn->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (BossTurn_In) PlayAnimation(BossTurn_In, 0.f, 1);
		if (PlayerTurn_Out && Overlay_playerturn->IsVisible()) PlayAnimation(PlayerTurn_Out, 0.f, 1);
	}
}



void UBattleHUDWidget::PlayParrySuccessEffect()
{
	if (Parry_Success) PlayAnimation(Parry_Success, 0.f, 1);
}
void UBattleHUDWidget::PlayParryPerfectEffect()
{
	if (Parry_Perfect) PlayAnimation(Parry_Perfect, 0.f, 1);
}

void UBattleHUDWidget::ShowDamagePopup(AActor* Target, float Amount)
{
	if (!Target || Amount <= 0.f || !PopupCanvas || !PopupWidgetClass) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	FVector WorldLoc = Target->GetActorLocation() + FVector(0, 0, 120);
	FVector2D ScreenPos;
	if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, WorldLoc, ScreenPos, false)) return;

	// ★ Outer는 PlayerController!
	UDamagePopupWidget* W = CreateWidget<UDamagePopupWidget>(PC, PopupWidgetClass);
	if (!W) return;

	W->InitPopup(FString::Printf(TEXT("-%d"), FMath::RoundToInt(Amount)), FLinearColor::Red);

	if (UCanvasPanelSlot* CanvasSlot = PopupCanvas->AddChildToCanvas(W))
	{
		CanvasSlot->SetAutoSize(true);
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetPosition(ScreenPos);
	}
}

void UBattleHUDWidget::ShowAPGainPopup(float Amount)
{
	if (Amount <= 0.f || !PopupWidgetClass) return;
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	UDamagePopupWidget* W = CreateWidget<UDamagePopupWidget>(PC, PopupWidgetClass);
	if (!W) return;

	W->InitPopup(FString::Printf(TEXT("+%d"), FMath::RoundToInt(Amount)), FLinearColor::White);

	// CanvasPanel의 중심 좌표를 화면 좌표로 변환
	FGeometry Geo = APPopupAnchor->GetCachedGeometry();
	FVector2D LocalCenter = Geo.GetLocalSize() * 0.5f;
	FVector2D PixelPos, ViewportPos;
	USlateBlueprintLibrary::LocalToViewport(GetWorld(), Geo, LocalCenter, PixelPos, ViewportPos);

	W->AddToViewport();
	W->SetPositionInViewport(PixelPos, true);
}

void UBattleHUDWidget::ShowUltGainPopup_HUD(float Amount)
{
	if (Amount <= 0.f || !PopupWidgetClass) return;
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	UDamagePopupWidget* W = CreateWidget<UDamagePopupWidget>(PC, PopupWidgetClass);
	if (!W) return;

	W->InitPopup(FString::Printf(TEXT("+%d"), FMath::RoundToInt(Amount)), FLinearColor::Yellow);

	// CanvasPanel의 중심 좌표를 화면 좌표로 변환
	FGeometry Geo = UltPopupAnchor->GetCachedGeometry();
	FVector2D LocalCenter = Geo.GetLocalSize() * 0.5f;
	FVector2D PixelPos, ViewportPos;
	USlateBlueprintLibrary::LocalToViewport(GetWorld(), Geo, LocalCenter, PixelPos, ViewportPos);

	W->AddToViewport();
	W->SetPositionInViewport(PixelPos, true);
}
