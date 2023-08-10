// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "TwinLinkAssetPlacementPanel.generated.h"

// コレクションへの参照
class UTwinLinkObservableCollection;

/**
 * アセット配置を行うパネルウィジェットの基底クラス
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkAssetPlacementPanel : public UTwinLinkWidgetBase {
    GENERATED_BODY()

public:
    /**
     * @brief TwinLinkのシステムと連携する、繋ぎ合わせる
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void SetupOnTwinLink();
    
protected:
    /**
     * @brief アセット配置情報を追加する
     * @param PlacementPresetID
     * @param Position
     * @param RotationEuler
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void AddAssetPlacement(const int PlacementPresetID, const FVector& PlacementPosition, const FVector& PlacementRotationEuler);

private:
    /** システム層が保持するコレクションへの参照 **/
    TWeakObjectPtr<UTwinLinkObservableCollection> ObservableCollection;
};
