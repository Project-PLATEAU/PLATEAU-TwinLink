// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkSceneComponentBase.h"
#include "TwinLinkRoomGetterBase.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TWINLINK_API UTwinLinkRoomGetterBase : public UTwinLinkSceneComponentBase
{
	GENERATED_BODY()
	
public:
    UTwinLinkRoomGetterBase();

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
    void LineTraceToRoom();

private:
    FDelegateHandle EvOnClickedHnd;

};
