// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TwinLinkScrollBoxBase.generated.h"

/** ScrollBox要素 **/
class UTwinLinkScrollBoxElementBase;

// ScrollBoxの要素群を保持するCollection
class UTwinLinkObservableCollection;

/**
 * TwinLinkで使用するスクロールボックスの基底クラス
 * 追加ダイアログを出力する関数アリ
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkScrollBoxBase : public UUserWidget {
    GENERATED_BODY()

public:
    /**
     * @brief TwinLinkのシステムと連携する、繋ぎ合わせる
     * memo システムとウィジェット以外の第三者から呼び出せるようにしたい
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void SetupOnTwinLink();

    /**
     * @brief ウィジェットのデータを設定する
     * @param Collection
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void Setup(UTwinLinkObservableCollection* Collection);

    /**
     * @brief 視点情報追加ダイアログの表示 未実装
     * @param Element 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void RequestDisplayAddDialog();

    /**
     * @brief コレクションに要素が追加された時に呼び出される
     * @param Element
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
        void OnPostAddElement(const UObject* Element);

    /**
     * @brief コレクションの要素が削除された時に呼び出される
     * @param Element
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
        void OnPostRemoveElement(const UObject* Element);

    /**
     * @brief コレクションリセットされた時に呼び出される（要素がクリアされた時）
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
        void OnPostReset();

protected:
    // memo
    // このクラスの継承先から子要素のウィジェットを上手く取得できなかったため
    // 管理ウィジェットのコレクションに対して追加、削除、リセットを行う必要がある
    // FindChild()内では管理ウィジェットから検索している

    /**
     * @brief 子要素をデータから探す
     * @param Elements
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        UTwinLinkScrollBoxElementBase* FindChild(UObject* Elements);

    /**
     * @brief 管理ウィジェットに追加する
     * ウィジェットを追加時にブルーから呼び出す必要
     * @param Widget
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void AddElementWidgets(UTwinLinkScrollBoxElementBase* Widget);

    /**
     * @brief 管理ウィジェットから削除する
     * ウィジェット削除時にブループリントから呼び出す必要
     * @param Widget
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void RemoveElementWidgets(UTwinLinkScrollBoxElementBase* Widget);

    /**
     * @brief 管理ウィジェットをリセットする
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void ResetElementWidgets();

protected:
    /**
     * @brief TwinLinkのシステムと連帯する設定を行う
     * 継承先でこの基底クラスで利用するコレクションを引数に設定してください
     * @param Collection このクラスで利用するUTwinLinkObservableCollectionの出力先
    */
    virtual void OnSetupOnTwinLink(TWeakObjectPtr<UTwinLinkObservableCollection>* OutCollection) const {};

private:
    /**
     * @brief システム層が保持するコレクションへの参照
    */
    TWeakObjectPtr<UTwinLinkObservableCollection> ObservableCollection;

    /**
     * @brief 管理ウィジェット検索用マップ
    */
    TMap<int, UTwinLinkScrollBoxElementBase*> ScrollBoxElementWidgets;

};
