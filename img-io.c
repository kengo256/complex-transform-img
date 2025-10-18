
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>

int main(void) {
    // --- 画像の読み込み ---

    int width;      // 画像の幅
    int height;     // 画像の高さ
    int channels;   // チャンネル数 (例: RGBなら3, RGBAなら4)

    // stbi_load()関数で画像をファイルからメモリに読み込む
    // 引数: (ファイル名, &幅の格納先, &高さの格納先, &チャンネル数の格納先, 要求するチャンネル数)
    // 最後の引数を0にすると、ファイル本来のチャンネル数で読み込む
    unsigned char *img = stbi_load("input.jpg", &width, &height, &channels, 0);

    // 読み込みが失敗したかチェック (imgがNULLなら失敗)
    if (img == NULL) {
        printf("画像の読み込みに失敗しました。\n");
        return 1; // 異常終了
    }

    // 読み込んだ画像情報を表示
    printf("画像サイズ: %d x %d\n", width, height);
    printf("チャンネル数: %d\n", channels);


    // --- 画像の書き出し ---

    // stbi_write_png()関数で、メモリ上のピクセルデータをPNGファイルとして保存する
    // 引数: (出力ファイル名, 幅, 高さ, チャンネル数, データ, 1行あたりのバイト数)
    if (stbi_write_png("output.png", width, height, channels, img, 0) == 0) {
        printf("画像の書き出しに失敗しました。\n");
    } else {
        printf("output.png として画像を保存しました。\n");
    }

    // --- メモリの解放 ---

    // stbi_load()で確保されたメモリは、必ずstbi_image_free()で解放する必要がある
    stbi_image_free(img);

    return 0; // 正常終了
}
