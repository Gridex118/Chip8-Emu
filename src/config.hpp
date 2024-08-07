#pragma once

#include <nlohmann/json.hpp>

namespace ch8cfg {

class Config {
public:
    Config();
    short disp_scale;
    short cpu_freq;
private:
    nlohmann::json data;
    int parse_json();
    void write_json();
};

}
