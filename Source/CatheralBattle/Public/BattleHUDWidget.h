#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleHUDWidget.generated.h"

class UProgressBar; class UTextBlock; class UVerticalBox;
class UHorizontalBox; class UImage;
class UOverlay;
class UWidgetAnimation;

UCLASS()
class CATHERALBATTLE_API UBattleHUDWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// 바인딩 위젯
	UPROPERTY(meta = (BindWidgetOptional)) class UProgressBar* PlayerHpBar;
	UPROPERTY(meta = (BindWidgetOptional)) class UProgressBar* BossHpBar;
	UPROPERTY(meta = (BindWidgetOptional)) class UProgressBar* UltBar;
	UPROPERTY(meta = (BindWidgetOptional)) class UTextBlock* APText;
	UPROPERTY(meta = (BindWidgetOptional)) class UVerticalBox* ToastBox;

	// AP 핍 UI 바인딩
	UPROPERTY(meta = (BindWidget)) UHorizontalBox* APBox;
	UPROPERTY(meta = (BindWidgetOptional)) UImage* AP0;
	UPROPERTY(meta = (BindWidgetOptional)) UImage* AP1;
	UPROPERTY(meta = (BindWidgetOptional)) UImage* AP2;
	UPROPERTY(meta = (BindWidgetOptional)) UImage* AP3;
	UPROPERTY(meta = (BindWidgetOptional)) UImage* AP4;
	UPROPERTY(meta = (BindWidgetOptional)) UImage* AP5;

	// 편집 가능 브러시(빈/채움)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AP")
	TObjectPtr<UTexture2D> PipEmptyTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AP")
	TObjectPtr<UTexture2D> PipFilledTexture;


	UFUNCTION(BlueprintCallable) void SetPlayerHP(float Cur, float Max);
	UFUNCTION(BlueprintCallable) void SetBossHP(float Cur, float Max);
	UFUNCTION(BlueprintCallable) void SetUlt(float Cur, float Max);
	UFUNCTION(BlueprintCallable) void SetAP(float AP);
	UFUNCTION(BlueprintCallable) void ShowToast(const FString& Msg, float Life = 1.0f);

	// 턴 배너 API
	UFUNCTION(BlueprintCallable) void ShowPlayerTurn(bool bShow);
	UFUNCTION(BlueprintCallable) void ShowBossTurn(bool bShow);

protected:
	virtual void NativeConstruct() override;

	// ★ 오버레이 바인딩
	UPROPERTY(meta = (BindWidgetOptional)) UOverlay* Overlay_playerturn = nullptr;
	UPROPERTY(meta = (BindWidgetOptional)) UOverlay* Overlay_bossturn = nullptr;

	// ★ 애니메이션 바인딩(UMG 이름과 동일)
	UPROPERTY(meta = (BindWidgetAnimOptional)) UWidgetAnimation* PlayerTurn_In = nullptr;
	UPROPERTY(meta = (BindWidgetAnimOptional)) UWidgetAnimation* PlayerTurn_Out = nullptr;
	UPROPERTY(meta = (BindWidgetAnimOptional)) UWidgetAnimation* BossTurn_In = nullptr;
	UPROPERTY(meta = (BindWidgetAnimOptional)) UWidgetAnimation* BossTurn_Out = nullptr;


	// ★ 패링 결과	
	UPROPERTY(meta = (BindWidgetAnimOptional)) class UWidgetAnimation* Parry_Success = nullptr;
	UPROPERTY(meta = (BindWidgetAnimOptional)) class UWidgetAnimation* Parry_Perfect = nullptr;

	UFUNCTION(BlueprintCallable) void PlayParrySuccessEffect();
	UFUNCTION(BlueprintCallable) void PlayParryPerfectEffect();

private:
	TArray<UImage*> APImages; // 6칸

	void UpdateAPPips(int32 FilledCount); // 0~6

};
