// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkFileDialogLib.h"

// WindowsのAPI
#if PLATFORM_WINDOWS
#include "Windows/WindowsSystemIncludes.h"

#include "Runtime/Core/Public/Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include "Runtime/Core/Public/Windows/HideWindowsPlatformTypes.h."
#endif 

// Unreal Engineのデスクトッププラットフォーム関係
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"

// 
#include "TwinLinkCommon.h"

void UTwinLinkFileDialogLib::OpenFileDialog(
    EDialogResult& OutputPin,
    TArray<FString>& OutFilePath,
    const FString& DialogTitle,
    const FString& DefaultPath,
    const FString& DefaultFile,
    const FString& FileTypeString,
    const bool bIsMultiSelect) {

    //ウィンドウハンドルを取得
    void* windowHandle = GetActiveWindow();

    if (windowHandle) {
        IDesktopPlatform* desktopPlatform = FDesktopPlatformModule::Get();
        if (desktopPlatform) {
            //ダイアログを開く
            bool result = desktopPlatform->OpenFileDialog(
                windowHandle,
                DialogTitle,
                DefaultPath,
                DefaultFile,
                FileTypeString,
                (uint32)(bIsMultiSelect ? EFileDialogFlags::Type::Multiple : EFileDialogFlags::Type::None),
                OutFilePath
            );

            if (result) {

                // OpenFileDialog()はエディタビルドだと絶対パス、アプリビルドだと相対パスを返す。
                // 統一するために変換している

                //相対パスを絶対パスに変換
                for (FString& fp : OutFilePath) {
                    fp = FPaths::ConvertRelativePathToFull(fp);
                }

                OutputPin = EDialogResult::Successful;
                return;
            }
        }
    }

    OutputPin = EDialogResult::Cancelled;
}

FString UTwinLinkFileDialogLib::GetFileTypeString(EFileType FileType) {
    const TMap<EFileType, FString> FileTypeStringMap{
        { EFileType::LoadableTextures, TEXT("すべてのテクスチャ|*.bmp;*.dib;*.jpg;*.jpeg;*.jpe;*jfif;*.png")},
        { EFileType::BMP, TEXT("ビットマップ ファイル|*.bmp;*.dib")},
        { EFileType::JPEG, TEXT("JPEG|*.jpg;*.jpeg;*.jpe;*jfif") },
        { EFileType::Png, TEXT("PNG|*.png") },
        { EFileType::All, TEXT("All|*.*") },
    };

    const auto Str = FileTypeStringMap.Find(FileType);
    check(Str);

    return *Str;

}

FString UTwinLinkFileDialogLib::CombineFileTypeString2(const FString& FileTypeStr0, const FString& FileTypeStr1) {
    return FString::Printf(TEXT("%s|%s"), *FileTypeStr0, *FileTypeStr1);
}

FString UTwinLinkFileDialogLib::CombineFileTypeString3(const FString& FileTypeStr0, const FString& FileTypeStr1, const FString& FileTypeStr2) {
    return FString::Printf(TEXT("%s|%s|%s"), *FileTypeStr0, *FileTypeStr1, *FileTypeStr2);
}

FString UTwinLinkFileDialogLib::CombineFileTypeString4(const FString& FileTypeStr0, const FString& FileTypeStr1, const FString& FileTypeStr2, const FString& FileTypeStr3) {
    return FString::Printf(TEXT("%s|%s|%s|%s"), *FileTypeStr0, *FileTypeStr1, *FileTypeStr2, *FileTypeStr3);
}

FString UTwinLinkFileDialogLib::CombineFileTypeString5(const FString& FileTypeStr0, const FString& FileTypeStr1, const FString& FileTypeStr2, const FString& FileTypeStr3, const FString& FileTypeStr4) {
    return FString::Printf(TEXT("%s|%s|%s|%s|%s"), *FileTypeStr0, *FileTypeStr1, *FileTypeStr2, *FileTypeStr3, *FileTypeStr4);
}

bool UTwinLinkFileDialogLib::CopyFile(const FString& Src, const FString& Dest) {
    auto& FileManager = FPlatformFileManager::Get();
    auto& PlatformFile = FileManager.GetPlatformFile();
    bool isSuc = PlatformFile.CopyFile(*Dest, *Src);
    ensure(isSuc);
    if (isSuc == false) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Faild CopyFile(): src %s, dest %s"),
            *Src, *Dest);
        return false;
    }

    return true;
}

bool UTwinLinkFileDialogLib::CreateDirectoryTree(const FString& Directory) {
    auto& FileManager = FPlatformFileManager::Get();
    auto& PlatformFile = FileManager.GetPlatformFile();
    bool isSuc = PlatformFile.CreateDirectoryTree(*Directory);

    ensure(isSuc);
    if (isSuc == false) {
        UE_TWINLINK_LOG(LogTemp, Log, TEXT("Faild CreateDirectoryTree(): dir %s"),
            *Directory);
        return false;
    }

    return true;
}
