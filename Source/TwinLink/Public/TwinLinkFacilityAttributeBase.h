// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TwinLinkFacilityAttributeBase.generated.h"

/**
 * @brief 属性情報コンポーネントの基底クラス
 * システム側にアクセスする
*/
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TWINLINK_API UTwinLinkFacilityAttributeBase : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTwinLinkFacilityAttributeBase();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
    virtual void BeginDestroy() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * @brief ブループリント側のライントレース処理を実行する
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void LineTraceToCityObject();

private:
    FDelegateHandle EvOnClickedHnd;
		
};
