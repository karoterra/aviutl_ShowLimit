# AviUtl プラグイン - 上限確認

[AviUtl](http://spring-fragrance.mints.ne.jp/aviutl/)
拡張編集内部に存在するスクリプト名等のバッファの使用率を確認するプラグインです。
これにより、あとどれくらいスクリプトを追加できるか確認できます。

## 導入方法

1. [Releases](https://github.com/karoterra/aviutl_ShowLimit/releases/)
   最新版の ZIP ファイルをダウンロードしてください。
2. ZIP ファイルを展開し、以下のファイルを AviUtl の `plugins` フォルダに配置してください。
   - `ShowLimit.auf`

## 使い方

AviUtl の「表示」→「上限確認の表示」からウィンドウを表示してください。

### 表の見方

- スクリプト名 ANM
  - アニメーション効果スクリプト名のバッファの状態を表します。
  - スクリプト名1バイトにつき使用量が1増えます。
- スクリプト名 OBJ
  - カスタムオブジェクトスクリプト名のバッファの状態を表します。
  - 基本的には「スクリプト名 ANM」と同様です。
- スクリプト名 SCN
  - シーンチェンジスクリプト名のバッファの状態を表します。
  - 基本的には「スクリプト名 ANM」と同様です。
- スクリプト名 CAM
  - カメラ効果スクリプト名のバッファの状態を表します。
  - 基本的には「スクリプト名 ANM」と同様です。
- スクリプト名 TRA
  - トラックバー変化方法スクリプト名のバッファの状態を表します。
  - 基本的には「スクリプト名 ANM」と同様です。
- 図形名
  - 図形名のバッファの状態を表します。
  - `figure` フォルダの図形ファイルのファイル名1バイトにつき使用量が1増えます。
- トランジション名
  - トランジション名のバッファの状態を表します。
  - `transition` フォルダの画像ファイルのファイル名1バイトにつき使用量が1増えます。
- EXA/EXO
  - エイリアス(exa)、オブジェクトファイル(exo)のバッファの状態を表します。
  - エイリアス、オブジェクトファイルのファイル名1バイトにつき使用量が1増えます。
- exedit extension
  - 拡張編集の拡張子とメディアオブジェクトの種類の関連付け設定のバッファ状態を表します。
  - `exedit.ini` の `[extension]` セクションの内容によって使用量が増えます。
- 入力プラグイン
  - 入力プラグイン1個につき使用量が1増えます。
- 出力プラグイン
  - 出力プラグイン1個につき使用量が1増えます。
- フィルタプラグイン
  - フィルタプラグイン1個につき使用量が1増えます。
- 色変換プラグイン
  - 色変換プラグイン1個につき使用量が1増えます。
- 言語拡張リソースプラグイン
  - 言語拡張リソースプラグイン1個につき使用量が1増えます。
- add_menu_item
  - フィルタプラグインによってメニュー項目が1個追加されるごとに使用量が1増えます。

### プラグイン情報の出力

AviUtl本体と導入しているプラグインの情報をクリップボード、またはテキストファイルに出力できます。

### スクリプト情報の出力

導入している拡張編集スクリプト一覧をクリップボード、またはテキストファイルに出力できます。

## License

このソフトウェアは MIT ライセンスのもとで公開されます。
詳細は [LICENSE](LICENSE) を参照してください。

使用したライブラリ等については [CREDITS](CREDITS.md) を参照してください。

## Change Log

更新履歴は [CHANGELOG](CHANGELOG.md) を参照してください。

## for Developers

このソフトウェアは [aviutl_exedit_sdk v1](https://github.com/ePi5131/aviutl_exedit_sdk) を使用しています。
ただし、ビルドの際は `AviUtl::OutputPlugin` の `name2`、`filefilter2`、`information2` を
256 から 260 に変更してください。
