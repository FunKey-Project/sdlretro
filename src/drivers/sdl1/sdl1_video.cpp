#include "sdl1_video.h"

#include "sdl1_ttf.h"

#include "cfg.h"

#include "util.h"

#include <SDL.h>
#include <unistd.h>

namespace drivers {

const int sdl_video_flags = SDL_SWSURFACE |
#ifdef SDL_TRIPLEBUF
    SDL_TRIPLEBUF
#else
    SDL_DOUBLEBUF
#endif
    ;

sdl1_video::sdl1_video() {
    SDL_ShowCursor(SDL_DISABLE);
    //printf("in %s l.%d\n", __func__, __LINE__);
    g_cfg.get_resolution(curr_width, curr_height);
    curr_pixel_format = 2;
    
    //printf("in %s l.%d\n", __func__, __LINE__);
    hw_screen = SDL_SetVideoMode(DEFAULT_WIDTH, DEFAULT_HEIGHT, 16, sdl_video_flags);
    if (hw_screen == NULL) {
        printf("Error in %s: %s\n", __func__, SDL_GetError());
        exit(1);
    }
    sdl1_reset_screen_surface(curr_width, curr_height, 16);
    
    //SDL_LockSurface(screen);
    screen_ptr = screen->pixels;
    //printf("in %s l.%d\n", __func__, __LINE__);

    const std::string font_path_regular(g_cfg.get_data_dir() + PATH_SEPARATOR_CHAR + "fonts" + PATH_SEPARATOR_CHAR + "regular.ttf");
    if (util::file_exists(font_path_regular)) {
        ttf[0] = std::make_shared<sdl1_ttf>();
        ttf[0]->init(16, 0);
        ttf[0]->add(font_path_regular, 0);
    }
    else{
        printf("In %s, font regular \"%s\" not found\n", __func__, font_path_regular.c_str());
    }
    const std::string font_path_bold(g_cfg.get_data_dir() + PATH_SEPARATOR_CHAR + "fonts" + PATH_SEPARATOR_CHAR + "regular.ttf");
    if (util::file_exists(font_path_bold)) {
        ttf[1] = std::make_shared<sdl1_ttf>();
        ttf[1]->init(16, 0);
        ttf[1]->add(font_path_bold, 0);
    }
    else{
        printf("In %s, font bold \"%s\" not found\n", __func__, font_path_regular.c_str());
    }
    //printf("in %s l.%d\n", __func__, __LINE__);
}

sdl1_video::~sdl1_video() {
    //SDL_UnlockSurface(screen);

    if(screen)
        SDL_FreeSurface(screen);
}

void sdl1_video::sdl1_reset_screen_surface(int width, int height, int bpp){
    if(screen)
        SDL_FreeSurface(screen);
    screen = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, bpp, 0, 0, 0, 0);    
}

bool sdl1_video::game_resolution_changed(int width, int height, int max_width, int max_height, unsigned pixel_format) {
    if (g_cfg.get_scaling_mode() == 0) {
        //SDL_UnlockSurface(screen);
        usleep(10000);
        bool bpp_changed=(curr_pixel_format != pixel_format);
        curr_pixel_format = pixel_format;
        unsigned bpp = pixel_format == 1 ? 32 : 16;

        if(bpp_changed)
            hw_screen = SDL_SetVideoMode(DEFAULT_WIDTH, DEFAULT_HEIGHT, bpp, sdl_video_flags);

        if (width != 0 && height != 0) {
            curr_width = (int)width;
            curr_height = (int)height;
            auto scale = force_scale == 0 ? g_cfg.get_scale() : force_scale;

    //printf("in %s l.%d\n", __func__, __LINE__);
            //screen = SDL_SetVideoMode(width * scale, height * scale, bpp, sdl_video_flags);
            sdl1_reset_screen_surface(width * scale, height * scale, bpp);
            if (screen == NULL) {
                printf("%s\n", SDL_GetError());
                exit(1);
            }
    //printf("in %s l.%d\n", __func__, __LINE__);
        } else {
            g_cfg.get_resolution(curr_width, curr_height);
            //screen = SDL_SetVideoMode(curr_width, curr_height, bpp, sdl_video_flags);
            sdl1_reset_screen_surface(curr_width, curr_height, bpp);
        }
    //printf("in %s l.%d\n", __func__, __LINE__);
        //SDL_LockSurface(screen);
    //printf("in %s l.%d\n", __func__, __LINE__);
        screen_ptr = screen->pixels;
    } else {
        curr_width = (int)width;
        curr_height = (int)height;
    }
    return true;
}

void sdl1_video::render(const void *data, int width, int height, size_t pitch) {
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

    if (curr_width != width || curr_height != height) {
        game_resolution_changed(width, height, 0, 0, curr_pixel_format);
    }
    int h = static_cast<int>(height);
    auto scale = g_cfg.get_scale();

    unsigned bpp = curr_pixel_format == 1 ? 32 : 16;
    if (scale == 1) {
        auto *pixels = static_cast<uint8_t *>(screen_ptr);
        const auto *input = static_cast<const uint8_t *>(data);
        int output_pitch = screen->pitch;
        if (output_pitch == pitch) {
            memcpy(pixels, input, h * pitch);
        } else {
            int line_bytes = width*(bpp >> 3);
            for (; h; h--) {
                memcpy(pixels, input, line_bytes);
                pixels += output_pitch;
                input += pitch;
            }
        }
    } else {
    #define CODE_WITH_TYPE(TYPE) \
        auto *pixels = static_cast<TYPE*>(screen_ptr); \
        const auto *input = static_cast<const TYPE*>(data); \
        int output_pitch = screen->pitch / sizeof(TYPE); \
        auto s = scale; \
        auto subp = pitch / sizeof(TYPE) - width; \
        auto subd = output_pitch - s; \
        output_pitch *= scale - 1; \
        for (; h; h--) { \
            for (unsigned z = width; z; z--) { \
                auto pix = *input++; \
                auto *out = pixels; \
                for (int j = s; j; j--) { \
                    for (int i = s; i; i--) { \
                        *out++ = pix; \
                    } \
                    out += subd; \
                } \
                pixels += s; \
            } \
            pixels += output_pitch; \
            input += subp; \
        }
        if (bpp == 32) {
            CODE_WITH_TYPE(uint32_t)
        } else {
            CODE_WITH_TYPE(uint16_t)
        }
    #undef CODE_WITH_TYPE
    }
    if (!messages.empty()) {
        uint32_t lh = get_font_size() + 2;
        uint32_t y = (curr_height - 5 - (messages.size() - 1) * lh) * scale;
        for (auto &m: messages) {
            draw_text(5, y, m.first.c_str(), 0, true);
            y += lh;
        }
    }
}

/// Nearest neighboor optimized with possible out of screen coordinates (for cropping)
void sdl1_video::scale_NN_AllowOutOfScreen(SDL_Surface *src_surface, SDL_Surface *dst_surface, int new_w, int new_h){

    /// Sanity check
    if(src_surface->format->BytesPerPixel != dst_surface->format->BytesPerPixel){
        printf("Error in %s, src_surface bpp: %d != dst_surface bpp: %d", __func__, 
            src_surface->format->BytesPerPixel, dst_surface->format->BytesPerPixel);
        return;
    }

    int BytesPerPixel=src_surface->format->BytesPerPixel;
    int w1=src_surface->w;
    //int h1=src_surface->h;
    int w2=new_w;
    int h2=new_h;
    int x_ratio = (int)((src_surface->w<<16)/w2);
    int y_ratio = (int)((src_surface->h<<16)/h2);
    int x2, y2 ;

    /// --- Compute padding for centering when out of bounds ---
    int y_padding = (RES_HW_SCREEN_VERTICAL-new_h)/2;
    int x_padding = 0;
    if(w2>RES_HW_SCREEN_HORIZONTAL){
        x_padding = (w2-RES_HW_SCREEN_HORIZONTAL)/2 + 1;
    }
    int x_padding_ratio = x_padding*w1/w2;
    //printf("src_surface->h=%d, h2=%d\n", src_surface->h, h2);

    for (int i=0;i<h2;i++)
    {
        if(i>=RES_HW_SCREEN_VERTICAL){
              continue;
        }

        uint8_t* t = (uint8_t*)(dst_surface->pixels) + ((i+y_padding) * ((w2>RES_HW_SCREEN_HORIZONTAL)?RES_HW_SCREEN_HORIZONTAL:w2))*BytesPerPixel;
        y2 = ((i*y_ratio)>>16);
        uint8_t* p = (uint8_t*)(src_surface->pixels) + (y2*w1 + x_padding_ratio)*BytesPerPixel;
        int rat = 0;
        for (int j=0;j<w2;j++)
        {
            if(j>=RES_HW_SCREEN_HORIZONTAL){
               continue;
            }
            x2 = (rat>>16);
            //*t++ = p[x2];
            memcpy(t, &p[x2*BytesPerPixel], BytesPerPixel);
            t+=BytesPerPixel;
            rat += x_ratio;
        }
    }
}

void sdl1_video::frame_render() {
    if (drawn) {
        flip();
    }
}

void *sdl1_video::get_framebuffer(unsigned *width, unsigned *height, size_t *pitch, int *format) {
    if (!screen) return nullptr;
    *width = screen->w;
    *height = screen->h;
    *pitch = screen->pitch;
    *format = 2;
    return screen_ptr;
}

void sdl1_video::clear() {
    memset(screen_ptr, 0, screen->pitch * screen->h);
}

void sdl1_video::flip() {
    //SDL_UnlockSurface(screen);

    /// Stretched AR
    scale_NN_AllowOutOfScreen(screen, hw_screen, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    
    SDL_Flip(hw_screen);
    //SDL_LockSurface(screen);
    screen_ptr = screen->pixels;
}

int sdl1_video::get_font_size() const {
    if (ttf[0]) {
        return ttf[0]->get_font_size();
    } else {
#ifdef FUNKEY
        return 8;
#else
        return 16;
#endif
    }
}

void sdl1_video::set_draw_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    draw_color[0] = r;
    draw_color[1] = g;
    draw_color[2] = b;
    draw_color[3] = a;
}

void sdl1_video::draw_rectangle(int x, int y, int w, int h) {
    auto bytespp = screen->format->BytesPerPixel;
    uint32_t pixel_color = SDL_MapRGB(screen->format, draw_color[0], draw_color[1], draw_color[2]);
    uint8_t *ptr = (uint8_t*)screen_ptr + screen->pitch * y + x * bytespp;
    int rx = x + w;
    int by = y + h;
    for (int cx = x; cx < rx; ++cx) {
        memcpy(ptr, &pixel_color, bytespp);
        ptr += bytespp;
    }
    ptr = (uint8_t*)screen_ptr + screen->pitch * y + x * bytespp;
    for (int cy = y + 1; cy <= by; ++cy) {
        memcpy(ptr, &pixel_color, bytespp);
        ptr += screen->pitch;
    }
    ptr = (uint8_t*)screen_ptr + screen->pitch * by + x * bytespp;
    for (int cx = x; cx < rx; ++cx) {
        memcpy(ptr, &pixel_color, bytespp);
        ptr += bytespp;
    }
    ptr = (uint8_t*)screen_ptr + screen->pitch * y + rx * bytespp;
    for (int cy = y; cy <= by; ++cy) {
        memcpy(ptr, &pixel_color, bytespp);
        ptr += screen->pitch;
    }
}

void sdl1_video::fill_rectangle(int x, int y, int w, int h) {
    auto bytespp = screen->format->BytesPerPixel;
    uint32_t pixel_color = SDL_MapRGB(screen->format, draw_color[0], draw_color[1], draw_color[2]);
    uint8_t *ptr = (uint8_t*)screen_ptr + screen->pitch * y + x * bytespp;
    int rx = x + w;
    int by = y + h;
    size_t lp = screen->pitch - bytespp * w;
    for (int cy = y; cy < by; ++cy) {
        for (int cx = x; cx < rx; ++cx) {
            memcpy(ptr, &pixel_color, bytespp);
            ptr += bytespp;
        }
        ptr += lp;
    }
}

void sdl1_video::draw_text(int x, int y, const char *text, int width, bool shadow) {
    if (ttf[0]) {
        //printf("ttf[0]->render \"%s\" at {%d, %d}, width=%d, shadown=%d\n", text, x, y, width, shadow);
        ttf[0]->render(screen, x, y, text, width, shadow);
    } else {
        //printf("draw_text_pixel \"%s\" at {%d, %d}, width=%d, shadown=%d\n", text, x, y, width, shadow);
        draw_text_pixel(x, y, text, width, shadow);
    }
}

#include "bmfont.inl"

inline const font_data_t &get_pixel_font_data(uint8_t c) {
#ifdef FUNKEY
    return font_small_data[c];
#else
    return font_big_data[c];
#endif
}

void sdl1_video::get_text_width_and_height(const char *text, int &w, int &t, int &b) const {
    w = 0;
    t = 255;
    b = -255;
    if (ttf[0]) {
        while (*text != 0) {
            uint32_t ch = util::utf8_to_ucs4(text);
            if (ch == 0 || ch > 0xFFFFu) continue;
            uint8_t width;
            int8_t tt, tb;
            ttf[0]->get_char_width_and_height(ch, width, tt, tb);
            if (width) {
                w += width;
                if (tt < t) t = tt;
                if (tb > b) b = tb;
            }
        }
    } else {
        while (*text) {
            uint8_t c = *text++;
            if (c > 0x7F) continue;
            const auto &fd = get_pixel_font_data(c);
            w += fd.sw;
            if (fd.y < t) t = fd.y;
            if (fd.y + fd.h > b) b = fd.y + fd.h;
        }
    }
}

void sdl1_video::draw_text_pixel(int x, int y, const char *text, int width, bool shadow) {
    bool allow_wrap = false;
    int nwidth;
    int ox = x;
#ifdef FUNKEY
    //y -= 8;
#else
    y -= 16;
#endif
    unsigned bpp = curr_pixel_format == 1 ? 32 : 16;
    if (width == 0) {
        nwidth = width = screen->w - x;
    } else if (width == -1) {
        nwidth = width = screen->w - x;
        allow_wrap = true;
    } else {
        if (width < 0) {
            allow_wrap = true;
            width = -width;
            nwidth = width;
        } else {
            nwidth = width;
        }
    }
    auto swidth = screen->pitch / screen->format->BytesPerPixel;
    while (*text) {
        uint8_t c = *text++;
        if (c > 0x7F) continue;
        const auto &fd = get_pixel_font_data(c);
        if (fd.sw > nwidth) {
            if (!allow_wrap) break;
            x = ox;
            nwidth = width;
#ifdef FUNKEY
            y += 8 + 1;
#else
            y += 16 + 1;
#endif
        }
        nwidth -= fd.sw;
    #define CODE_WITH_TYPE(TYPE) \
        auto *ptr = (TYPE*)screen_ptr + x + fd.x + (y + fd.y) * swidth; \
        auto *fontdata = fd.data; \
        uint32_t wrapx = swidth - fd.w; \
        uint32_t step = (fd.w + 7) >> 3; \
        if (shadow) { \
            for (int h = fd.h; h; h--) { \
                uint8_t bitflag = 0x01; \
                uint32_t fdidx = 0; \
                for (int w = fd.w; w; w--) { \
                    if (fontdata[fdidx] & bitflag) { \
                        *ptr++ = (TYPE)-1; \
                        *(ptr + swidth) = 0; \
                    } else ++ptr; \
                    if (bitflag == 0x80) { \
                        fdidx++; \
                        bitflag = 1; \
                    } else { \
                        bitflag <<= 1; \
                    } \
                } \
                ptr += wrapx; \
                fontdata += step; \
            } \
        } else { \
            for (int h = fd.h; h; h--) { \
                uint8_t bitflag = 0x01; \
                uint32_t fdidx = 0; \
                for (int w = fd.w; w; w--) { \
                    if (fontdata[fdidx] & bitflag) \
                        *ptr++ = (TYPE)-1; \
                    else ++ptr; \
                    if (bitflag == 0x80) { \
                        fdidx++; \
                        bitflag = 1; \
                    } else { \
                        bitflag <<= 1; \
                    } \
                } \
                ptr += wrapx; \
                fontdata += step; \
            } \
        }
        if (bpp == 32) {
            CODE_WITH_TYPE(uint32_t)
        } else {
            CODE_WITH_TYPE(uint16_t)
        }
    #undef CODE_WITH_TYPE
        x += fd.sw;
    }
}

void sdl1_video::enter_menu() {
    //SDL_UnlockSurface(screen);
    usleep(10000);
    saved_width = curr_width;
    saved_height = curr_height;
    saved_pixel_format = curr_pixel_format;
    g_cfg.get_resolution(curr_width, curr_height);

    if(curr_pixel_format != 2)
        hw_screen = SDL_SetVideoMode(DEFAULT_WIDTH, DEFAULT_HEIGHT, 16, sdl_video_flags);

    curr_pixel_format = 2;
    //screen = SDL_SetVideoMode(curr_width, curr_height, curr_pixel_format == 1 ? 32 : 16, sdl_video_flags);
    sdl1_reset_screen_surface(RES_HW_SCREEN_HORIZONTAL, RES_HW_SCREEN_VERTICAL, curr_pixel_format == 1 ? 32 : 16);

    //SDL_LockSurface(screen);
    screen_ptr = screen->pixels;
}

void sdl1_video::leave_menu() {
    game_resolution_changed(saved_width, saved_height, 0, 0, saved_pixel_format);
}

}
