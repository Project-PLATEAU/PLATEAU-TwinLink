// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkDataObjBase.h"
#include "TwinLinkObservableDataObjBase.generated.h"

// データ更新時のイベント
DECLARE_EVENT(UTwinLinkObservableDataObjBase, FDelOnChangedData);
DECLARE_EVENT(UTwinLinkObservableDataObjBase, FDelOnDeleteData);

/**
 * データの変更を感知できるデータ基底クラス
 * 
 * データの変更を検知する方法
 * 外部のクラスがこのデータの変更を検知したい時は EvOnChangedのイベントを購読する
 * 
 * データの変更を通知する方法
 * このクラスを継承したデータオブジェクトクラスで データの変更後に BroadcastEvOnChanged() を呼ぶ
 */
UCLASS(Abstract, BlueprintType)
class TWINLINK_API UTwinLinkObservableDataObjBase : public UTwinLinkDataObjBase
{
	GENERATED_BODY()

public:
    /** データが変更された **/
    FDelOnChangedData EvOnChanged;

    /** データが削除される(削除される直前) **/
    FDelOnChangedData EvOnDeleted;


protected:
    /**
     * @brief イベントに登録された処理を呼び出す
    */
    void BroadcastEvOnChanged();

};
