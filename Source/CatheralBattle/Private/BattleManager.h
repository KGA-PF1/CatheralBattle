#pragma once
#include "CoreMinimal.h"
#include "TurnMenuWidget.h"
#include "BattleHUDWidget.h"
#include "GameFramework/Actor.h"
#include "ATBPlayerCharacter.h"
#include "BattleManager.generated.h"

class APlayerCharacter;
class ABoss_Sevarog;
class AParryInputProxy;
class UAnimMontage;
class UTurnMenuWidget;
class UBattleHUDWidget;

enum class EPlayerCommand : uint8;

UENUM(BlueprintType)
enum class ETurn : uint8 { Player, Boss };

/** 턴제 매니저 */
UCLASS()
class CATHERALBATTLE_API ABattleManager : public AActor
{
	GENERATED_BODY()

public:
	ABattleManager();


	UPROPERTY() ATBPlayerCharacter* TBPlayer = nullptr;

	UPROPERTY(EditAnywhere, Category = "Classes")
	TSubclassOf<ATBPlayerCharacter> TBPlayerClass;

	/** 보스 패턴 몽타주 리스트(랜덤 선택, 직전 반복 금지 옵션) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle|Setup")
	TArray<UAnimMontage*> BossPatternMontages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle|Setup")
	bool bDisallowRepeat = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle|Setup")
	bool bAutoStart = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle|Setup")
	AParryInputProxy* ParryProxy = nullptr;

	/** 의존성 주입 */
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void Initialize(APlayerCharacter* InPlayer, ABoss_Sevarog* InBoss, AParryInputProxy* InProxy = nullptr);

	/** 시작/종료 */
	UFUNCTION(BlueprintCallable, Category = "Battle") void StartBattle();
	UFUNCTION(BlueprintCallable, Category = "Battle") void EndBattle();

	/** 플레이어 턴 종료 콜백(UI에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Battle") void NotifyPlayerTurnDone();

	UFUNCTION(BlueprintPure, Category = "Battle") ETurn GetCurrentTurn() const { return CurrentTurn; }

	// UI 클래스(UMG에서 만들고 지정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle|UI")
	TSubclassOf<UTurnMenuWidget> TurnMenuClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle|UI")
	TSubclassOf<UBattleHUDWidget> HUDClass;

	// 플레이어 몽타주(기본 공격/스킬/궁극기)
	UPROPERTY(EditAnywhere, Category = "Battle|PlayerMontage") UAnimMontage* PlayerAttackMontage = nullptr;
	UPROPERTY(EditAnywhere, Category = "Battle|PlayerMontage") UAnimMontage* PlayerSkill1Montage = nullptr;
	UPROPERTY(EditAnywhere, Category = "Battle|PlayerMontage") UAnimMontage* PlayerSkill2Montage = nullptr;
	UPROPERTY(EditAnywhere, Category = "Battle|PlayerMontage") UAnimMontage* PlayerUltMontage = nullptr;

	bool bWaitingForPlayerMontage = false;

	UFUNCTION() void OnPlayerMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void PlayPlayerMontage(UAnimMontage* MontageToPlay);

	// Spawn 앵커(레벨에 놓은 TargetPoint나 빈 Actor를 드롭)
	UPROPERTY(EditAnywhere, Category = "TB|Spawn")
	AActor* TBPlayerSpawnAnchor = nullptr;

	UPROPERTY(EditAnywhere, Category = "TB|Spawn")
	AActor* BossSpawnAnchor = nullptr;

	// 필요시 보스 새로 스폰할 클래스(없으면 기존 보스를 자리만 이동)
	UPROPERTY(EditAnywhere, Category = "TB|Spawn")
	TSubclassOf<ABoss_Sevarog> BossClass;

	// 기존 보스를 앵커 위치로 ‘이동’시킬지 여부
	UPROPERTY(EditAnywhere, Category = "TB|Spawn")
	bool bRelocateExistingBoss = true;


protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY() APlayerCharacter* PlayerRef = nullptr;
	UPROPERTY() ABoss_Sevarog* BossRef = nullptr;

	ETurn CurrentTurn = ETurn::Player;
	bool  bRunning = false;
	int32 LastPatternIdx = -1;

	/** 턴 전환 */
	void EnterPlayerTurn();
	void EnterBossTurn();

	/** 유틸 */
	int32 PickMontageIndex() const;
	void  SetParryEnabled(bool bEnable);
	void  TryAutoWireProxy();

	/** 승패 및 이벤트 */
	void  CheckEnd();

	UFUNCTION() void OnPlayerHpChanged(float NewHp, float MaxHp);
	UFUNCTION() void OnBossPatternFinished();

	// UI
	UPROPERTY() UTurnMenuWidget* TurnMenu = nullptr;
	UPROPERTY() UBattleHUDWidget* HUD = nullptr;

	void ShowPlayerUI(bool bShowMenu);
	void UpdateHUDSnapshot();
	void BindRuntimeSignals();

	UFUNCTION() void HandleMenuConfirm(EPlayerCommand Command);
	UFUNCTION() void OnParryToast();
	UFUNCTION() void OnBossPatternPerfect();
	UFUNCTION() void OnBossDealDmg(float Amt);

	// 플레이어 교체
	UPROPERTY() APawn* OriginalPawn = nullptr;
	FVector OriginalPawnLoc;
	FRotator OriginalPawnRot;

	void SetOriginalPawnVisible(bool bVisible);
};
