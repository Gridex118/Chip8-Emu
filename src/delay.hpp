#pragma once

#include <chrono>

using Clock = std::chrono::steady_clock;
using std::chrono::milliseconds;

class DelayClock {
public:
    DelayClock(unsigned short frequency);
    void tick() noexcept;
    [[nodiscard]] bool has_moved() noexcept;
private:
    unsigned short frequency;
    std::chrono::time_point<Clock> last_time_stamp{Clock::now()};
    bool moved{false};
};
