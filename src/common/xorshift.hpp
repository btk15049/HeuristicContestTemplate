#include <cstdint>
#include <vector>

namespace xorshift {
    constexpr uint64_t next(uint64_t p) {
        p = p ^ (p << 13);
        p = p ^ (p >> 7);
        return p ^ (p << 17);
    }

    struct Generator {
        uint64_t seed;
        Generator(uint64_t seed = 939393939393llu) : seed(seed) {}
        inline uint64_t gen() {
            seed = next(seed);
            return seed;
        }
    } _gen;

    // https://github.com/yosupo06/library-checker-problems/blob/5face7acd002a8fe2cd76fa6744f0901c194eb36/common/random.h#L38-L51
    // random choice from [0, upper]
    inline uint64_t gen_with_upper(uint64_t upper) {
        if (!(upper & (upper + 1))) {
            // b = 00..0011..11
            return _gen.gen() & upper;
        }
        int lg        = 63 - __builtin_clzll(upper);
        uint64_t mask = (lg == 63) ? ~0ULL : (1ULL << (lg + 1)) - 1;
        while (true) {
            uint64_t r = _gen.gen() & mask;
            if (r <= upper) return r;
        }
    }

    inline int64_t getInt(int64_t n) { return gen_with_upper(n - 1); }
    inline int64_t getInt(int l, int r) { return l + getInt(r - l + 1); }
    inline uint64_t getUint() { return _gen.gen(); }

    inline double getDouble() { return (_gen.gen() % 65536) / 65535.0; }
    inline double getDouble(double l, double r) {
        return l + getDouble() * (r - l);
    }
    inline double gauss(double mu, double sigma) {
        double z =
            sqrt(-2.0 * log(getDouble())) * sin(2.0 * M_PI * getDouble());
        return mu + sigma * z;
    }
    template <typename T>
    inline void shuffle(std::vector<T>& v) {
        const int n = v.size();
        for (int i = 0; i < n - 1; i++) {
            std::swap(v[i], v[i + 1 + getInt(n - i - 1)]);
        }
    }

    void set_seed(uint64_t seed) { _gen = Generator(seed); }
    uint64_t get_seed() { return _gen.seed; }
} // namespace xorshift
