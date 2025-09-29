#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "TurnMenuWidget.generated.h"

UENUM(BlueprintType)
enum class EPlayerCommand : uint8 { Attack = 0, Skill1 = 1, Skill2 = 2, Ultimate = 3 };

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMenuConfirm, EPlayerCommand, Command);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMenuChanged, int32, NewIndex);

UCLASS()
class CATHERALBATTLE_API UTurnMenuWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable, Category = "TurnMenu") FOnMenuConfirm OnConfirm;
	UPROPERTY(BlueprintAssignable, Category = "TurnMenu") FOnMenuChanged OnChanged;

	// UMG에서 바인딩할 4개 텍스트(또는 버튼) — 디자인에 맞게 이름만 일치
	UPROPERTY(meta = (BindWidgetOptional)) class UTextBlock* OptionAttack;
	UPROPERTY(meta = (BindWidgetOptional)) class UTextBlock* OptionSkill1;
	UPROPERTY(meta = (BindWidgetOptional)) class UTextBlock* OptionSkill2;
	UPROPERTY(meta = (BindWidgetOptional)) class UTextBlock* OptionUltimate;

	// 현재 선택(0~3)
	UPROPERTY(BlueprintReadOnly) int32 SelectedIndex = 0;

	// 외부에서 궁극기 사용 가능/불가 표시
	UFUNCTION(BlueprintCallable) void SetUltimateEnabled(bool bEnabled) { bUltimateEnabled = bEnabled; UpdateVisuals(); }

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Ult_CantUse;

	UFUNCTION(BlueprintCallable)
	void UpdateUltReady(bool bReady);

protected:
	virtual void NativeOnInitialized() override
	{
		bIsFocusable = true;
		SetKeyboardFocus();
		UpdateVisuals();
	}
	virtual FReply NativeOnKeyDown(const FGeometry& InGeo, const FKeyEvent& InKeyEvent) override;

	virtual void NativeConstruct() override;



private:
	bool bUltimateEnabled = true;
	void MoveSelection(int32 Delta);
	void UpdateVisuals();
};
