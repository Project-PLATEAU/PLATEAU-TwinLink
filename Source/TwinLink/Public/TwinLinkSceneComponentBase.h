// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TwinLinkSceneComponentBase.generated.h"

/**
 * @brief シーンコンポーネントの基底クラス
 * BP_TwinLinkにアタッチするコンポーネントを想定している
*/
UCLASS(Abstract, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TWINLINK_API UTwinLinkSceneComponentBase : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTwinLinkSceneComponentBase();


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

    virtual void BeginDestroy() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * @brief ライントレースを有効化フラグを変更する
     * @param InIsEnableLineTrace 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    static void EnableLineTrace(bool InIsEnableLineTrace);

    /**
     * @brief ライントレースが有効化確認する
     * @return 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    bool IsEnableLineTrace() const;

private:
    static bool bIsEnableLineTrace;

};
