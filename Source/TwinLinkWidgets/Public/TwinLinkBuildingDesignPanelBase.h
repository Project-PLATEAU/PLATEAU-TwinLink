// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "TwinLinkBuildingDesignPanelBase.generated.h"

class UTwinLinkBuildingDesignInfo;

/**
 * 設計情報表示機能を提供するウィジェットの基底クラス
 * 閲覧用、編集用で同じ基底クラスを使う　　閲覧用では編集機能を使用しない
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = TwinLink)
class TWINLINKWIDGETS_API UTwinLinkBuildingDesignPanelBase : public UTwinLinkWidgetBase
{
	GENERATED_BODY()

public:
    /**
     * @brief ウィジェットとデータの連携
     * 注意　初期化用のデータはシステムから取得しているので注意
     * @param Info 利用する施設情報
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void Setup(/*UTwinLinkBuildingDesignInfo* Info*/);

    /**
     * @brief データ取得
     * @return
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    UTwinLinkBuildingDesignInfo* GetData() const {
        //check(BuildingDesignInfo.IsValid());  // nullptrの可能性あり
        return BuildingDesignInfo.Get();
    }

    /**
     * @brief 変更リクエストを行う
     * @param ImageFileName
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void RequestEdit(
        const FString& ImageFileName);

    /**
     * @brief 所持しているデータを削除する
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void RequestRemoveCurrentData();

    /**
     * @brief ファイルパスを取得する
     * @return 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FString GetPathToImageFile() const;
    
    /**
     * @brief SrcPathのファイルをコピーしたものをプロジェクトフォルダにインポートする
     * @param SrcPath 
     * @return 失敗したら空文字列
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    FString CopyImageAndImport(const FString& SrcPath) const;

    /**
     * @brief 情報が変更された時に呼ばれる
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void OnChangedInfo();

    /**
     * @brief Setup()直後に呼ばれる
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
    void SetupWBP();

private:
    void BindChangedEvent();

private:
    TWeakObjectPtr<UTwinLinkBuildingDesignInfo> BuildingDesignInfo;

    FDelegateHandle EvOnChangedHnd;
    FDelegateHandle EvOnDeletedHnd;
    FDelegateHandle EvOnAddedBuildingDesignInfoInstanceHnd;
};
