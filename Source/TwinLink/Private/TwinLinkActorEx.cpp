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

void TwinLinkActorEx::OnSpawnActor(AActor* Self, const FString& Name) {
#if UE_BUILD_DEVELOPMENT && WITH_EDITOR
    Self->SetActorLabel(Name);
#endif
}

void TwinLinkActorEx::OnSpawnChildActor(AActor* Parent, AActor* Self, const FString& Name) {
#if UE_BUILD_DEVELOPMENT && WITH_EDITOR
    Self->SetActorLabel(Name);
#endif
    Self->AttachToActor(Parent, FAttachmentTransformRules::KeepWorldTransform);
}

FBox TwinLinkActorEx::GetActorBounds(const AActor* Self, bool bOnlyCollidingComponents,
    bool bIncludeFromChildActors) {
    FVector Center;
    FVector Extent;
    Self->GetActorBounds(bOnlyCollidingComponents, Center, Extent, bIncludeFromChildActors);
    return FBox(Center - Extent, Center + Extent);
}
