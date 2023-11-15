#include "TwinLinkActorEx.h"

USceneComponent* TwinLinkActorEx::GetChild(USceneComponent* Self,
    const std::initializer_list<int>& Indices) {
    if (!Self)
        return nullptr;
    for (auto I : Indices) {
        auto& Children = Self->GetAttachChildren();
        if (I >= Children.Num())
            return nullptr;
        Self = Children[I];
    }
    return Self;
}

FBox TwinLinkActorEx::GetActorBounds(const AActor* Self, bool bOnlyCollidingComponents,
    bool bIncludeFromChildActors) {
    FVector Center;
    FVector Extent;
    Self->GetActorBounds(bOnlyCollidingComponents, Center, Extent, bIncludeFromChildActors);
    return FBox(Center - Extent, Center + Extent);
}
