// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TwinLinkName3DData.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTwinLinkName3DData : public UInterface
{
	GENERATED_BODY()
};

/**
 * 3D空間上に名前を表示する際に利用するインターフェイス
 * データオブジェクトクラスで多重継承して利用する
 * https://docs.unrealengine.com/5.2/ja/interfaces-in-unreal-engine/
 */
class TWINLINK_API ITwinLinkName3DData
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    /**
     * @brief 名前の取得
     * @return
    */
    virtual FString GetName() const { return TEXT(""); }

    /**
     * @brief 座標の取得
     * @return
    */
    virtual FVector GetPosition() const { return FVector::Zero(); }

};
