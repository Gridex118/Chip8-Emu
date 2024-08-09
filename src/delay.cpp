#include "delay.hpp"

DelayClock::DelayClock(unsigned short frequency)
: frequency(frequency) { }

void DelayClock::tick() noexcept {
    auto current_time_stamp = Clock::now();
    milliseconds duration = std::chrono::duration_cast<milliseconds>(current_time_stamp - last_time_stamp);
    const milliseconds min_ms_elapsed = static_cast<milliseconds>(1000/frequency);
    if (duration >= min_ms_elapsed) {
        last_time_stamp = current_time_stamp;
        moved = true;
    }
}

bool DelayClock::has_moved() noexcept {
    if (moved) {
        moved = false;
        return true;
    } else return false;
}
