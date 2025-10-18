#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <math.h> // floorのため

// 線形補間を行うヘルパー関数
// v0とv1の間をt(0.0〜1.0)の割合で混ぜる
unsigned char lerp(double v0, double v1, double t) {
    return (unsigned char)((1 - t) * v0 + t * v1);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("画像ファイル名を入力してください。 \n");
        return 1;
    }

    int width, height, channels;
    char *input_file;
    input_file = argv[1];
    unsigned char *input_img = stbi_load(input_file, &width, &height, &channels, 0);
    if (input_img == NULL) { /* エラー処理 */ return 1; }

    size_t img_size = width * height * channels;
    unsigned char *output_img = malloc(img_size);
    if (output_img == NULL) { /* エラー処理 */ return 1; }
    
    // --- 逆写像による変換処理 ---
    // 出力画像の全ピクセル(nx, ny)を走査
    for (int ny = 0; ny < height; ny++) {
        for (int nx = 0; nx < width; nx++) {
            // 1. 出力座標(nx, ny)を複素数wに変換
            double complex w = ((double)nx / width * 4.0 - 2.0) +
                               ((double)ny / height * 4.0 - 2.0) * I;

            // 2. 逆関数を使って、wがどのzから来たのかを計算
            //   w = z*z  の逆関数は z = sqrt(w)
            double complex z = csqrt(w);

            // 3. 複素数zを入力画像の座標(sx, sy)に変換 (sはsourceのs)
            double sx = (creal(z) + 2.0) / 4.0 * width;
            double sy = (cimag(z) + 2.0) / 4.0 * height;

            // 出力画像の(nx, ny)に書き込む色を決定
            unsigned char color[4] = {0, 0, 0, 255}; // デフォルトは黒

            // 4. バイリニア補間
            // 元画像の座標(sx, sy)が範囲内にある場合のみ色を計算
            if (sx >= 0 && sx < width -1 && sy >= 0 && sy < height -1) {
                // 周囲4ピクセルの座標
                int x1 = (int)floor(sx);
                int y1 = (int)floor(sy);
                int x2 = x1 + 1;
                int y2 = y1 + 1;

                // 4ピクセル間の距離(0.0-1.0)
                double x_diff = sx - x1;
                double y_diff = sy - y1;

                // 4ピクセルのアドレスを計算
                int p1_idx = (y1 * width + x1) * channels; //左上
                int p2_idx = (y1 * width + x2) * channels; //右上
                int p3_idx = (y2 * width + x1) * channels; //左下
                int p4_idx = (y2 * width + x2) * channels; //右下

                // 各チャンネル(R, G, B)ごとに補間を行う
                for (int i = 0; i < channels; i++) {
                    // 上段の2ピクセルを水平方向に補間
                    double top_val = lerp(input_img[p1_idx + i], input_img[p2_idx + i], x_diff);
                    // 下段の2ピクセルを水平方向に補間
                    double bottom_val = lerp(input_img[p3_idx + i], input_img[p4_idx + i], x_diff);
                    // 上下段の結果を垂直方向に補間
                    color[i] = lerp(top_val, bottom_val, y_diff);
                }
            }

            // 計算した色を出力画像に書き込む
            int dest_index = (ny * width + nx) * channels;
            memcpy(output_img + dest_index, color, channels);
        }
    }
    
    printf("逆写像とバイリニア補間を使って高品質な変換を行いました。\n");
    stbi_write_png("output_inverse.png", width, height, channels, output_img, 0);

    stbi_image_free(input_img);
    free(output_img);
    return 0;
}