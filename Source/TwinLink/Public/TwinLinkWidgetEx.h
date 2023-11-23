#pragma once
#include <optional>
class UWidget;
class USceneComponent;
class TWINLINK_API TwinLinkWidgetEx {
public:
    /*
     * @brief : UserWidgetの場合, そのウィジットのトップCanvasまで到達するとGetParentはnullptrを返すが、実際にはさらに親にウィジットがいる場合もある(OuterWidget).
     *        : それを考慮してとってくる
     */
    static UWidget* GetParentIncludeOuter(UWidget* Self);

    /*
     * @brief : 親ウィジット, アウターウィジットを考慮してVisibleを取得する
     */
    static bool IsVisibleIncludeOuter(UWidget* Self);

    /*
     * @brief : Outerを検索していき最初に見つかったUWidgetを返す
     */
    static UWidget* GetOuterWidget(UWidget* Self);
};
