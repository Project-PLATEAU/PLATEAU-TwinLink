#pragma once

class TwinLinkActorEx {
public:
    template<class T>
    static auto SpawnChildActor(AActor* Self, const TSubclassOf<T>& Class, const TCHAR* Name) -> T* {
        FActorSpawnParameters Params;
        Params.Owner = Self;
        Params.Name = MakeUniqueObjectName(Self->GetWorld(), Class, FName(Name));
        const auto Ret = Self->GetWorld()->SpawnActor<T>(Class, Params);
        if (!Ret)
            return Ret;
        Ret->SetActorLabel(FString(Name));
        Ret->AttachToActor(Self, FAttachmentTransformRules::KeepWorldTransform, Name);
        return Ret;
    }

    template<class T>
    static const T* FindActorInOwner(const AActor* Self, bool bIncludeSelf = false) {
        if (!Self)
            return nullptr;
        auto Actor = bIncludeSelf ? Self : Self->Owner;
        while (Actor) {
            if (auto Ret = Cast<T>(Actor))
                return Ret;
            Actor = Actor->Owner;
        }
        return nullptr;
    }

    template<class T>
    static T* FindActorInParent(AActor* Self, bool bIncludeSelf = false) {
        return const_cast<T*>(FindActorInParent<T>(const_cast<const AActor*>(Self), bIncludeSelf));
    }

    template<class T>
    static T* FindFirstActorInWorld(const UWorld* World) {
        TArray<AActor*> Actors;
        UGameplayStatics::GetAllActorsOfClass(World, T::StaticClass(), Actors);
        ATwinLinkNavSystem* NavSystem = nullptr;
        if (Actors.Num() == 0)
            return nullptr;
        return Cast<T>(Actors[0]);
    }
};
