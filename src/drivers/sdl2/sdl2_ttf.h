#pragma once

#include "ttf_font.h"

extern "C" {
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;
}

namespace drivers {

class sdl2_ttf: public ttf_font {
public:
    inline explicit sdl2_ttf(SDL_Renderer *r): renderer(r) {}
    ~sdl2_ttf() override;

protected:
    const font_data *make_cache(uint16_t) override;

private:
    SDL_Renderer *renderer;
    std::vector<SDL_Texture*> textures;
};

}