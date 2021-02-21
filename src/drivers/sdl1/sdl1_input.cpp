#include "sdl1_input.h"

#include <libretro.h>

#include <SDL.h>

namespace drivers {

sdl1_input::sdl1_input() {
    if (!SDL_WasInit(SDL_INIT_JOYSTICK))
        SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    SDL_JoystickEventState(SDL_ENABLE);
    for (int i = 0; i < 2; ++i)
        joystick[i] = SDL_JoystickOpen(i);
}

sdl1_input::~sdl1_input() {
    for (int i = 0; i < 2; ++i) {
        if (joystick[i]) {
            SDL_JoystickClose(joystick[i]);
            joystick[i] = nullptr;
        }
    }
}

void sdl1_input::post_init() {
    input_base::post_init();
#ifdef FUNKEY
    keymap = {
        SDLK_b,   // RETRO_DEVICE_ID_JOYPAD_B
        SDLK_y, // RETRO_DEVICE_ID_JOYPAD_Y
        SDLK_k, // RETRO_DEVICE_ID_JOYPAD_SELECT
        SDLK_s, // RETRO_DEVICE_ID_JOYPAD_START
        SDLK_u,     // RETRO_DEVICE_ID_JOYPAD_UP
        SDLK_d,   // RETRO_DEVICE_ID_JOYPAD_DOWN
        SDLK_l,   // RETRO_DEVICE_ID_JOYPAD_LEFT
        SDLK_r,  // RETRO_DEVICE_ID_JOYPAD_RIGHT
        SDLK_a,  // RETRO_DEVICE_ID_JOYPAD_A
        SDLK_x,  // RETRO_DEVICE_ID_JOYPAD_X
        SDLK_m,    // RETRO_DEVICE_ID_JOYPAD_L
        SDLK_n, // RETRO_DEVICE_ID_JOYPAD_R
        SDLK_v, // RETRO_DEVICE_ID_JOYPAD_L2
        SDLK_o, // RETRO_DEVICE_ID_JOYPAD_R2
        SDLK_j, // RETRO_DEVICE_ID_JOYPAD_L3
        SDLK_i, // RETRO_DEVICE_ID_JOYPAD_R3
    };
#else
    keymap = {
        SDLK_k, // RETRO_DEVICE_ID_JOYPAD_B
        SDLK_j, // RETRO_DEVICE_ID_JOYPAD_Y
        SDLK_c, // RETRO_DEVICE_ID_JOYPAD_SELECT
        SDLK_v, // RETRO_DEVICE_ID_JOYPAD_START
        SDLK_w, // RETRO_DEVICE_ID_JOYPAD_UP
        SDLK_s, // RETRO_DEVICE_ID_JOYPAD_DOWN
        SDLK_a, // RETRO_DEVICE_ID_JOYPAD_LEFT
        SDLK_d, // RETRO_DEVICE_ID_JOYPAD_RIGHT
        SDLK_l, // RETRO_DEVICE_ID_JOYPAD_A
        SDLK_i, // RETRO_DEVICE_ID_JOYPAD_X
        SDLK_q, // RETRO_DEVICE_ID_JOYPAD_L
        SDLK_e, // RETRO_DEVICE_ID_JOYPAD_R
        SDLK_1, // RETRO_DEVICE_ID_JOYPAD_L2
        SDLK_3, // RETRO_DEVICE_ID_JOYPAD_R2
        SDLK_z, // RETRO_DEVICE_ID_JOYPAD_L3
        SDLK_x, // RETRO_DEVICE_ID_JOYPAD_R3
    };
#endif
    if (game_to_km_mapping.empty()) {
        for (size_t i = 0; i < keymap.size(); ++i) {
            set_km_mapping(keymap[i], i);
        }
    }
    assign_port(0, 0);
}

void sdl1_input::input_poll() {
    SDL_JoystickUpdate();
    for (size_t z = 0; z < ports.size(); ++z) {
        auto &port = ports[z];
        if (!port.enabled) continue;

        if (joystick[z]) {
            port.analog_axis[0] = SDL_JoystickGetAxis(joystick[z], 0);
            port.analog_axis[1] = SDL_JoystickGetAxis(joystick[z], 1);
            port.analog_axis[2] = SDL_JoystickGetAxis(joystick[z], 2);
            port.analog_axis[3] = SDL_JoystickGetAxis(joystick[z], 3);
        }
    }
}

void sdl1_input::port_connected(int index) {
}

void sdl1_input::port_disconnected(int device_id) {
}

void sdl1_input::get_input_name(uint64_t input, std::string &device_name, std::string &name) const {
}

uint64_t sdl1_input::get_input_from_name(const std::string &device_name, const std::string &name) const {
    return 0;
}

}
