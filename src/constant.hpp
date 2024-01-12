#include <array>
#include <cstdint>
#include <iostream>

constexpr int TIME_LIMIT_MS = 2000;

/// @brief 手札枚数上限
constexpr int N_UB = 7;

/// @brief 手札枚数下限
constexpr int N_LB = 2;

/// @brief 山数の上限
constexpr int M_UB = 8;

/// @brief 山数の下限
constexpr int M_LB = 2;

/// @brief 手札補充選択肢の上限
constexpr int K_UB = 5;

/// @brief 手札補充選択肢の下限
constexpr int K_LB = 2;

/// @brief ターン数
constexpr int T = 1000;

constexpr int64_t INF    = 1e18;
constexpr int64_t ERASED = -INF;

constexpr double SCALE_UP_RATE_A =
#ifdef PARAM_SCALE_UP_RATE_A
    PARAM_SCALE_UP_RATE_A
#else
    0.85
#endif
    ;

constexpr int SCALE_UP_FREQ_A =
#ifdef PARAM_SCALE_UP_FREQ_A
    PARAM_SCALE_UP_FREQ_A
#else
    10
#endif
    ;

constexpr double SCALE_UP_RATE_B =
#ifdef PARAM_SCALE_UP_RATE_B
    PARAM_SCALE_UP_RATE_B
#else
    0.85
#endif
    ;

constexpr int SCALE_UP_FREQ_B =
#ifdef PARAM_SCALE_UP_FREQ_B
    PARAM_SCALE_UP_FREQ_B
#else
    5
#endif
    ;

constexpr double SCALE_UP_RATE_C =
#ifdef PARAM_SCALE_UP_RATE_C
    PARAM_SCALE_UP_RATE_C
#else
    0.8
#endif
    ;


constexpr std::pair<double, int> scale_up_rate_by_current_money_seed[] = {
    {SCALE_UP_RATE_A, SCALE_UP_FREQ_A},
    {SCALE_UP_RATE_B, SCALE_UP_FREQ_B},
    {SCALE_UP_RATE_C, 21 - SCALE_UP_FREQ_A - SCALE_UP_FREQ_B},
};

constexpr std::array<double, 21> scale_up_rate_by_current_money = [] {
    std::array<double, 21> ret{};
    constexpr auto& g = scale_up_rate_by_current_money_seed;
    int id            = 0;
    std::fill(ret.begin() + id, ret.begin() + id + g[0].second, g[0].first);
    id += g[0].second;
    std::fill(ret.begin() + id, ret.begin() + id + g[1].second, g[1].first);
    id += g[1].second;
    std::fill(ret.begin() + id, ret.begin() + id + g[2].second, g[2].first);
    id += g[2].second;
    return ret;
}();


enum CardType {
    WORK_ONE   = 0,
    WORK_ALL   = 1,
    DELETE_ONE = 2,
    DELETE_ALL = 3,
    SCALE_UP   = 4,
    CARD_TYPE_NUM,
};

std::istream& operator>>(std::istream& is, CardType& card_type) {
    int tmp;
    is >> tmp;
    card_type = static_cast<CardType>(tmp);
    return is;
}

constexpr int WEIGHT_MAX[]   = {20, 10, 10, 5, 3};
constexpr int WEIGHT_MAX_SUM = WEIGHT_MAX[WORK_ONE] + WEIGHT_MAX[WORK_ALL]
                               + WEIGHT_MAX[DELETE_ONE] + WEIGHT_MAX[DELETE_ALL]
                               + WEIGHT_MAX[SCALE_UP];
constexpr double PROB_MAX[] = {
    WEIGHT_MAX[WORK_ONE] / (double)(WEIGHT_MAX[WORK_ONE] + 4),
    WEIGHT_MAX[WORK_ALL] / (double)(WEIGHT_MAX[WORK_ALL] + 4),
    WEIGHT_MAX[DELETE_ONE] / (double)(WEIGHT_MAX[DELETE_ONE] + 4),
    WEIGHT_MAX[DELETE_ALL] / (double)(WEIGHT_MAX[DELETE_ALL] + 4),
    WEIGHT_MAX[SCALE_UP] / (double)(WEIGHT_MAX[SCALE_UP] + 4)};

constexpr double PROB_MIN[] = {
    1.0 / (WEIGHT_MAX_SUM - WEIGHT_MAX[WORK_ONE] + 1),
    1.0 / (WEIGHT_MAX_SUM - WEIGHT_MAX[WORK_ALL] + 1),
    1.0 / (WEIGHT_MAX_SUM - WEIGHT_MAX[DELETE_ONE] + 1),
    1.0 / (WEIGHT_MAX_SUM - WEIGHT_MAX[DELETE_ALL] + 1),
    1.0 / (WEIGHT_MAX_SUM - WEIGHT_MAX[SCALE_UP] + 1)};

constexpr double PROB_MEAN[] = {
    0.375209, 0.211730, 0.211730, 0.120046, 0.081284,
};

constexpr double SIMULATION_MS_THRESHOLD =
#ifdef PARAM_SIMULATION_MS_THRESHOLD
    PARAM_SIMULATION_MS_THRESHOLD
#else
    6.5
#endif
    ;

constexpr int64_t SIMULATION_SAMPLES_WHEN_FAST_CASE =
#ifdef PARAM_SIMULATION_SAMPLES_WHEN_FAST_CASE
    PARAM_SIMULATION_SAMPLES_WHEN_FAST_CASE
#else
    140
#endif
    ;

constexpr int64_t SIMULATION_SAMPLES_WHEN_SLOW_CASE =
#ifdef PARAM_SIMULATION_SAMPLES_WHEN_SLOW_CASE
    PARAM_SIMULATION_SAMPLES_WHEN_SLOW_CASE
#else
    90
#endif
    ;

constexpr int SIMULATION_TURNS =
#ifdef PARAM_SIMULATION_TURNS
    PARAM_SIMULATION_TURNS
#else
    30
#endif
    ;

constexpr int SETUP_TURN = 50;

constexpr int TEARDOWN_TURN = 50;

// 確率調査に用いるサンプルの数
constexpr int PROBABILITY_SAMPLES =
#ifdef PARAM_PROBABILITY_SAMPLES
    PARAM_PROBABILITY_SAMPLES
#else
    1000
#endif
    ;

constexpr double GREEDY_PICK_WORK_THRESHOLD =
#ifdef PARAM_GREEDY_PICK_WORK_THRESHOLD
    PARAM_GREEDY_PICK_WORK_THRESHOLD
#else
    0.8
#endif
    ;

constexpr double GREEDY_PICK_DELETE_ONE_THRESHOLD =
#ifdef PARAM_GREEDY_PICK_DELETE_ONE_THRESHOLD
    PARAM_GREEDY_PICK_DELETE_ONE_THRESHOLD
#else
    0.01
#endif
    ;

constexpr double OVER_THRESHOLD_RATE =
#ifdef PARAM_OVER_THRESHOLD_RATE
    PARAM_OVER_THRESHOLD_RATE
#else
    12.0
#endif
    ;

constexpr double DELETE_ONE_THRESHOLD_RATE =
#ifdef PARAM_DELETE_ONE_THRESHOLD_RATE
    PARAM_DELETE_ONE_THRESHOLD_RATE
#else
    0.87
#endif
    ;

constexpr double C1 =
#ifdef PARAM_C1
    PARAM_C1
#else
    1
#endif
    ;

constexpr double UCB_C =
#ifdef PARAM_UCB_C
    PARAM_UCB_C
#else
    0.7
#endif
    ;

constexpr int EACH_FIRST_TRIES =
#ifdef PARAM_EACH_FIRST_TRIES
    PARAM_EACH_FIRST_TRIES
#else
    30
#endif
    ;
