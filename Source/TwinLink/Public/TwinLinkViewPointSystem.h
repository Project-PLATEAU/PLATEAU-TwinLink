// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkSubSystemBase.h"

#include "TwinLinkSystemVersionInfo.h"
#include "TwinLinkObservableCollection.h"
#include "TwinLinkViewPointInfoCollection.h"

#include "TwinLinkViewPointSystem.generated.h"

// 視点情報
class UTwinLinkViewPointInfo;

/**
 * 視点情報の管理、適用を行うサブシステム
 */
UCLASS()
class TWINLINK_API UTwinLinkViewPointSystem : public UTwinLinkSubSystemBase
{
	GENERATED_BODY()

public:

    /**
     * @brief 
     * @param Collection 
    */
    virtual void Initialize(FSubsystemCollectionBase& Collection);

    /**
     * @brief 
    */
    virtual void Deinitialize();

public:
    /**
     * @brief 追加可能な視点情報かチェックする
     * @param ViewPointName 
     * @return 
    */
    bool CheckAddableViewPointInfo(const FString ViewPointName);

    /**
     * @brief 視点情報を追加する
     * @param ViewPointName 
    */
    void AddViewPointInfo(const FString ViewPointName);

    /**
     * @brief 視点情報を削除する
     * @param ViewPointInfo 
    */
    void RemoveViewPointInfo(const TWeakObjectPtr<UTwinLinkViewPointInfo> ViewPointInfo);

    /**
     * @brief 視点情報群をエキスポートする
    */
    void ExportViewPointInfo();

    /**
     * @brief 視点情報群をインポートする
    */
    void ImportViewPointInfo();

    /**
     * @brief 視点情報を適応する
     * @param ViewPointInfo 
    */
    void ApplyViewPointInfo(const TWeakObjectPtr<UTwinLinkViewPointInfo> ViewPointInfo);

    /**
     * @brief 視点情報群を保持する通知機能付きのコレクションを取得する
     * @return 
    */
    TWeakObjectPtr<UTwinLinkObservableCollection> GetViewPointCollection() const;

private:
    /** 視点情報機能のバージョン　永続化時に組み込む **/
    const TwinLinkSystemVersionInfo VersionInfo = TwinLinkSystemVersionInfo(0, 0, 0);
    
    /** 視点情報の永続化時のパス **/
    FString Filepath;

    /** 視点情報群 **/
    UPROPERTY()
    TObjectPtr<UTwinLinkViewPointInfoCollection> ViewPointInfoCollection;

private:
    /**
     * @brief 視点情報の適応ターゲットを取得する
     * @return 
    */
    TObjectPtr<ACharacter> GetApplicableTargets();


};
