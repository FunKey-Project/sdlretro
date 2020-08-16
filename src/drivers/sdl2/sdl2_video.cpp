#include "sdl2_video.h"

#include "cfg.h"

#include <SDL.h>

namespace drivers {

sdl2_video::sdl2_video() {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    std::tie(curr_width, curr_height) = g_cfg.get_resolution();

    window = SDL_CreateWindow("SDLRetro", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              curr_width, curr_height, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
}

sdl2_video::~sdl2_video() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

bool sdl2_video::resolution_changed(unsigned width, unsigned height, unsigned pixel_format) {
    game_pixel_format = pixel_format;
    return true;
}

void sdl2_video::render(const void *data, unsigned width, unsigned height, size_t pitch) {
    if (!data) {
        drawn = false;
        return;
    }
    if (skip_frame) {
        drawn = false;
        skip_frame = false;
        return;
    }
    drawn = true;

    uint32_t pitch_in_pixel;
    switch(game_pixel_format) {
    case 0:
        pitch_in_pixel = pitch >> 1;
        break;
    case 1:
        pitch_in_pixel = pitch >> 2;
        break;
    default:
        pitch_in_pixel = pitch >> 1;
        break;
    }
    if (height != game_height || pitch_in_pixel != game_pixel_format) {
        if (texture) SDL_DestroyTexture(texture);
        game_height = height;
        game_pitch = pitch_in_pixel;
        switch (game_pixel_format) {
        case 0:
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB555, SDL_TEXTUREACCESS_STREAMING, game_pitch, game_height);
            break;
        case 1:
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBX8888, SDL_TEXTUREACCESS_STREAMING, game_pitch, game_height);
            break;
        default:
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, game_pitch, game_height);
            break;
        }
    }
    void *pixels;
    int lock_pitch;
    SDL_LockTexture(texture, nullptr, &pixels, &lock_pitch);
    memcpy(pixels, data, pitch * height);
    SDL_Rect rc {0, 0, (int)width, (int)height};
    SDL_UnlockTexture(texture);
    SDL_RenderCopy(renderer, texture, &rc, nullptr);
    SDL_RenderPresent(renderer);
}

void *sdl2_video::get_framebuffer(unsigned int *width, unsigned int *height, size_t *pitch, int *format) {
    return video_base::get_framebuffer(width, height, pitch, format);
}

void sdl2_video::clear() {
    SDL_RenderClear(renderer);
}

void sdl2_video::flip() {
    SDL_RenderPresent(renderer);
}

void sdl2_video::draw_text(int x, int y, const char *text, int width, bool shadow) {

}

uint32_t sdl2_video::get_text_width(const char *text) {
    return 0;
}

void sdl2_video::draw_text_pixel(int x, int y, const char *text, int width, bool shadow) {

}

void sdl2_video::enter_menu() {

}

void sdl2_video::leave_menu() {

}

}
