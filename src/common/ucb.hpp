#include <cmath>
#include <iostream>
#include <utility>
#include <vector>


struct UcbArmMetrics {
    int count;
    double total_reward;
    double total_squared_reward;

    UcbArmMetrics() : count(0), total_reward(0.0), total_squared_reward(0.0) {}

    void update(double reward) {
        ++count;
        total_reward += reward;
        total_squared_reward += reward * reward;
    }

    inline double average() const { return total_reward / count; }
    inline double variance() const {
        const double average = this->average();
        return total_squared_reward / count - average * average;
    }

    inline double exploration_factor(int total_count) const {
        return std::sqrt(std::log(total_count) / count);
    }

    inline double score(double c, int total_count) const {
        return average() + c * exploration_factor(total_count);
    }

    constexpr static double z95 = 1.96; // 95% confidence interval
    constexpr static double z99 = 2.58; // 99% confidence interval
    inline double confidence_interval() const {
        return z95 * std::sqrt(variance() / count);
    }
};

struct UpperConfidenceBound {
    int total_count;
    std::vector<UcbArmMetrics> metrics;

    UpperConfidenceBound(int arms)
        : total_count(0), metrics(arms, UcbArmMetrics()) {}

    inline double score(int arm, double c) const {
        return metrics[arm].score(c, total_count);
    }

    inline double average(int arm) const { return metrics[arm].average(); }

    inline void update(int arm, double reward) {
        ++total_count;
        metrics[arm].update(reward);
    }

    int select_arm(double c) const {
        int ret          = 0;
        double max_score = score(0, c);
        for (int i = 1; i < (int)metrics.size(); ++i) {
            const double s = score(i, c);
            if (s > max_score) {
                max_score = s;
                ret       = i;
            }
        }
        return ret;
    }

    int best_arm() const {
        int ret          = 0;
        double max_score = average(0);
        for (int i = 1; i < (int)metrics.size(); ++i) {
            const double s = average(i);
            if (s > max_score) {
                max_score = s;
                ret       = i;
            }
        }
        return ret;
    }

    inline int count(int arm) const { return metrics[arm].count; }

    bool check_early_stop(double c) const {
        (void)c;
        const int best = this->best_arm();
        const double best_lower_bound =
            metrics[best].average() - metrics[best].confidence_interval();
        // metrics[best].average()
        // - c * metrics[best].exploration_factor(total_count);

        for (int i = 0; i < (int)metrics.size(); ++i) {
            if (i == best) {
                continue;
            }
            const double upper_bound =
                metrics[i].average() + metrics[i].confidence_interval();
            // metrics[i].average()
            // + c * metrics[i].exploration_factor(total_count);
            if (upper_bound > best_lower_bound) {
                return false;
            }
        }
        return true;
    }
};
