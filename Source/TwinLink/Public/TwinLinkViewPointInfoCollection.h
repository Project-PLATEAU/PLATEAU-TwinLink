// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TwinLinkObservableCollection.h"
#include "TwinLinkViewPointInfoCollection.generated.h"

// 視点情報
class UTwinLinkViewPointInfo;

/**
 * 通知機能を持った視点情報コレクション
 */
UCLASS(BlueprintType)
class TWINLINK_API UTwinLinkViewPointInfoCollection : public UTwinLinkObservableCollection {
    GENERATED_BODY()

protected:
    virtual void OnAdd(UObject* Obj) override;
    virtual void OnRemove(const TWeakObjectPtr<UObject>& Obj) override;
    virtual void OnReset() override;
    virtual bool OnContains(const TWeakObjectPtr<UObject>& Obj) override;
    virtual TArray<UObject*> OnGetCollectionRaw() override;

public:
    // 範囲ベース Loop用

    TMap<uint32, TObjectPtr<UTwinLinkViewPointInfo>>::TRangedForConstIterator begin() const {
        return ViewPointInfoCollection.begin();
    }
    TMap<uint32, TObjectPtr<UTwinLinkViewPointInfo>>::TRangedForConstIterator end() const {
        return ViewPointInfoCollection.end();
    }

private:
    /** 視点情報群 **/
    UPROPERTY()
    TMap<uint32, TObjectPtr<UTwinLinkViewPointInfo>> ViewPointInfoCollection;


};
