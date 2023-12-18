// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkObservableCollection.h"
#include "TwinLinkFloorInfoCollection.generated.h"

// コレクション要素
class UTwinLinkFloorInfo;

/**
 * 施設内情報群を保持するCollectionクラス
 */
UCLASS(BlueprintType)
class TWINLINK_API UTwinLinkFloorInfoCollection : public UTwinLinkObservableCollection {
    GENERATED_BODY()

protected:
    virtual void OnAdd(UObject* Obj) override;
    virtual void OnRemove(const TWeakObjectPtr<UObject>& Obj) override;
    virtual void OnReset() override;
    virtual bool OnContains(const TWeakObjectPtr<UObject>& Obj) override;
    virtual TArray<UObject*> OnGetCollectionRaw() override;

public:
    // 範囲ベース Loop用

    TMap<uint32, TObjectPtr<UTwinLinkFloorInfo>>::TRangedForConstIterator begin() const {
        return FloorInfoCollection.begin();
    }
    TMap<uint32, TObjectPtr<UTwinLinkFloorInfo>>::TRangedForConstIterator end() const {
        return FloorInfoCollection.end();
    }

    UFUNCTION(BlueprintCallable)
        void SetFloorVisible(bool V);

    UFUNCTION(BlueprintCallable)
        bool IsFloorVisible() const;
private:
    /** 視点情報群 **/
    UPROPERTY()
        TMap<uint32, TObjectPtr<UTwinLinkFloorInfo>> FloorInfoCollection;

    /** リストに表示するかどうか **/
    UPROPERTY()
        bool bFloorVisible = true;
};
