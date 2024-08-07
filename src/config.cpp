#include "config.hpp"
#include <fstream>

namespace ch8cfg {

int Config::parse_json() {
    std::ifstream config("config.json");
    if (config.good()) {
        data = nlohmann::json::parse(config);
    } else {
        return -1;
    }
    config.close();
    return 0;
}

void Config::write_json() {
    std::ofstream config("config.json");
    data = {
        {"scale", 10},
        {"freq", 540}
    };
    config << std::setw(4) << data << std::endl;
    config.close();
}

Config::Config() {
    if (parse_json() != 0)
        write_json();
    disp_scale = data["scale"];
    cpu_freq = data["freq"];
}

}
