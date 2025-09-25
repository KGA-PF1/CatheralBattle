#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleHUDWidget.generated.h"

class UProgressBar; class UTextBlock; class UVerticalBox;
class UHorizontalBox; class UImage;

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

protected:
	virtual void NativeConstruct() override;

private:
	TArray<UImage*> APImages; // 6칸

	void UpdateAPPips(int32 FilledCount); // 0~6

};
