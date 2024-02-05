# 環境構築手順書
# 1 本書について
本書では、PLATEAU Twinlink（以下「本ツール」という。）の環境構築手順について記載しています。本システムの構成や仕様の詳細については[技術検証レポート](https://)も参考にしてください。

# 2 動作環境
本ツールの動作環境は以下のとおりです。

| 項目 | 推奨動作環境 |
| - | - |
| OS | Microsoft Windows 11 |
| CPU | Intel Core i9-12900H 以上 |
| メモリ | 32GB 以上 |

# 3 インストール手順
## Unreal Engine
[Epic Games Store](https://store.epicgames.com/ja/download) からEpic Games Launcher をインストール後、Epic Games Launcher を開く。Unreal Engine（①）→ライブラリ（②）→＋（③）を選択し、Unreal Engine 5.3 をインストールしてください。
![Unreal Engine のインストール](../resources/Setup/Unreal%20Engine%20のインストール.jpg)


インストールされたUnreal Engine を開き、プロジェクトデフォルトとしてブループリントではなくC++を選択してから作成します。
![C++の選択](../resources/Setup/C++の選択.jpg)


## PLATEAU SDKおよびPLATEAU Twinlink
PLATEAU SDK、本ツールのリリースページから各プラグインのzip ファイルをダウンロードしてください。作成したプロジェクトのフォルダを開き、Plugins という名前のフォルダを作成後、各zip ファイルを展開して格納します。
![ファイル格納](../resources/Setup/ファイル格納.jpg)

# 4 ビルド手順
## 3D都市モデルのインポート
PLATEAU SDK から使用したい3D都市モデルをインポートします。インポート方法については[PLATEAU SDK のマニュアル](https://project-plateau.github.io/PLATEAU-SDK-for-Unreal/manual/ImportCityModels.html)を参照してください。

## TwinLink の設定
上部メニューから「PLATEAU TwinLink」を選択すると、設定ウィンドウが開きます。

まずフォーカス対象建築物インポートからLOD4の建築物を以下の手順でインポートします。

1. 参照ボタンを押した後、エクスプローラからCityGMLファイルを選択します。
2. インポートボタンを押します。
3. インポートが完了すると「インポート完了」と表示されます。
![フォーカス対象建築物](../resources/Setup/フォーカス対象建築物のインポート.jpg)

描画設定では、以下の項目の設定が可能です。

- 環境設定
    - 環境設定にチェックを入れることで、時間帯と月を指定できます。
- カリング設定
    - 遠距離のオブジェクトを非表示にしたい場合はこの項目にチェックを入れます。
- LOD1の広範囲地図の読み込み
    - インポートした3D都市モデルより広範囲の地形モデルが追加でインポートされます。

下部の適用ボタンを押すことでチェックが入れられた描画設定が反映されます。
![描画設定](../resources/Setup/描画設定.jpg)


データ生成項目では、3D 都市モデルと管理対象建築物（LOD4）のアクターを選択して、下部の実行ボタンを押すとツールの実行に必要なデータの生成が行われます。

生成が完了すると「データの生成が完了しました。」と表示されます。
![データ生成](../resources/Setup/データ生成.jpg)

管理者PIN 設定では、空欄に数字4 文字を入力し、設定ボタンを押すことで、管理モードに入るためのPIN を設定できます。PLATEAU TwinLink ウィンドウを閉じて再生ボタンを押すと、先ほど設定した内容でシーンを再生できます。
![データ生成](../resources/Setup/データ生成.jpg)


## ビルドアプリの作成
スタンドアロンで実行可能なexeファイルを生成する方法について記載します。

プロジェクト設定からGame Default Map を作成したシーンに設定します。
![ビルドアプリの作成](../resources/Setup/ビルドアプリの作成.jpg)


プラットフォーム→Windows→パッケージ化を選択し、ビルドアプリを作成します。
![パッケージ化](../resources/Setup/パッケージ.jpg)