#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <SDL2/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// プログラムの状態を定義する
typedef enum {
    STATE_INIT_FORWARD,    // 順写像の準備段階
    STATE_FORWARD_MAPPING, // 順写像での変換中
    STATE_CLEANUP_FORWARD, // 順写像の後片付け
    STATE_INIT_WAIT,       // Enterキー入力待ちの準備段階
    STATE_WAIT_FOR_ENTER,  // Enterキー入力待ち
    STATE_INVERSE_MAPPING, // 逆写像での変換中
    STATE_DONE             // 完成、静止画表示
} ProgramState;



// -----------------------------------------------

// 複素関数 f(z) = z^2
double complex f(double complex z) {
     return z * z;
    }

// 導関数 f'(z) = 2z
double complex df(double complex z) {
    return 2 * z;
}

// 逆関数 (ニュートン法)
double complex f_inv(double complex w) {
    double complex z = w;
     for (int i = 0; i < 20; i++) {
        double complex f_z = f(z);
        double complex df_z = df(z);
            
        // ゼロ除算を避ける
        if (cabs(df_z) < 1e-6) {
            break;
        }

        // ニュートン法の更新式: z_new = z - (f(z)-w) / f'(z)
        z = z - (f_z - w) / df_z;
     }
     return z;
    }

// -----------------------------------------------



// 線形補間
unsigned char lerp(double v0, double v1, double t) {
    return (unsigned char)((1 - t) * v0 + t * v1);
}

int main(int argc, char* argv[]) {
    // --- 0. 基本的な準備 ---
    if (argc < 2) {
        printf("画像ファイル名入力してください\n");
        return 1;
    }

    char *input_file = argv[1];
    int width, height, channels;
    unsigned char *original_img = stbi_load(input_file, &width, &height, &channels, 0);
    
    if (!original_img) { 
        printf("画像読み込みエラー\n"); 
        return -1;
    }

    size_t img_size = width * height * channels;
    unsigned char *source_work_img = malloc(img_size);   // 左画面用（黒く塗りつぶしていく）
    unsigned char *holey_dest_img = malloc(img_size); // 穴あき画像用
    unsigned char *final_img = malloc(img_size);         // 最終画像用

    if (!source_work_img || !holey_dest_img || !final_img) {
        printf("メモリ確保エラー\n"); 
        return -1;
    }
    
    memcpy(source_work_img, original_img, img_size); // 作業用イメージをコピー
    memset(holey_dest_img, 0, img_size);             // 穴あき画像を黒で初期化
    memcpy(final_img, holey_dest_img, img_size);   // 最終画像も最初は穴あき画像

    SDL_Init(SDL_INIT_VIDEO);

    // ウィンドウ、レンダラー、テクスチャのポインタを準備
    SDL_Window *win_src = NULL, *win_dest = NULL, *win_main = NULL;
    SDL_Renderer *ren_src = NULL, *ren_dest = NULL, *ren_main = NULL;
    SDL_Texture *tex_src = NULL, *tex_dest = NULL, *tex_main = NULL;
    
    int pixel_format = (channels == 4) ? SDL_PIXELFORMAT_RGBA32 : SDL_PIXELFORMAT_RGB24;

    // --- 1. メインループ ---
    ProgramState currentState = STATE_INIT_FORWARD;
    int running = 1;
    int forward_progress = 0;
    int inverse_row = 0;

    while (running) {
        // イベント処理
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            if (event.type == SDL_KEYDOWN) {
                if (currentState == STATE_WAIT_FOR_ENTER && event.key.keysym.sym == SDLK_RETURN) {
                    currentState = STATE_INVERSE_MAPPING;
                }
            }
        }

        // --- 2. 状態ごとの処理 (State Machine) ---
        switch (currentState) {
            case STATE_INIT_FORWARD:
                // 左右のウィンドウを作成
                win_src = SDL_CreateWindow("元画像", 0, 0, width, height, 0);
                win_dest = SDL_CreateWindow("変換後（順写像）", 600 , 100, width, height, 0);
                ren_src = SDL_CreateRenderer(win_src, -1, SDL_RENDERER_ACCELERATED);
                ren_dest = SDL_CreateRenderer(win_dest, -1, SDL_RENDERER_ACCELERATED);
                tex_src = SDL_CreateTexture(ren_src, pixel_format, SDL_TEXTUREACCESS_STREAMING, width, height);
                tex_dest = SDL_CreateTexture(ren_dest, pixel_format, SDL_TEXTUREACCESS_STREAMING, width, height);
                currentState = STATE_FORWARD_MAPPING;
                break;

            case STATE_FORWARD_MAPPING:
                // 順写像を少しずつ進める
                int pixels_per_frame = width * 5; // 速度調整
                for (int i = 0; i < pixels_per_frame && forward_progress < width * height; i++) {
                    int x = forward_progress % width;
                    int y = forward_progress / width;
                    
                    double complex z = ((double) x / width * 4 - 2) + ((double) y / height * 4 - 2) * I;
                    double complex w = f(z);
                    int nx = (int) ((creal(w) + 2) / 4 * width);
                    int ny = (int) ((cimag(w) + 2) / 4 * height);

                    int src_idx = (y * width + x) * channels;
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        int dest_idx = (ny * width + nx) * channels;
                        memcpy(holey_dest_img + dest_idx, source_work_img + src_idx, channels);
                    }
                    memset(source_work_img + src_idx, 0, channels); // 元画像のピクセルを黒くする
                    forward_progress++;
                }
                if (forward_progress >= width * height) {
                    currentState = STATE_CLEANUP_FORWARD;
                }
                break;

            case STATE_CLEANUP_FORWARD:
                // 左右のウィンドウを破棄
                SDL_DestroyTexture(tex_src);
                SDL_DestroyTexture(tex_dest);
                SDL_DestroyRenderer(ren_src);
                SDL_DestroyRenderer(ren_dest);
                SDL_DestroyWindow(win_src);
                SDL_DestroyWindow(win_dest);
                win_src = win_dest = NULL; // ポインタをNULLにしておくのが安全
                currentState = STATE_INIT_WAIT;
                break;

            case STATE_INIT_WAIT:
                // 中央のウィンドウを作成
                win_main = SDL_CreateWindow("Enterを押して修復", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
                ren_main = SDL_CreateRenderer(win_main, -1, SDL_RENDERER_ACCELERATED);
                tex_main = SDL_CreateTexture(ren_main, pixel_format, SDL_TEXTUREACCESS_STREAMING, width, height);
                memcpy(final_img, holey_dest_img, img_size); // 最終画像を穴あき画像で初期化
                currentState = STATE_WAIT_FOR_ENTER;
                break;

            case STATE_WAIT_FOR_ENTER:
                // 何もせずキー入力を待つ
                break;

            case STATE_INVERSE_MAPPING:
                SDL_SetWindowTitle(win_main, "修復中...");
                // 逆写像を少しずつ進める
                int rows_per_frame = 5; // 速度調整
                for (int i = 0; i < rows_per_frame && inverse_row < height; i++) {
                    int ny = inverse_row;
                    for (int nx = 0; nx < width; nx++) {
                        double complex w = ((double) nx / width * 4 - 2) + ((double) ny / height * 4 - 2) * I;
                        double complex z = f_inv(w);
                        double sx = (creal(z) + 2) / 4 * width, sy = (cimag(z) + 2) / 4 * height;
                        
                        unsigned char color[] = {0, 0, 0, 255};
                        if (sx >= 0 && sx < width - 1 && sy >= 0 && sy < height - 1) {
                            // バイリニア補間
                            int x1 = (int) sx, y1 = (int) sy;
                            double xd = sx - x1, yd = sy - y1;
                            for(int c = 0; c < channels; c++) {
                                double p1 = original_img[(y1 * width + x1) * channels + c];
                                double p2 = original_img[(y1 * width + x1 + 1) * channels + c];
                                double p3 = original_img[((y1 + 1) * width + x1) * channels + c];
                                double p4 = original_img[((y1 + 1) * width + x1 + 1) * channels + c];
                                double top = lerp(p1, p2, xd), bot = lerp(p3 ,p4 ,xd);
                                color[c] = lerp(top, bot, yd);
                            }
                        }
                        memcpy(final_img + (ny * width + nx) * channels, color, channels);
                    }
                    inverse_row++;
                }
                if (inverse_row >= height) {
                    currentState = STATE_DONE;
                    SDL_SetWindowTitle(win_main, "変換完了！");
                }
                break;

            case STATE_DONE:
                // 何もせず静止画を表示
                break;
        }

        // --- 3. 描画 ---
        if (currentState <= STATE_FORWARD_MAPPING) {
            SDL_UpdateTexture(tex_src, NULL, source_work_img, width * channels);
            SDL_RenderClear(ren_src);
            SDL_RenderCopy(ren_src, tex_src, NULL, NULL);
            SDL_RenderPresent(ren_src);

            SDL_UpdateTexture(tex_dest, NULL, holey_dest_img, width * channels);
            SDL_RenderClear(ren_dest);
            SDL_RenderCopy(ren_dest, tex_dest, NULL, NULL);
            SDL_RenderPresent(ren_dest);
        } else if (currentState >= STATE_WAIT_FOR_ENTER) {
            SDL_UpdateTexture(tex_main, NULL, (currentState == STATE_INVERSE_MAPPING || currentState == STATE_DONE) ? final_img : holey_dest_img, width * channels);
            SDL_RenderClear(ren_main);
            SDL_RenderCopy(ren_main, tex_main, NULL, NULL);
            SDL_RenderPresent(ren_main);
        }
        SDL_Delay(16); // 負荷軽減 & アニメーション速度調整
    }

    // --- 4. 終了処理 ---
    stbi_image_free(original_img);
    free(source_work_img);
    free(holey_dest_img);
    free(final_img);

    // まだ破棄されていない可能性のあるリソースを安全に破棄
    if (win_src) { 
        SDL_DestroyTexture(tex_src);
        SDL_DestroyRenderer(ren_src);
        SDL_DestroyWindow(win_src);
    }
    if (win_dest) { 
        SDL_DestroyTexture(tex_dest);
        SDL_DestroyRenderer(ren_dest);
        SDL_DestroyWindow(win_dest);
    }
    if (win_main) { 
        SDL_DestroyTexture(tex_main);
        SDL_DestroyRenderer(ren_main);
        SDL_DestroyWindow(win_main);
    }
    
    SDL_Quit();
    return 0;
}