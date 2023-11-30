// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkBuildingDesignPanelBase.h"

#include "Misc/Guid.h"
#include "TwinLinkFileDialogLib.h"

#include "TwinLinkCommon.h"
#include "TwinLinkPersistentPaths.h"
#include "TwinLinkFloorInfoSystem.h"
#include "TwinLinkBuildingDesignInfo.h"

void UTwinLinkBuildingDesignPanelBase::Setup(/*UTwinLinkBuildingDesignInfo* Info*/) {
    TWeakObjectPtr<UTwinLinkFloorInfoSystem> Sys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFloorInfoSystem>();
    check(Sys.IsValid());
    
    const auto Key = Sys->GetKeyBySelectedFloor();
    TWeakObjectPtr<UTwinLinkBuildingDesignInfo> DataInfo = Sys->FindBuildingDesign(Key);

    // イベントへの追加は一度のみ
    if (EvOnAddedBuildingDesignInfoInstanceHnd.IsValid() == false) {
        EvOnAddedBuildingDesignInfoInstanceHnd = 
            Sys->EvOnAddedBuildingDesignInfoInstance.AddLambda(
                [this]() {
                    Setup();
            });
    }

    // 登録済みのイベントを取り除く
    if (BuildingDesignInfo.IsValid()) {
        check(EvOnChangedHnd.IsValid());
        BuildingDesignInfo->EvOnChanged.Remove(EvOnChangedHnd);
        EvOnChangedHnd.Reset();

        check(EvOnDeletedHnd.IsValid());
        BuildingDesignInfo->EvOnDeleted.Remove(EvOnDeletedHnd);
        EvOnDeletedHnd.Reset();
    }

    BuildingDesignInfo = DataInfo;

    BindChangedEvent();

    SetupWBP();
}

void UTwinLinkBuildingDesignPanelBase::BindChangedEvent() {
    if (BuildingDesignInfo.IsValid()) {
        // 変更された際に更新関数を呼び出す
        EvOnChangedHnd = BuildingDesignInfo->EvOnChanged.AddLambda([this]() {
            check(TwinLinkSubSystemHelper::GetInstance<UTwinLinkFloorInfoSystem>().IsValid());
            OnChangedInfo();
            });

        // 無効になった際に呼び出す
        EvOnDeletedHnd = BuildingDesignInfo->EvOnDeleted.AddLambda([this]() {
            check(TwinLinkSubSystemHelper::GetInstance<UTwinLinkFloorInfoSystem>().IsValid());
            BuildingDesignInfo = nullptr;
            OnChangedInfo();
            });

    }
}

void UTwinLinkBuildingDesignPanelBase::RequestEdit(const FString& ImageFileName) {
    TWeakObjectPtr<UTwinLinkFloorInfoSystem> Sys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFloorInfoSystem>();
    check(Sys.IsValid());
    const auto Key = Sys->GetKeyBySelectedFloor();
    if (Key.IsEmpty()) {
        UE_TWINLINK_C_LOG(LogTemp, Log, TEXT("Not selected floor"));
        return;
    }

    if (BuildingDesignInfo.IsValid() == false) {
        Sys->AddBuildingDesign(Key, ImageFileName);
    }
    else {
        const auto bIsSuc = Sys->EditBuildingDesign(Key, BuildingDesignInfo, ImageFileName);
        if (bIsSuc == false) {
            UE_TWINLINK_C_LOG(LogTemp, Log, TEXT("Failed Edit BuildingDesignInfo"));
        }
    }
}

void UTwinLinkBuildingDesignPanelBase::RequestRemoveCurrentData() {
    TWeakObjectPtr<UTwinLinkFloorInfoSystem> Sys = TwinLinkSubSystemHelper::GetInstance<UTwinLinkFloorInfoSystem>();
    check(Sys.IsValid());
    const auto Key = Sys->GetKeyBySelectedFloor();
    if (Key.IsEmpty()) {
        UE_TWINLINK_C_LOG(LogTemp, Log, TEXT("Not selected floor"));
        return;
    }

    if (Sys->RemoveBuildingDesign(Key, BuildingDesignInfo) == false) {
        UE_TWINLINK_C_LOG(LogTemp, Log, TEXT("Failed Remove BuildingDesignInfo"));
    }
}

FString UTwinLinkBuildingDesignPanelBase::GetPathToImageFile() const {
    if (BuildingDesignInfo.IsValid() == false)
        return FString();
    const auto ImageFileName = BuildingDesignInfo.Get()->GetImageFileName();
    const auto Path = TwinLinkPersistentPaths::CreateBuildingDesignImagePath(ImageFileName);

    
    return Path;
}

FString UTwinLinkBuildingDesignPanelBase::CopyImageAndImport(const FString& SrcPath) const {
    const auto BuildingDesignImageDir = TwinLinkPersistentPaths::CreateBuildingDesignImagePath(TEXT(""));;
    const auto bIsSucCreateDirectory = UTwinLinkFileDialogLib::CreateDirectoryTree(BuildingDesignImageDir);
    if (bIsSucCreateDirectory == false) {
        check(false);
        return FString();
    }

    const auto NewFileName = FGuid::NewGuid().ToString();
    const auto NewFilePath = TwinLinkPersistentPaths::CreateBuildingDesignImagePath(NewFileName);

    const auto bIsSucCopyFile = UTwinLinkFileDialogLib::CopyFile(SrcPath, NewFilePath);
    if (bIsSucCopyFile == false) {
        check(bIsSucCopyFile);
        return FString();
    }
    return NewFilePath;
}
