#include "Misc/TwinLinkWidgetEx.h"

#include "Components/PanelWidget.h"
#include "Components/Widget.h"
#include "Components/WidgetSwitcher.h"

UWidget* TwinLinkWidgetEx::GetParentIncludeOuter(UWidget* Self) {
    if (!Self)
        return nullptr;
    auto W = Self;
    UWidget* Parent = Self->GetParent();
    // https://forums.unrealengine.com/t/get-parent-userwidget-of-widget/476221
    // User作成のWidgetの場合トップのキャンパスパネルに到達した場合, さらに親にWidgetがいてもParentはnullptrを返す仕様になっている模様
    // その場合、Outerを見ていきUWidgetがあれば、さらに親のUserWidgetが自分を持っていたことになる
    // トップレベルになったとき
    if (Parent == nullptr) {
        // 親UserWidgetをチェックする
        Parent = GetOuterWidget(W);
    }
    return Parent;
}

bool TwinLinkWidgetEx::IsVisibleIncludeOuter(UWidget* Self) {
    if (!Self)
        return false;
    auto W = Self;
    while (W != nullptr) {
        if (W->IsVisible() == false)
            return false;

        const auto Parent = GetParentIncludeOuter(W);
        // WidgetSwitcherの非アクティブな子は表示されないのにIsVisibleはfalseにならない
        // そのため、ActiveWidgetかどうかで判断する
        if (const auto Switcher = Cast<UWidgetSwitcher>(Parent)) {
            if (Switcher->GetActiveWidget() != W)
                return false;
        }
        W = Parent;
    }
    return true;
}

UWidget* TwinLinkWidgetEx::GetOuterWidget(UWidget* Self) {
    // 親UserWidgetをチェックする
    UObject* Outer = Self;
    while (Outer) {
        Outer = Outer->GetOuter();
        // Outerが存在しない時はそこで打ち切り
        if (Outer == nullptr)
            break;
        // アウターがいるけどそれがUWidgetじゃない場合もある
        // ただし、その場合もさらに上のアウターがUWidgetの場合もあるのでそれも見る必要がある
        if (const auto OuterWidget = Cast<UWidget>(Outer)) {
            return OuterWidget;
        }
    }
    return nullptr;
}
