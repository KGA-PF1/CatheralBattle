#include "TurnMenuWidget.h"
#include "Components/TextBlock.h"
#include "InputCoreTypes.h"

FReply UTurnMenuWidget::NativeOnKeyDown(const FGeometry& G, const FKeyEvent& E)
{
	const FKey K = E.GetKey();
	if (K == EKeys::Up || K == EKeys::W) { MoveSelection(-1); return FReply::Handled(); }
	if (K == EKeys::Down || K == EKeys::S) { MoveSelection(+1); return FReply::Handled(); }
	if (K == EKeys::Enter || K == EKeys::Virtual_Accept)
	{
		// 궁극기 비활성 시 선택이 3이면 무시
		if (SelectedIndex == 3 && !bUltimateEnabled) return FReply::Handled();
		OnConfirm.Broadcast(static_cast<EPlayerCommand>(SelectedIndex));
		return FReply::Handled();
	}
	// 좌/우 및 기타 입력 무시
	return FReply::Unhandled();
}

void UTurnMenuWidget::MoveSelection(int32 Delta)
{
	const int32 Prev = SelectedIndex;
	SelectedIndex = (SelectedIndex + Delta + 4) % 4;
	if (SelectedIndex == 3 && !bUltimateEnabled) // Ult 못쓰면 다음 항목으로 스킵
	{
		SelectedIndex = (SelectedIndex + Delta + 4) % 4;
	}
	if (SelectedIndex != Prev) { OnChanged.Broadcast(SelectedIndex); UpdateVisuals(); }
}

void UTurnMenuWidget::UpdateVisuals()
{
	auto SetHi = [&](UTextBlock* T, bool bHi, bool bDisabled = false) {
		if (!T) return;
		T->SetOpacity(bDisabled ? 0.35f : 1.f);
		T->SetRenderScale(bHi ? FVector2D(1.1, 1.1) : FVector2D(1.0, 1.0));
		};
	SetHi(OptionAttack, SelectedIndex == 0);
	SetHi(OptionSkill1, SelectedIndex == 1);
	SetHi(OptionSkill2, SelectedIndex == 2);
	SetHi(OptionUltimate, SelectedIndex == 3, !bUltimateEnabled);
}

void UTurnMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	bIsFocusable = true;             // ★ 반드시
	SetKeyboardFocus();              // 화면에 올라오자마자 포커스
}

void UTurnMenuWidget::UpdateUltReady(bool bReady)
{
	if (Ult_CantUse)
	{
		Ult_CantUse->SetOpacity(bReady ? 0.f : 0.5f);
	}
	SetUltimateEnabled(bReady);

}