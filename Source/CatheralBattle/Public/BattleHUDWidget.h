#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleHUDWidget.generated.h"

class UProgressBar; class UVerticalBox; class UHorizontalBox; class UImage;

UCLASS()
class CATHERALBATTLE_API UBattleHUDWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// 상시 HUD
	UPROPERTY(meta = (BindWidgetOptional)) UProgressBar* PlayerHpBar;
	UPROPERTY(meta = (BindWidgetOptional)) UProgressBar* BossHpBar;
	UPROPERTY(meta = (BindWidgetOptional)) UProgressBar* UltBar;
	UPROPERTY(meta = (BindWidgetOptional)) UVerticalBox* ToastBox;

	// ★ AP 핍 컨테이너(수평 6칸)
	UPROPERTY(meta = (BindWidget)) UHorizontalBox* APBox;

	// (선택) 이름 지정해놨다면 자동 바인딩됨. 없으면 APBox 자식으로 채움
	UPROPERTY(meta = (BindWidgetOptional)) UImage* AP0;
	UPROPERTY(meta = (BindWidgetOptional)) UImage* AP1;
	UPROPERTY(meta = (BindWidgetOptional)) UImage* AP2;
	UPROPERTY(meta = (BindWidgetOptional)) UImage* AP3;
	UPROPERTY(meta = (BindWidgetOptional)) UImage* AP4;
	UPROPERTY(meta = (BindWidgetOptional)) UImage* AP5;

	// ★ 빈/채움 텍스처 지정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AP") TObjectPtr<UTexture2D> PipEmptyTexture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AP") TObjectPtr<UTexture2D> PipFilledTexture;

	// HUD API
	UFUNCTION(BlueprintCallable) void SetPlayerHP(float Cur, float Max);
	UFUNCTION(BlueprintCallable) void SetBossHP(float Cur, float Max);
	UFUNCTION(BlueprintCallable) void SetUlt(float Cur, float Max);
	UFUNCTION(BlueprintCallable) void SetAP(float AP);
	UFUNCTION(BlueprintCallable) void ShowToast(const FString& Msg, float Life = 1.0f);

protected:
	virtual void NativeConstruct() override;

private:
	TArray<UImage*> APImages; // 6칸 모음
	void UpdateAPPips(int32 FilledCount); // 0~6
};
