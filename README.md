# complex-transform-img
## プログラム概要
このプログラムは、任意のPNG・JPEG形式の画像ファイルを、
複素関数を用いて様々な平面図形に変換することを目的に作成した。

また、このプログラムは画像ファイルを読み込むためにstdライブラリを、
変換の様子を描画するためにSDL2フレームワークを使用している。
stbライブラリについては、ヘッダーファイルをリポジトリ内に含んでいるためインストールの必要はない。

ただし、**SDL2ライブラリについては各自でインストールする必要**があり。

## SDL2フレームワークを以下のコマンドでインストール
```
sudo apt-get install libsdl2-dev libsdl2-gfx-dev libsdl2-image-de
```
## 以下のコマンドでコンパイル
```
gcc main-transform.c -o main-transform $(sdl2-config --cflags --libs) -lm -fopenmp
```
## 使用方法
### 1.プログラム起動
`./main-transform　画像ファイル名 `
### 2.Enterキーで順写像の変換を開始
### 3.Enterキーで逆写像の変換を開始

