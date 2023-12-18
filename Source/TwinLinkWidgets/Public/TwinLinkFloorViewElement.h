// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkWidgetBase.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "TwinLinkFloorViewElement.generated.h"

class UTwinLinkFloorViewPanel;

/**
 *　階層表示パネル要素ウィジェット
 */
UCLASS()
class TWINLINKWIDGETS_API UTwinLinkFloorViewElement : public UTwinLinkWidgetBase {
    GENERATED_BODY()

public:
    /**
     * @brief 階層表示パネル要素セットアップ
     * @param Label
     * @param Parent
     * @param CanChangeFloorVisible : フロアの表示/非表示設定切り替えられるかどうか
    */
    void ElementSetup(const FString& Label, TObjectPtr<UTwinLinkFloorViewPanel> Parent, bool CanChangeFloorVisible);
    /**
     * @brief 選択解除ブループリント通知
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
        void OnDeselect();
    /**
     * @brief 選択ブループリント通知
    */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
        void OnSelect();

    /*
     * @brief : フロアの表示非表示設定が更新されたときに呼ばれる(ブループリント側でボタンの状態同期用)
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "TwinLink")
        void OnSetFloorVisible(bool FloorVisible);

    /**
     * @brief このフロアの表示/非表示設定を切り替え(FloorInfoSystemにアクセス)
     * @return 切り替え後にこのフロアが表示なのか
     */
    /*UFUNCTION(BlueprintCallable, Category = "TwinLink")
        bool ChangeFloorVisible();*/

    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        void SetFloorVisible(bool Visible);

    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        bool IsFloorVisible() const;

    UFUNCTION(BlueprintCallable, Category = "TwinLink")
        bool CanChangeFloorVisible() const;

public:
    /** ボタンウィジェット **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UButton> ListButton;
    /** 表示テキストウィジェット **/
    UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTextBlock> FacilityInfoListText;
    /* フロア表示非表示ボタン */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
        TObjectPtr<UButton> ButtonVisible;
private:
    /**
     * @brief デリゲート
    */
    UFUNCTION()
        void OnSelectElement();

    /*
     * @brief : フロア表示/非表示ボタンクリック時
     */
    UFUNCTION()
        void OnClickButtonVisible();

    /** 選択時通知先ウィジェットへの参照 **/
    UPROPERTY()
        TObjectPtr<UTwinLinkFloorViewPanel> ElementRoot;

    /** 対象フロアの表示設定 **/
    UPROPERTY(VisibleAnywhere)
        bool bFloorVisible = true;

    // フロアの非表示設定を変更できるかのフラグ
    UPROPERTY(VisibleAnywhere)
        bool bCanChangeFloorVisible = true;
};
