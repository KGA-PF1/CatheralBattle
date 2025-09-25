#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleHUDWidget.generated.h"

UCLASS()
class CATHERALBATTLE_API UBattleHUDWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// 바인딩 위젯(UMG에서 만들고 이름 일치)
	UPROPERTY(meta = (BindWidgetOptional)) class UProgressBar* PlayerHpBar;
	UPROPERTY(meta = (BindWidgetOptional)) class UProgressBar* BossHpBar;
	UPROPERTY(meta = (BindWidgetOptional)) class UProgressBar* UltBar;
	UPROPERTY(meta = (BindWidgetOptional)) class UTextBlock* APText;
	UPROPERTY(meta = (BindWidgetOptional)) class UVerticalBox* ToastBox;

	UFUNCTION(BlueprintCallable) void SetPlayerHP(float Cur, float Max);
	UFUNCTION(BlueprintCallable) void SetBossHP(float Cur, float Max);
	UFUNCTION(BlueprintCallable) void SetUlt(float Cur, float Max);
	UFUNCTION(BlueprintCallable) void SetAP(float AP);
	UFUNCTION(BlueprintCallable) void ShowToast(const FString& Msg, float Life = 1.0f);
};
