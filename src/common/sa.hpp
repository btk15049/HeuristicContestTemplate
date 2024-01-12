namespace {
#include <cmath>
#include <cstdint>

    using namespace std;

    inline double calc_temperature(int64_t i, int64_t tot, double t_start,
                                   double t_end) {
        const double delta = (t_end - t_start) / (tot - 1);
        return t_start + delta * i;
        // double alpha = pow(t_end / t_start, 1.0 / (tot - 1));
        // return t_start * pow(alpha, i);
    }

    inline double calc_probability(double current_score, double next_score,
                                   double t) {
        return exp((next_score - current_score) / t);
    }

} // namespace
