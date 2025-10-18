
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>  // malloc, freeのため
#include <string.h>  // memcpy, memsetのため
#include <complex.h> // 複素数計算のため

int main(int argc, char * argv[]) {
    // --- 1. 入力画像の読み込み ---
    if (argc < 2) {
        printf("画像ファイル名を入力してください。 \n");
        return 1;
    }

    int width, height, channels;
    char *input_file;
    input_file = argv[1];
    unsigned char *input_img = stbi_load(input_file, &width, &height, &channels, 0);

    if (input_img == NULL) {
        printf("画像の読み込みに失敗しました。\n");
        return 1;
    }
    printf("画像サイズ: %d x %d, チャンネル数: %d\n", width, height, channels);

    // --- 2. 出力用の画像領域を確保 ---
    size_t img_size = width * height * channels;
    unsigned char *output_img = malloc(img_size);
    if (output_img == NULL) {
        printf("出力用メモリの確保に失敗しました。\n");
        stbi_image_free(input_img);
        return 1;
    }
    // メモリを0で埋めて、真っ黒な画像にしておく
    memset(output_img, 0, img_size);

    // --- 3-7. 全ピクセルを変換・コピー ---
    // 2重のforループで入力画像の全ピクセルを走査
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // 4. 画像座標(x, y)を複素数zに変換
            //    複素平面の範囲を(-2, -2)から(2, 2)に設定
            double complex z = ((double)x / width * 4.0 - 2.0) +
                               ((double)y / height * 4.0 - 2.0) * I;

            // 5. 複素関数で変換！ ★ここを変えると色々な変換が楽しめる！★
            double complex w = z * z * z; 

            // 6. 結果の複素数wを新しい画像座標(nx, ny)に変換
            int nx = (int)((creal(w) + 2.0) / 4.0 * width);
            int ny = (int)((cimag(w) + 2.0) / 4.0 * height);

            // 7. ピクセルをコピー
            // 新しい座標が画像の範囲内にあるかチェック
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                // (x, y)のピクセルのアドレスを計算
                int src_index = (y * width + x) * channels;
                // (nx, ny)のピクセルのアドレスを計算
                int dest_index = (ny * width + nx) * channels;

                // memcpyでピクセルの色情報(RGB or RGBA)を丸ごとコピー
                memcpy(output_img + dest_index, input_img + src_index, channels);
            }
        }
    }

    printf("複素関数を使って画像を変換しました。\n");

    // --- 8. 完成した画像を保存 ---
    stbi_write_png("output_transform.png", width, height, channels, output_img, 0);
    printf("output_transform.png として画像を保存しました。\n");

    // --- メモリ解放 ---
    stbi_image_free(input_img);
    free(output_img);

    return 0;
}