# 環境構築手順書

# 1 本書について

本書では、TwinLinkシステム（以下「本システム」という。）で使用する外部空間IDデータ連携機能の利用環境構築手順について記載しています。本システムの構成や仕様の詳細については以下も参考にしてください。

[技術検証レポート](https://www.mlit.go.jp/plateau/file/libraries/doc/plateau_tech_doc_0030_ver01.pdf)

# 2 動作環境

本システムの動作環境は以下のとおりです。

| 項目         | 最小動作環境                                                                                                                                                 | 推奨動作環境  |
| ------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------ | ------------- |
| OS           | macOS Monterey以上                                                                                                                                          | 同左          |
| CPU          | Intel Core i3以上またはApple M1以上                                                                                                                      | 同左 |
| ライブラリ   | docker及びNode.js 18.0.0以上                                                                                                                 | 同左          |
| メモリ       | 8GB以上                                                                                                                                                     | 同左      |
| ネットワーク | 【データ取得】<br>カメラデータを配信するSFTPサーバーが許可しているIPアドレスからリクエストできるネットワーク環境が必要<br><br>【集計データ配信】<br>不要 | 同左          |

# 3 ビルド手順

自身でソースファイルをダウンロードしdocker imageのビルドを行うことで、実行することができます。\
ソースファイルは[こちら](https://github.com/Project-PLATEAU/PLATEAU-TwinLink/tree/main/plateau-spatialid-bim)をご利用ください。

サーバーは３つあります。

1. SFTPサーバーからデータを取得、集計してMongoDBに保存するサーバー
2. MongoDBに保存された集計データを配信するサーバー
3. MongoDBサーバー

## データ取得サーバーImageのビルド

1. Terminalからソースコードの`server`ディレクトに移動します。
2. `npm run build && docker build -t batch -f ./Dockerfile.batch .`

## 配信サーバーImageのビルド

1. Terminalからソースコードの`server`ディレクトに移動します。
2. `npm run build && docker build -t server -f ./Dockerfile .`

## MongoDBサーバーのビルド

1. Terminalからソースコードのrootディレクトに移動します。
2. `docker compose build`

# 4 準備物一覧

アプリケーションを利用するために以下のデータを入手します。
これらを準備したのち環境変数として設定します。
またこれら以外に指定できる[環境変数はこちらのソースコード](https://github.com/Project-PLATEAU/PLATEAU-TwinLink/tree/main/plateau-spatialid-bim/server/src/config.ts)を参照ください。

|     | データ種別                                             | 環境変数 key 名  | 値サンプル                                                                                                                                               |
| --- | ------------------------------------------------------ | ---------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------- |
| ①   | SFTPサーバーのIPアドレス                            | SFTP_HOST        | 192.168.0.0                                                                                                                                              |
| ②   | SFTPサーバーのSFTP PORT                              | SFTP_PORT        | 22                                                                                                                                                       |
| ③   | SFTPサーバーにSFTP通信できるUSER_NAME              | SFTP_USERNAME    | test-user                                                                                                                                                |
| ④   | SFTPサーバーにSFTP通信できるPASSPHRASE             | SFTP_PASSPHRASE  | passphrase                                                                                                                                               |
| ⑤   | SFTPサーバーにSFTP通信できる秘密鍵(pem形式)        | SFTP_PRIVATE_KEY |                                                                                                                                                          |
| ⑥   | MongoDBのURI                                         | MONGO_URI        | mongodb://root:example@127.0.0.1:27017                                                                                                                   |
| ⑦   | MongoDBのデータベース名                                             | MONGO_DB_NAME    | common                                                                                                                                                   |
| ⑧   | カメラデータと位置情報のJSONデータ                   | CAMERAS          | [{"id":"1","name":"西新宿 001","lat":35.691149,"lng":139.695587,"height":0}, {"id":"2","name":"西新宿 002","lat":35.691149,"lng":139.695587,"height":0}] |
| ⑨   | 利用するデータベースの種別 (mongo/mock/inMemory) | DATABASE_TYPE    | mongo                                                                                                                                                    |

# 5 サーバーの起動

## MongoDBサーバーの起動

1. Terminalからソースコードのrootディレクトに移動します。
2. `docker compose up -d`

## データ取得サーバーの実行

1. Terminalからソースコードの`server`ディレクトに移動します。
2. ⑤の環境変数を設定します。
3. `docker run -it batch`

## 配信サーバーの起動

1. Terminalからソースコードの`server`ディレクトに移動する
2. ⑤の環境変数を設定する
3. `docker run -it server`
