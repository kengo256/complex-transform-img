#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <math.h> 
#include <SDL2/SDL.h>

// 逆写像で使う関数 (今回は z*z で試します)
double complex f_inv(double complex w) {
    return csqrt(w);
}

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
    
    // 最初は真っ黒な画像にしておく
    memset(output_img, 0, img_size);

    // --- 2. SDLの初期化とウィンドウ作成 ---
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL初期化エラー: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window *win = SDL_CreateWindow("リアルタイム画像変換", 
                                     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                     width, height, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture *tex = SDL_CreateTexture(ren, (channels == 4) ? SDL_PIXELFORMAT_RGBA32 : SDL_PIXELFORMAT_RGB24, 
                                       SDL_TEXTUREACCESS_STREAMING, width, height);

    // --- 3. メインループ ---
    int running = 1;
    int current_row = 0; // 現在どこまで計算したかを記録する変数

    while (running) {
        // イベント処理 (ウィンドウのxボタンが押されたかなど)
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        // --- 画像変換処理 (1フレームに数行ずつ進める) ---
        if (current_row < height) {
            int rows_per_frame = 1; // 1フレームあたりに計算する行数 (この値で速度調整)
            for (int i = 0; i < rows_per_frame && current_row < height; i++) {
                int ny = current_row;
                for (int nx = 0; nx < width; nx++) {
                    // (ここから下は inverse_transform.c とほぼ同じロジック)
                    double complex w = ((double)nx / width * 4.0 - 2.0) + ((double)ny / height * 4.0 - 2.0) * I;
                    double complex z = f_inv(w);
                    double sx = (creal(z) + 2.0) / 4.0 * width;
                    double sy = (cimag(z) + 2.0) / 4.0 * height;

                    unsigned char color[4] = {0, 0, 0, 255};
                    if (sx >= 0 && sx < width - 1 && sy >= 0 && sy < height - 1) {
                        int x1 = (int)sx, y1 = (int)sy;
                        double x_diff = sx - x1, y_diff = sy - y1;
                        for (int ch = 0; ch < channels; ch++) {
                            double v1 = input_img[(y1*width+x1)*channels+ch];
                            double v2 = input_img[(y1*width+x1+1)*channels+ch];
                            double v3 = input_img[((y1+1)*width+x1)*channels+ch];
                            double v4 = input_img[((y1+1)*width+x1+1)*channels+ch];
                            double top = lerp(v1, v2, x_diff);
                            double bot = lerp(v3, v4, x_diff);
                            color[ch] = lerp(top, bot, y_diff);
                        }
                    }
                    memcpy(output_img + (ny * width + nx) * channels, color, channels);
                }
                current_row++;
            }
        }

        // --- 描画処理 ---
        SDL_UpdateTexture(tex, NULL, output_img, width * channels); // ピクセルデータをテクスチャにコピー
        SDL_RenderClear(ren);                                     // 画面をクリア
        SDL_RenderCopy(ren, tex, NULL, NULL);                     // テクスチャを画面に描画
        SDL_RenderPresent(ren);                                   // 描画内容を実際に表示
    }

    // --- 4. 終了処理 ---
    stbi_image_free(input_img);
    free(output_img);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}