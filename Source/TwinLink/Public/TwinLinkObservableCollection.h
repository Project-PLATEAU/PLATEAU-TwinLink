// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TwinLinkObservableCollection.generated.h"

DECLARE_EVENT_OneParam(UTwinLinkObservableCollection, FDelOnPostAdd, const TWeakObjectPtr<UObject>, Obj);
DECLARE_EVENT_OneParam(UTwinLinkObservableCollection, FDelOnPostRemove, const TWeakObjectPtr<UObject>, Obj);
DECLARE_EVENT(UTwinLinkObservableCollection, FDelOnPostReset);

/**
 * 通知機能付きのコレクションラッパークラス
 */
UCLASS(Abstract)
class TWINLINK_API UTwinLinkObservableCollection : public UObject {
    GENERATED_BODY()

public:
    /**
     * @brief 追加
     * @param Obj 
    */
    void Add(UObject* Obj);

    /**
     * @brief 削除
     * @param Obj 
    */
    void Remove(const TWeakObjectPtr<UObject> Obj);

    /**
     * @brief 
    */
    void Reset();

    /**
     * @brief 
     * @param Obj 
     * @return 
    */
    bool Contains(const TWeakObjectPtr<UObject> Obj);

    /**
     * @brief 走査用のコレクションを取得する
     * @return 
    */
    TArray<UObject*> GetCollectionRaw();

public:
    /** 要素追加時に実行される　外部から実行しないでください **/
    FDelOnPostAdd EvOnPostAdd;

    /** 要素削除時に実行される　外部から実行しないでください **/
    FDelOnPostRemove EvOnPostRemove;

    /** 要素リセットに実行される　外部から実行しないでください **/
    FDelOnPostReset EvOnPostReset;

protected:
    // 継承時には以下の関数をオーバライドして機能を実装してください

    /**
     * @brief 要素追加時に呼ばれる　内部コレクションに追加する
     * 継承先で継承先のコレクションに要素を追加する処理を実装してください
     * @param Obj 
    */
    virtual void OnAdd(UObject* Obj) {};

    /**
     * @brief 要素削除時に呼ばれる　内部コレクションに追加する
     * 継承先で継承先のコレクションで要素を削除する処理を実装してください
     * @param Obj
    */
    virtual void OnRemove(const TWeakObjectPtr<UObject> Obj) {};

    /**
     * @brief 要素リセットに呼ばれる　内部コレクションに追加する
     * 継承先で継承先のコレクションをリセットする処理を実装してください
     *
    */
    virtual void OnReset() {};

    /**
     * @brief 要素チェックに呼ばれる　内部コレクションに含まれているかチェックする
     * 継承先で継承先のコレクションに要素が含まれているか確認する処理を実装してください
     * @param Obj
     * @return 
    */
    virtual bool OnContains(const TWeakObjectPtr<UObject> Obj) { return false; };

    /**
     * @brief 走査用コレクション取得時に呼ばれる　内部コレクションの走査用のコレクションを提供する
     * 継承先で継承先のコレクションを走査するコレクションを提供する処理を実装してください
     * @return
    */
    virtual TArray<UObject*> OnGetCollectionRaw() { return TArray<UObject*>(); };

private:

};
