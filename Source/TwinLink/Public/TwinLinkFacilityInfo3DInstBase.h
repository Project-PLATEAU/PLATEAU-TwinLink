// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TwinLinkFacilityInfo3DInstBase.generated.h"

// ScrollBoxの要素群を保持するCollection
class UTwinLinkObservableCollection;

/**
 * @brief 3Dの施設名を表示する機能を付与するコンポーネントの基底クラス
 * memo BP_TwinLinkにアタッチしてあるはず
*/
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TWINLINK_API UTwinLinkFacilityInfo3DInstBase : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTwinLinkFacilityInfo3DInstBase();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    /**
     * @brief システム層のコレクションへの監視を開始します。Widget初期化時に呼び出してください。
     * 例 コレクションへ要素追加時に呼び出す処理をバインドする
     * @param Info 利用する施設情報
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void Setup(UTwinLinkObservableCollection* Collection);

    /**
     * @brief システム層のコレクションへの監視を終了します。Widget破棄前に呼び出してください。
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink")
    void Reset();

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



protected:
    /* 表示位置の高さオフセット */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TwinLink", meta = (ClampMin = "0", UIMin = "0", ForceUnits = "cm"))
    float HeightOffset = 0.0f;


private:
    /**
    * @brief システム層が保持するコレクションへの参照
    */
    TWeakObjectPtr<UTwinLinkObservableCollection> ObservableCollection;

    FDelegateHandle OnPostAddElementHnd;
    FDelegateHandle OnPostRemoveElementHnd;
    FDelegateHandle OnPostResetHnd;

		
};
