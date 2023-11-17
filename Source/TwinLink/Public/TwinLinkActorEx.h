#pragma once
class USceneComponent;
class TWINLINK_API TwinLinkActorEx {
public:
    /*
     * @brief: Selfの子をIndicesで指定した番号に従い子孫をたどって取得する
     * SelfのIndices[0]番目の子のIndices[1]番目の子の...Indices[...]番目の子を取得する
     */
    static USceneComponent* GetChild(USceneComponent* Self, const std::initializer_list<int>& Indices);

    static void OnSpawnActor(AActor* Self, const FString& Name);
    static void OnSpawnChildActor(AActor* Parent, AActor* Self, const FString& Name);

    template<class T>
    static auto SpawnChildActor(AActor* Self, const TSubclassOf<T>& Class, const FString& Name) -> T* {
        FActorSpawnParameters Params;
        Params.Owner = Self;
        Params.Name = MakeUniqueObjectName(Self->GetWorld(), Class, FName(Name));
        const auto Ret = Self->GetWorld()->SpawnActor<T>(Class, Params);
        if (!Ret)
            return Ret;
        OnSpawnChildActor(Self, Ret, Name);
        return Ret;
    }

    template<class T>
    static auto SpawnActor(UWorld* World, const TSubclassOf<T>& Class, const FString& Name) -> T* {
        FActorSpawnParameters Params;
        Params.Owner = nullptr;
        Params.Name = MakeUniqueObjectName(World, Class, FName(Name));
        const auto Ret = World->SpawnActor<T>(Class, Params);
        if (!Ret)
            return Ret;
        OnSpawnActor(Ret, Name);
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
        if (!World)
            return nullptr;
        return Cast<T>(UGameplayStatics::GetActorOfClass(World, T::StaticClass()));
    }
};
