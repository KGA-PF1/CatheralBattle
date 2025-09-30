#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamagePopupWidget.h"
#include "BattleHUDWidget.generated.h"

class UProgressBar; class UTextBlock; class UVerticalBox;
class UHorizontalBox; class UImage;
class UOverlay;
class UWidgetAnimation;
class UCanvasPanel;          
class UCanvasPanelSlot;      

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

	// ★ 화면에 임의 위젯을 올릴 캔버스(UMG에서 최상단에 CanvasPanel 하나 추가해 이름을 PopupCanvas로!)
	UPROPERTY(meta = (BindWidgetOptional))  UCanvasPanel* PopupCanvas;


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

	// 패링 결과 API
	UFUNCTION(BlueprintCallable) void PlayParrySuccessEffect();
	UFUNCTION(BlueprintCallable) void PlayParryPerfectEffect();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Popup")
	TSubclassOf<class UDamagePopupWidget> PopupWidgetClass;

	// 데미지/게이지 팝업(월드 위치 → 화면 좌표로 변환하여 PopupCanvas에 추가)
	UFUNCTION(BlueprintCallable)
	void ShowDamagePopup(AActor* Target, float Amount);

	UFUNCTION(BlueprintCallable)
	void ShowAPGainPopup(float Amount);

	UFUNCTION(BlueprintCallable)
	void ShowUltGainPopup_HUD(float Amount);

	UPROPERTY(meta = (BindWidgetOptional))
	class UCanvasPanel* APPopupAnchor;

	UPROPERTY(meta = (BindWidgetOptional))
	class UCanvasPanel* UltPopupAnchor;


protected:
	virtual void NativeConstruct() override;

	// 오버레이
	UPROPERTY(meta = (BindWidgetOptional)) UOverlay* Overlay_playerturn = nullptr;
	UPROPERTY(meta = (BindWidgetOptional)) UOverlay* Overlay_bossturn = nullptr;

	// 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* PlayerTurn_In = nullptr;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* PlayerTurn_Out = nullptr;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* BossTurn_In = nullptr;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* BossTurn_Out = nullptr;


	// ★ 패링 결과	
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* Parry_Success = nullptr;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* Parry_Perfect = nullptr;



private:
	TArray<UImage*> APImages; // 6칸

	void UpdateAPPips(int32 FilledCount); // 0~6

};
