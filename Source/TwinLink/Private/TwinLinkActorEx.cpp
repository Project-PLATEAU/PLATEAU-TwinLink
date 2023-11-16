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
#if UE_BUILD_DEVELOPMENT
    Self->SetActorLabel(Name);
#endif
}

void TwinLinkActorEx::OnSpawnChildActor(AActor* Parent, AActor* Self, const FString& Name) {
#if UE_BUILD_DEVELOPMENT
    Self->SetActorLabel(Name);
#endif
    Self->AttachToActor(Parent, FAttachmentTransformRules::KeepWorldTransform);
}