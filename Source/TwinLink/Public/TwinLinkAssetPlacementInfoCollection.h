// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkObservableCollection.h"
#include "TwinLinkAssetPlacementInfo.h"
#include "TwinLinkAssetPlacementInfoCollection.generated.h"

//class UTwinLinkAssetPlacementInfo;

/**
 * 通知機能付きのアセット配置情報コレクションクラス
 */
UCLASS(BlueprintType)
class TWINLINK_API UTwinLinkAssetPlacementInfoCollection : public UTwinLinkObservableCollection {
    GENERATED_BODY()

protected:
    virtual void OnAdd(UObject* Obj) override;
    virtual void OnRemove(const TWeakObjectPtr<UObject> Obj) override;
    virtual void OnReset() override;
    virtual bool OnContains(const TWeakObjectPtr<UObject> Obj) override;
    virtual TArray<UObject*> OnGetCollectionRaw() override;

public:
    // 範囲ベース Loop用

    TMap<uint32, TObjectPtr<UTwinLinkAssetPlacementInfo>>::TRangedForConstIterator begin() const {
        return AssetPlacementInfoCollection.begin();
    }
    TMap<uint32, TObjectPtr<UTwinLinkAssetPlacementInfo>>::TRangedForConstIterator end() const {
        return AssetPlacementInfoCollection.end();
    }

    /**
     * @brief 指定のキーの値を更新
     * @param Key
     * @param Obj
    */
    void Update(const uint32 Key, const TWeakObjectPtr<UObject> Obj);

private:
    /** アセット配置情報群 **/
    UPROPERTY()
        TMap<uint32, TObjectPtr<UTwinLinkAssetPlacementInfo>> AssetPlacementInfoCollection;
};
