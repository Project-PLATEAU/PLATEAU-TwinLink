// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TwinLinkFileDialogLib.generated.h"


/*
* @summary
ファイルダイアログ、ディレクトリダイアログの表示
* ファイルダイアログで使用するデータを扱うための補助関数などが実装されている
*
* @how_to_use　
それぞれの関数を参照
*
* @sample なし
*
* @relation
*
* @implementation
* 内部でWindowsのAPIを使用して実装しています。
* パスの形式を統一させるため選択したファイルの相対パスは絶対パスに変換している。
* (本来は開発ビルドは絶対パス、製品ビルドは相対パスを返す。)
*
* @problem
* Windowsビルド以外通らない。
* 選択したファイルの絶対パスしか返せない。
*/

/**
 * @brief ダイアログの結果
 * ノードのピンの拡張で使用
*/
UENUM()
enum class EDialogResult : uint8 {
    Successful, Cancelled
};

/**
 * @brief ファイルタイプを示す enum
 * 要素を追加、削除する際はGetFileTypeString()内部で利用しているマップを修正してください
 *
*/
UENUM(BlueprintType)
enum class EFileType : uint8 {
    LoadableTextures,
    BMP,
    JPEG,
    Png,
    Shp,
    All,
    DownloadImageNodeLoadableTextures,
};
ENUM_CLASS_FLAGS(EFileType);

/**
 * ファイルダイアログを表示に関わる機能を実装した関数ライブラリ
 */
UCLASS()
class TWINLINK_API UTwinLinkFileDialogLib : public UBlueprintFunctionLibrary {
    GENERATED_BODY()

public:

    // 参考先
    // https://qiita.com/Naotsun/items/2f8a94e92df5a41d95f3

    /**
     * @brief ダイアログを開く (BluePrint用)
     * 子のウィンドウが起動する形になるためフルスクリーンモードの場合、
     * 起動したウィンドウが正常に表示されない。
     *
     * memo FileTypeはenumのビットフラグでは実装出来なかった。
     * UENUM(BlueprintType) だとuint8しか対応していないため拡張していくと直ぐに表現域を超えてしまう。
     *
     * @param OutputPin
     * @param OutFilePath 選択されたファイルの絶対パス
     * @param DialogTitle
     * @param DefaultPath
     * @param DefaultFile
     * @param FileType 表示|拡張子|表示|拡張子|...    ex Png(*.png)|*.png|Jpeg(*.jpeg;*.JPEG)|*.jpeg;*.JPEG
     * @param bIsMultiSelect 複数選択可能か
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink OpenFileDialog", meta = (ExpandEnumAsExecs = "OutputPin"))
    static void OpenFileDialog(
        EDialogResult& OutputPin,
        TArray<FString>& OutFilePath,
        const FString& DialogTitle = TEXT("Open File Dialog"),
        const FString& DefaultPath = TEXT(""),
        const FString& DefaultFile = TEXT(""),
        const FString& FileType = TEXT("All (*.*)|*.*"),
        const bool bIsMultiSelect = false
    );

    /**
     * @brief ダイアログを開く (BluePrint用)
     * 子のウィンドウが起動する形になるためフルスクリーンモードの場合、
     * 起動したウィンドウが正常に表示されない。
     *
     * memo FileTypeはenumのビットフラグでは実装出来なかった。
     * UENUM(BlueprintType) だとuint8しか対応していないため拡張していくと直ぐに表現域を超えてしまう。
     *
     * @param OutputPin
     * @param OutFilePath 選択されたファイルの絶対パス
     * @param DialogTitle
     * @param DefaultPath
     * @param DefaultFile
     * @param FileType 表示|拡張子|表示|拡張子|...    ex Png(*.png)|*.png|Jpeg(*.jpeg;*.JPEG)|*.jpeg;*.JPEG
     * @param bIsMultiSelect 複数選択可能か
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink OpenFileDialog", meta = (ExpandEnumAsExecs = "OutputPin"))
    static void SaveFileDialog(
        EDialogResult& OutputPin,
        TArray<FString>& OutFilePath,
        const FString& DialogTitle = TEXT("Save File Dialog"),
        const FString& DefaultPath = TEXT(""),
        const FString& DefaultFile = TEXT(""),
        const FString& FileType = TEXT("All (*.*)|*.*"),
        const bool bIsMultiSelect = false
    );

    // OpenDirectoryDialog
    //https://docs.unrealengine.com/5.2/en-US/API/Developer/DesktopPlatform/IDesktopPlatform/OpenDirectoryDialog/
    UFUNCTION(BlueprintCallable, Category = "TwinLink OpenFileDialog", meta = (ExpandEnumAsExecs = "OutputPin"))
    static void OpenDirectoryDialog(EDialogResult& OutputPin, FString& OutFolderName, const FString& DialogTitle, const FString& DefaultPath);

    /**
     * @brief ファイルタイプを表現する文字列を取得する
     * @param FileType
     * @return
    */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TwinLink OpenFileDialog")
    static FString GetFileTypeString(EFileType FileType);

    /**
     * @brief ファイルタイプ文字列を結合する
     * @param FileTypeStr...
     * @return
    */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TwinLink OpenFileDialog")
    static FString CombineFileTypeString2(const FString& FileTypeStr0, const FString& FileTypeStr1);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TwinLink OpenFileDialog")
    static FString CombineFileTypeString3(const FString& FileTypeStr0, const FString& FileTypeStr1, const FString& FileTypeStr2);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TwinLink OpenFileDialog")
    static FString CombineFileTypeString4(const FString& FileTypeStr0, const FString& FileTypeStr1, const FString& FileTypeStr2, const FString& FileTypeStr3);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TwinLink OpenFileDialog")
    static FString CombineFileTypeString5(const FString& FileTypeStr0, const FString& FileTypeStr1, const FString& FileTypeStr2, const FString& FileTypeStr3, const FString& FileTypeStr4);

    UFUNCTION(BlueprintCallable, Category = "TwinLink OpenFileDialog")
    static bool CopyFile(const FString& Src, const FString& Dest);

    /**
     * @brief 親ディレクトリを含むディレクトリを作成し、
     * ディレクトリが作成されているか既に存在する場合は true を返します。
     * @param Directory 
     * @return 
    */
    UFUNCTION(BlueprintCallable, Category = "TwinLink OpenFileDialog")
    static bool CreateDirectoryTree(const FString& Directory);

    UFUNCTION(BlueprintCallable, Category = "TwinLink OpenFileDialog")
    static FString GetFilenameOnDisk(const FString& Filename);

private:

};
