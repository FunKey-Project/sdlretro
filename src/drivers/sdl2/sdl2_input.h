#pragma once

#include "input_base.h"

#include <vector>
#include <array>
#include <string>

extern "C" {
typedef struct _SDL_GameController SDL_GameController;
}

namespace drivers {

class sdl2_input: public input_base {
public:
    sdl2_input();
    ~sdl2_input() override;

    void post_init() override;

    void get_input_name(uint16_t id, std::string &device_name, std::string &name) const override;
    uint16_t get_input_from_name(const std::string &device_name, const std::string &name) const override;
};

}
