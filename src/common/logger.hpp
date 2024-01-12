#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace logger {
    using namespace std;
    constexpr int INF = 99999999;

    vector<tuple<string, int, string>> logger;

    inline void push(string key, int turn, int64_t v) {
        logger.emplace_back(key, turn, to_string(v));
    }
    inline void push(const char* key, int turn, int64_t v) {
        logger.emplace_back(key, turn, to_string(v));
    }
    inline void push(string key, int64_t v) { push(key, INF, v); }
    inline void push(const char* key, int64_t v) { push(key, INF, v); }

    inline void puah(string key, int turn, double v) {
        logger.emplace_back(key, turn, to_string(v));
    }
    inline void push(const char* key, int turn, double v) {
        logger.emplace_back(key, turn, to_string(v));
    }
    inline void push(string key, double v) { push(key, INF, v); }
    inline void push(const char* key, double v) { push(key, INF, v); }

    inline void push(string key, int turn, string v) {
        logger.emplace_back(key, turn, v);
    }
    inline void push(const char* key, int turn, string v) {
        logger.emplace_back(key, turn, v);
    }
    inline void push(string key, string v) { push(key, INF, v); }
    inline void push(const char* key, string v) { push(key, INF, v); }


    inline void flush() {
#ifdef LOCAL
        stringstream ss;
        for (auto& [key, turn, v] : logger) {
            // [key](turn)v or [key]v
            if (turn == INF) {
                ss << "[" << key << "]" << v << "\n";
            }
            else {
                ss << "[" << key << "](" << turn << ")" << v << "\n";
            }
        }
        cerr << ss.str();
        cerr.flush();
#endif
    }

} // namespace logger
