# 環境構築手順書

# 1 本書について

本書では、TwinLinkシステム（以下「本システム」という。）で使用する外部空間IDデータ連携機能の利用環境構築手順について記載しています。本システムの構成や仕様の詳細については以下も参考にしてください。

[技術検証レポート](https://www.mlit.go.jp/plateau/file/libraries/doc/plateau_tech_doc_0030_ver01.pdf)

# 2 動作環境

本システムの動作環境は以下のとおりです。

| 項目         | 最小動作環境                                                                                                                                                 | 推奨動作環境  |
| ------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------ | ------------- |
| OS           | macOS Monterey 以上                                                                                                                                          | 同左          |
| CPU          | Intel Core i3 以上 または Apple M1 以上                                                                                                                      | Apple M1 以上 |
| ライブラリ   | docker 　が利用できること, Node.js >= 18.0.0                                                                                                                 | 同左          |
| メモリ       | 8GB 以上                                                                                                                                                     | 8GB 以上      |
| ネットワーク | 【データ取得】<br>カメラデータを配信する SFTP サーバーが許可している IP アドレスからリクエストできるネットワーク環境が必要<br><br>【集計データ配信】<br>不要 | 同左          |

# 3 ビルド手順

自身でソースファイルをダウンロードし docker image のビルドを行うことで、実行することができます。\
ソースファイルは
[こちら](https://github.com/eukarya-inc/plateau-spatialid-bim/)
からダウンロード可能です。

サーバーは３つあります。

1. SFTP サーバーからデータを取得、集計して MongoDB に保存するサーバー
2. MongoDB に保存された集計データを配信するサーバー
3. MongoDB サーバー

（1）Docker Image のビルド

## データ取得サーバー Image のビルド

1. Terminal からソースコードの`server`ディレクトに移動する
2. `npm run build && docker build -t batch -f ./Dockerfile.batch .`

## 配信サーバー Image のビルド

1. Terminal からソースコードの`server`ディレクトに移動する
2. `npm run build && docker build -t server -f ./Dockerfile .`

## MongoDB サーバーのビルド

1. Terminal からソースコードの root ディレクトに移動する
2. `docker compose build`

# 4 準備物一覧

アプリケーションを利用するために以下のデータを入手します。
これらを準備したのち環境変数として設定します。
またこれら以外に指定できる[環境変数はこちらのソースコード](plateau-spatialid-bim/blob/develop/server/src/config.ts)を参照ください。

|     | データ種別                                             | 環境変数 key 名  | 値サンプル                                                                                                                                               |
| --- | ------------------------------------------------------ | ---------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------- |
| ①   | SFTP サーバーの IP アドレス                            | SFTP_HOST        | 192.168.0.0                                                                                                                                              |
| ②   | SFTP サーバーの SFTP PORT                              | SFTP_PORT        | 22                                                                                                                                                       |
| ③   | SFTP サーバーに SFTP 通信できる USER_NAME              | SFTP_USERNAME    | test-user                                                                                                                                                |
| ④   | SFTP サーバーに SFTP 通信できる PASSPHRASE             | SFTP_PASSPHRASE  | passphrase                                                                                                                                               |
| ⑤   | SFTP サーバーに SFTP 通信できる秘密鍵(pem 形式)        | SFTP_PRIVATE_KEY |                                                                                                                                                          |
| ⑥   | MongoDB の URI                                         | MONGO_URI        | mongodb://root:example@127.0.0.1:27017                                                                                                                   |
| ⑦   | MongoDB の                                             | MONGO_DB_NAME    | common                                                                                                                                                   |
| ⑧   | カメラデータと位置情報の JSON データ                   | CAMERAS          | [{"id":"1","name":"西新宿 001","lat":35.691149,"lng":139.695587,"height":0}, {"id":"2","name":"西新宿 002","lat":35.691149,"lng":139.695587,"height":0}] |
| ⑨   | 利用するデータベースの種別 (mongo or mock or inMemory) | DATABASE_TYPE    | mongo                                                                                                                                                    |

# 5 サーバーの起動

## MongoDB サーバーの起動

1. Terminal からソースコードの root ディレクトに移動する
2. `docker compose up -d`

## データ取得サーバー の実行

1. Terminal からソースコードの`server`ディレクトに移動する
2. 5 の環境変数を設定する
3. `docker run -it batch`

## 配信サーバー の起動

1. Terminal からソースコードの`server`ディレクトに移動する
2. 5 の環境変数を設定する
3. `docker run -it server`
