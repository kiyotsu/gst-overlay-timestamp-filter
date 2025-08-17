# gst-overlay-timestamp-filter

## 📝 Overview
映像にシステム時刻を重畳するGStreamerのプラグインです。

### 🙂 Before applying filter 
![before_applying_filter](before_applying_filter.png)

### 😄 After applying filter
![after_applying_filter](after_applying_filter.png)


## 🔧 Build
以下のコマンドで`build`ディレクトリ配下に共有ライブラリが生成されます。  
この方法は、ビルドで生成されるファイルがソースコードと混在しないため、推奨されます。

```bash
cmake -B build && cd build && cmake --build .
```

### CMake Arguments
* `-DBUILD_TEST=TRUE`: 単体テストコードをビルドします。 `./build/run_test`
* `-DENABLE_COVERAGE=TRUE`: カバレッジ計測を有効にします。


## 🚀 Running the Sample
付属のサンプルプログラムを使って、動画にタイムスタンプを重畳する機能を試せます。  
サンプルプログラムはDockerを利用する為、ホスト環境を汚すことはありません。

### Build
`docker build`コマンドで、サンプルプログラムを含むDockerイメージをビルドします。
```bash
docker build -t sample-player . --file example/Dockerfile
```

### Run
`docker run`コマンドでコンテナを起動し、動画を再生します。

* 補足: このコマンドは、ローカルのMP4ファイルをコンテナ内で再生し、デスクトップに表示するための設定を含んでいます。`input.mp4`の部分は、お好きな動画ファイル名に置き換えてください。

```bash
docker run -it --rm \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -v $PWD:/video \
    sample-player:latest /video/input.mp4 --text-color="#ff0000" --text-align="center"
```

#### Parameter
サンプルプログラムがサポートするパラメータです。
* **text-color:** テキストカラーをRGBで指定します。
* **text-align:** テキスト位置を指定します。利用可能な値は以下の通りです。
    * `top-left`
    * `top-center`
    * `top-right`
    * `center-left`
    * `center`
    * `center-right`
    * `bottom-left`
    * `bottom-center`
    * `bottom-right`
* **outline-color:** テキストの外枠の色をRGBで指定します。
* **format:** タイムスタンプのフォーマットを指定します。利用可能な書式は以下の通りです。
    * `%Y`: 4桁の年 (例: 2023)
    * `%m`: 2桁の月 (01-12)
    * `%d`: 2桁の日 (01-31)
    * `%H`: 2桁の時 (00-23)
    * `%M`: 2桁の分 (00-59)
    * `%S`: 2桁の秒 (00-59)
    * `%q`: 3桁のミリ秒 (000-999)
    * `%%`: '%'リテラル文字
