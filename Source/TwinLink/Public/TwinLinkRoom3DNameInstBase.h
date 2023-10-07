// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkSceneComponentBase.h"
#include "TwinLinkRoom3DNameInstBase.generated.h"

// コレクション
class UTwinLinkObservableCollection;

/**
 * 3D名表示用ウィジェットのインスタンス管理用クラス
 * BP_TwinLinkにアタッチすることを想定して作成
 */
UCLASS(Abstract, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TWINLINK_API UTwinLinkRoom3DNameInstBase : public UTwinLinkSceneComponentBase
{
	GENERATED_BODY()
	
public:
    /**
     * @brief セットアップ
     * 同期するコレクションを設定する
     * @param Collection 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void Setup(UTwinLinkObservableCollection* Collection);

    /**
     * @brief セットアップの内容を取り消す 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void Reset();

    /**
     * @brief コレクションを取得する
     * 最初の同期用　何度も使用しないこと
     * @return 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    TArray<UObject*> GetCollectionRaw();

    /**
     * @brief 要素が追加されたときに呼び出される
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnPostAddElement(const UObject* Element);

    /**
     * @brief 要素が削除されたときに呼び出される
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnPostRemoveElement(const UObject* Element);

    /**
     * @brief 全要素がクリアされたときに呼び出される
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnPostReset();
  
private:
    /**
    * @brief システム層が保持するコレクションへの参照
    */
    TWeakObjectPtr<UTwinLinkObservableCollection> ObservableCollection;

    FDelegateHandle OnPostAddElementHnd;
    FDelegateHandle OnPostRemoveElementHnd;
    FDelegateHandle OnPostResetHnd;

};
