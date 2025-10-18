#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>

int main(void) {
    int width, height, channels;
    unsigned char *img = stbi_load("input.jpg", &width, &height, &channels, 0);

    if (img == NULL) {
        printf("画像の読み込みに失敗しました。\n");
        return 1;
    }

    printf("画像サイズ: %d x %d, チャンネル数: %d\n", width, height, channels);

    // --- 赤い四角形の描画処理 ---

    // 四角形のサイズと位置を定義
    int rect_size = 100; // 100x100ピクセルの四角形
    int rect_x = (width - rect_size) / 2;  // 画像の中央に配置するためのX座標
    int rect_y = (height - rect_size) / 2; // 画像の中央に配置するためのY座標

    // 2重のforループで、四角形の範囲内のピクセルを一つずつ処理する
    for (int y = rect_y; y < rect_y + rect_size; y++) {
        for (int x = rect_x; x < rect_x + rect_size; x++) {
            // ピクセル(x, y)のデータが配列の何番目にあるか計算
            int index = (y * width + x) * channels;

            // 色を赤に設定 (R=255, G=0, B=0)
            img[index] = 255;     // R
            if (channels > 1) {
                img[index + 1] = 0; // G
            }
            if (channels > 2) {
                img[index + 2] = 0; // B
            }
            // 4チャンネル(RGBA)画像の場合、アルファ値(透明度)は不透明(255)にしておく
            if (channels > 3) {
                img[index + 3] = 255; // A
            }
        }
    }

    printf("画像の中央に赤い四角形を描画しました。\n");

    // 変更した画像を保存
    stbi_write_png("output_square.png", width, height, channels, img, 0);
    printf("output_square.png として画像を保存しました。\n");

    // メモリ解放
    stbi_image_free(img);

    return 0;
}