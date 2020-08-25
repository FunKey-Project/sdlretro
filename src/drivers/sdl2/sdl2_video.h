#pragma once

#include "video_base.h"

#include <memory>

extern "C" {
typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;
}

namespace drivers {

class sdl2_ttf;

class sdl2_video: public video_base {
public:
    sdl2_video();
    ~sdl2_video() override;
    bool resolution_changed(unsigned width, unsigned height, unsigned pixel_format) override;
    void render(const void *data, unsigned width, unsigned height, size_t pitch) override;
    void *get_framebuffer(unsigned *width, unsigned *height, size_t *pitch, int *format) override;
    bool frame_drawn() override { return drawn; }
    void get_resolution(int &width, int &height) override {
        width = curr_width; height = curr_height;
    }
    void clear() override;
    void flip() override;

    int get_font_size() const override;
    void draw_rectangle(int x, int y, int w, int h) override;
    /* width: 0=fullscreen -1=fullscreen allow wrap
     *        others: negative = allow wrap */
    void draw_text(int x, int y, const char *text, int width, bool shadow) override;
    void get_text_width_and_height(const char *text, uint32_t &w, int &t, int &b) const override;

    void enter_menu() override;
    void leave_menu() override;
    void predraw_menu() override;
    void config_changed() override;

private:
    void do_render();

private:
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr, *background = nullptr;

    uint32_t curr_width = 0, curr_height = 0;
    uint32_t game_pitch = 0, game_width = 0, game_height = 0, game_pixel_format = 0;
    std::array<int, 4> display_rect = {};

    /* ttf[0] is regular font
     * fft[1] is bold font
     * */
    std::shared_ptr<sdl2_ttf> ttf[2];

    /* indicate wheather frame was drawn, for auto frameskip use */
    bool drawn = false;

    /* check renderer with flag SDL_RENDERER_TARGETTEXTURE */
    bool support_render_to_texture = false;
};

}
