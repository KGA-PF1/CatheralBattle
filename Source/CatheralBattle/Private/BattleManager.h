#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleManager.generated.h"

class APlayerCharacter;
class ABoss_Sevarog;
class AParryInputProxy;
class UAnimMontage;

UENUM(BlueprintType)
enum class ETurn : uint8 { Player, Boss };

/** 턴제 매니저 */
UCLASS()
class CATHERALBATTLE_API ABattleManager : public AActor
{
	GENERATED_BODY()

public:
	ABattleManager();

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
};
