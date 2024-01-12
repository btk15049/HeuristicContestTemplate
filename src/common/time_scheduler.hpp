#include <chrono>

namespace scheduler {
    using namespace std::chrono;
    struct Scheduler {
        high_resolution_clock::time_point bg;
        int64_t limit;
        int64_t progress;
        int64_t latest;
        int64_t latest_interval  = 0;
        constexpr static int buf = 256;
        int not_updated;
        Scheduler(int time_limit) {
            latest      = 0;
            bg          = high_resolution_clock::now();
            limit       = time_limit * 1e3;
            not_updated = 0;
        }
        inline bool update() {
            if (not_updated >= buf) {
                const int64_t elapsed = duration_cast<microseconds>(
                                            high_resolution_clock::now() - bg)
                                            .count();
                not_updated     = 0;
                latest_interval = elapsed - latest;
                latest          = elapsed;
                progress        = elapsed;
            }
            else {
                not_updated++;
                progress = latest + latest_interval * not_updated / buf;
            }

            return progress < limit;
        }
    };
} // namespace scheduler
