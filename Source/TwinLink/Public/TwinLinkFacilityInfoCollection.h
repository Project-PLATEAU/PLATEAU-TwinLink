// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkObservableCollection.h"
#include "TwinLinkFacilityInfoCollection.generated.h"

// コレクション要素
class UTwinLinkFacilityInfo;

/**
 * 施設所法を保持するクラス
 */
UCLASS(BlueprintType)
class TWINLINK_API UTwinLinkFacilityInfoCollection : public UTwinLinkObservableCollection {
    GENERATED_BODY()

protected:
    virtual void OnAdd(UObject* Obj) override;
    virtual void OnRemove(const TWeakObjectPtr<UObject>& Obj) override;
    virtual void OnReset() override;
    virtual bool OnContains(const TWeakObjectPtr<UObject>& Obj) override;
    virtual TArray<UObject*> OnGetCollectionRaw() override;

public:
    // 範囲ベース Loop用

    TMap<uint32, TObjectPtr<UTwinLinkFacilityInfo>>::TRangedForConstIterator begin() const {
        return FacilityInfoCollection.begin();
    }
    TMap<uint32, TObjectPtr<UTwinLinkFacilityInfo>>::TRangedForConstIterator end() const {
        return FacilityInfoCollection.end();
    }

private:
    /** 視点情報群 **/
    UPROPERTY()
    TMap<uint32, TObjectPtr<UTwinLinkFacilityInfo>> FacilityInfoCollection;


};
