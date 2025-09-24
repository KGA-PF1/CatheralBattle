#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputCoreTypes.h"
#include "ParryInputProxy.generated.h"

class UParryComponent;

UCLASS()
class CATHERALBATTLE_API AParryInputProxy : public AActor
{
	GENERATED_BODY()

public:
	AParryInputProxy();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") FKey PrimaryKey = EKeys::E;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") FKey SecondaryKey = EKeys::SpaceBar;

protected:
	virtual void BeginPlay() override;

private:
	TWeakObjectPtr<UParryComponent> ParryComp;

	void CacheParryComponent();
	void BindKeys();
	void OnParryPressed();
};
