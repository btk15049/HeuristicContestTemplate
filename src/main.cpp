// clang-format off

#include "common/macros.hpp"
#include "common/debug.hpp"

#include "common/stl.hpp"
#include "common/sa.hpp"
#include "common/time_scheduler.hpp"
#include "common/xorshift.hpp"
#include "common/logger.hpp"
#include "common/original_vector.hpp"
#include "common/ucb.hpp"

#include "constant.hpp"
// clang-format on


struct Card {
    int id;
    CardType type;
    int64_t work_amount;
    int64_t cost;
};

struct Mountain {
    int64_t height;
    int64_t value;
};

struct Field {
    int m;
    Mountain mountains[M_UB];
    void load() {
        for (int i = 0; i < m; ++i) {
            cin >> mountains[i].height >> mountains[i].value;
        }
    }
};

struct Hand {
    int n;
    Card cards[N_UB];
    void load() {
        for (int i = 0; i < n; ++i) {
            cin >> cards[i].type >> cards[i].work_amount;
        }
    }
    void assign_id() {
        for (int i = 0; i < n; ++i) {
            cards[i].id = i;
        }
    }

    void sort_by_cost_amount() {
        std::sort(cards, cards + n, [](const auto& a, const auto& b) {
            if (a.cost == b.cost) {
                return a.work_amount > b.work_amount;
            }
            return a.cost < b.cost;
        });
    }
};


struct NextCards {
    int k;
    Card cards[K_UB];
    void load() {
        for (int i = 0; i < k; ++i) {
            cin >> cards[i].type >> cards[i].work_amount >> cards[i].cost;
        }
    }
    void assign_id() {
        for (int i = 0; i < k; ++i) {
            cards[i].id = i;
        }
    }

    void sort_by_cost_amount() {
        sort(cards, cards + k, [](const auto& a, const auto& b) {
            if (a.cost == b.cost) {
                return a.work_amount > b.work_amount;
            }
            return a.cost < b.cost;
        });
    }
};

namespace io {
    void input_first(Hand& hand, Field& field, NextCards& next_cards) {
        int t;
        cin >> hand.n >> field.m >> next_cards.k >> t;
        hand.load();
        hand.assign_id();
        field.load();
    }

    void input_next(int64_t& money, Field& field, NextCards& next_cards) {
        field.load();
        cin >> money;
// -Werror=array-bounds を無視
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
        next_cards.load();
        next_cards.assign_id();
        next_cards.sort_by_cost_amount();
#pragma GCC diagnostic pop
    }

    void output_use_card(int c_pos, int m_pos) {
        cout << c_pos << " " << m_pos << endl;
    }

    void output_pick_card(int k_pos) { cout << k_pos << endl; }

} // namespace io

namespace input {
    Hand hand;
    Field field;
    NextCards next_cards;
} // namespace input

using C = Card;

inline int64_t clamp(int64_t x, int64_t l, int64_t r) {
    return max(l, min(r, x));
}

inline double clamp_double(double x, double l, double r) {
    return max(l, min(r, x));
}

struct InputGenerator {
    double w[5];

    Mountain generate_mountain(int scale) {
        const int base = 1 << scale;
        Mountain ret;
        double b   = xorshift::getDouble(2.0, 8.0);
        ret.height = int64_t(pow(2, b)) * base;
        ret.value  =                                       //
            int64_t(                                       //
                pow(                                       //
                    2,                                     //
                    clamp_double(                          //
                        xorshift::gauss(b, 0.5), 0.0, 10.0 //
                        )                                  //
                    )                                      //
                )                                          //
            * base;
        return ret;
    }

    C generate_card(int scale, int m) {
        const int base = 1 << scale;
        double r       = xorshift::getDouble();
        C ret;
        if (r < w[0]) {
            const int64_t w_dash = xorshift::getInt(1, 50);
            ret.type             = WORK_ONE;
            ret.work_amount      = w_dash * base;
            ret.cost =
                clamp(round(xorshift::gauss(w_dash, w_dash / 3.0)), 1, 10000)
                * base;
            return ret;
        }
        r -= w[0];
        if (r < w[1]) {
            const int64_t w_dash = xorshift::getInt(1, 50);
            ret.type             = WORK_ALL;
            ret.work_amount      = w_dash * base;
            ret.cost =
                clamp(round(xorshift::gauss(w_dash * m, w_dash * m / 3.0)), 1,
                      10000)
                * base;
            return ret;
        }
        r -= w[1];
        if (r < w[2]) {
            ret.type        = DELETE_ONE;
            ret.work_amount = 0;
            ret.cost        = xorshift::getInt(0, 10) * base;
            return ret;
        }
        r -= w[2];
        if (r < w[3]) {
            ret.type        = DELETE_ALL;
            ret.work_amount = 0;
            ret.cost        = xorshift::getInt(0, 10) * base;
            return ret;
        }
        ret.type        = SCALE_UP;
        ret.work_amount = 0;
        ret.cost        = xorshift::getInt(200, 1000) * base;
        return ret;
    }

    void generate_cards(int turn_num, NextCards cards[]) {
        for (int i = 0; i < turn_num; ++i) {
            cards[i].k                    = input::next_cards.k;
            cards[i].cards[0].id          = 0;
            cards[i].cards[0].type        = WORK_ONE;
            cards[i].cards[0].work_amount = 1;
            cards[i].cards[0].cost        = 0;
            for (int j = 1; j < cards[i].k; ++j) {
                cards[i].cards[j]    = generate_card(0, input::field.m);
                cards[i].cards[j].id = j;
            }
            cards[i].sort_by_cost_amount();
        }
    }
};


std::chrono::high_resolution_clock::time_point start_time;

pair<int, int> use_card_greedy(Hand& h, Field& f, int64_t current_money,
                               int current_scale) {
    (void)current_money;
    (void)current_scale;
    static vector<pair<int64_t, int>> work_one_pos;
    static vector<pair<int64_t, int>> work_all_pos;
    static vector<int> delete_one_pos;
    static vector<int> delete_all_pos;
    static vector<int> scale_up_pos;

    work_one_pos.clear();
    work_all_pos.clear();
    delete_one_pos.clear();
    delete_all_pos.clear();
    scale_up_pos.clear();

    for (int i = 0; i < h.n; ++i) {
        switch (h.cards[i].type) {
            case WORK_ONE:
                work_one_pos.emplace_back(h.cards[i].work_amount, i);
                break;
            case WORK_ALL:
                work_all_pos.emplace_back(h.cards[i].work_amount, i);
                break;
            case DELETE_ONE:
                delete_one_pos.emplace_back(i);
                break;
            case DELETE_ALL:
                delete_all_pos.emplace_back(i);
                break;
            case SCALE_UP:
                if (current_scale < 20) scale_up_pos.emplace_back(i);
                break;
            default:
                assert(false);
        }
    }
    sort(work_one_pos.begin(), work_one_pos.end(), [&](auto& a, auto& b) {
        return h.cards[a.second].work_amount < h.cards[b.second].work_amount;
    });
    sort(work_all_pos.begin(), work_all_pos.end(), [&](auto& a, auto& b) {
        return h.cards[a.second].work_amount < h.cards[b.second].work_amount;
    });

    // SCALE_UP があるときは SCALE_UPを選ぶ
    if (scale_up_pos.size() > 0u) {
        return {scale_up_pos[0], 0};
    }


    int best_mt_pos  = 0;
    int worst_mt_pos = 0;
    for (int i = 1; i < f.m; ++i) {
        double pf = f.mountains[i].value / (double)f.mountains[i].height;

        if (pf > f.mountains[best_mt_pos].value
                     / (double)f.mountains[best_mt_pos].height) {
            best_mt_pos = i;
        }
        if (pf < f.mountains[worst_mt_pos].value
                     / (double)f.mountains[worst_mt_pos].height) {
            worst_mt_pos = i;
        }
    }

    // 働く場合の効率を先に計算する
    int64_t work_profit = 0;
    CardType work_type  = WORK_ONE;
    int64_t work_pos    = -1;
    {
        int64_t max_work = 0;
        if (work_all_pos.size() > 0u) {
            const auto cap = work_all_pos.back().first;
            int64_t work   = 0;
            for (int i = 0; i < f.m; ++i) {
                work += min(cap, f.mountains[i].height);
            }
            if (work > max_work) {
                work_type = WORK_ALL;
                work_pos  = work_all_pos.back().second;
                max_work  = work;
            }
        }
        if (work_one_pos.size() > 0u) {
            const auto cap = work_one_pos.back().first;
            int work       = min(cap, f.mountains[best_mt_pos].height);
            if (work > max_work) {
                work_type = WORK_ONE;
                work_pos  = work_one_pos.back().second;
                max_work  = work;
            }
        }

        if (work_pos != -1) {
            if (work_type == WORK_ALL) {
                for (int i = 0; i < f.m; ++i) {
                    if (f.mountains[i].height <= work_all_pos.back().first) {
                        work_profit += f.mountains[i].value;
                    }
                }
            }
            else {
                int p = work_one_pos.size() - 1;
                // 労働力が過剰なときは弱いカードを選ぶ
                while (p > 0) {
                    if (work_one_pos[p - 1].first
                        >= f.mountains[best_mt_pos].height) {
                        p--;
                    }
                    else {
                        const auto over = work_one_pos[p].first
                                          - f.mountains[best_mt_pos].height;
                        if (over
                            <= (1 << current_scale) * OVER_THRESHOLD_RATE) {
                            break;
                        }
                        p--;
                    }
                }
                work_pos = work_one_pos[p].second;
            }
            if (f.mountains[best_mt_pos].height <= work_one_pos.back().first) {
                work_profit += f.mountains[best_mt_pos].value;
            }
        }
    }

    // 手札にDELETE_ALLがあるとき
    if (delete_all_pos.size() > 0u) {
        return {delete_all_pos[0], 0};
    }

    const double delete_one_threshold_rate = DELETE_ONE_THRESHOLD_RATE;

    if (delete_one_pos.size() > 0u) {
        double pf = f.mountains[worst_mt_pos].value
                    / (double)f.mountains[worst_mt_pos].height;
        if (pf < delete_one_threshold_rate) {
            return {delete_one_pos[0], worst_mt_pos};
        }
    }

    if (work_pos != -1) {
        if (work_type == WORK_ALL) {
            return {work_pos, 0};
        }
        else {
            return {work_pos, best_mt_pos};
        }
    }

    if (h.cards[0].type == DELETE_ONE) {
        return {0, worst_mt_pos};
    }

    return {0, 0};
}

vector<int> filter_next_cards(const NextCards& nc, int64_t current_money,
                              int current_scale) {
    static vector<int> work_one_pos;
    static vector<int> work_all_pos;
    static vector<int> delete_one_pos;
    static vector<int> delete_all_pos;
    static vector<int> scale_up_pos;
    work_one_pos.clear();
    work_all_pos.clear();
    delete_one_pos.clear();
    delete_all_pos.clear();
    scale_up_pos.clear();

    for (int i = 0; i < nc.k; ++i) {
        if (nc.cards[i].cost > current_money) continue;
        switch (nc.cards[i].type) {
            case WORK_ONE:
                work_one_pos.emplace_back(i);
                break;
            case WORK_ALL:
                work_all_pos.emplace_back(i);
                break;
            case DELETE_ONE:
                if (delete_one_pos.empty()) delete_one_pos.emplace_back(i);
                break;
            case DELETE_ALL:
                if (delete_all_pos.empty()) delete_all_pos.emplace_back(i);
                break;
            case SCALE_UP:
                if (current_scale < 20 && scale_up_pos.empty())
                    scale_up_pos.emplace_back(i);
                break;
            default:
                assert(false);
        }
    }
    if (work_one_pos.size() > 0u) {
        static vector<int> work_one_pos_tmp;
        work_one_pos_tmp.clear();
        work_one_pos_tmp.emplace_back(work_one_pos[0]);
        for (int i = 1; i < (int)work_one_pos.size(); ++i) {
            if (nc.cards[work_one_pos_tmp.back()].work_amount
                < nc.cards[work_one_pos[i]].work_amount) {
                work_one_pos_tmp.emplace_back(work_one_pos[i]);
            }
        }
        swap(work_one_pos, work_one_pos_tmp);
    }
    if (work_all_pos.size() > 0u) {
        static vector<int> work_all_pos_tmp;
        work_all_pos_tmp.clear();
        work_all_pos_tmp.emplace_back(work_all_pos[0]);
        for (int i = 1; i < (int)work_all_pos.size(); ++i) {
            if (nc.cards[work_all_pos_tmp.back()].work_amount
                < nc.cards[work_all_pos[i]].work_amount) {
                work_all_pos_tmp.emplace_back(work_all_pos[i]);
            }
        }
        swap(work_all_pos, work_all_pos_tmp);
    }
    static vector<int> ret;
    ret.clear();
    ret.insert(ret.end(), work_one_pos.begin(), work_one_pos.end());
    ret.insert(ret.end(), work_all_pos.begin(), work_all_pos.end());
    ret.insert(ret.end(), delete_one_pos.begin(), delete_one_pos.end());
    ret.insert(ret.end(), delete_all_pos.begin(), delete_all_pos.end());
    ret.insert(ret.end(), scale_up_pos.begin(), scale_up_pos.end());
    return ret;
}

int pick_card_greedy(const Hand& h, const NextCards& nc, int64_t current_money,
                     int current_scale, int turn) {
    int own_delete_one = 0;
    for (int i = 0; i < h.n; ++i) {
        switch (h.cards[i].type) {
            case WORK_ONE:
            case WORK_ALL:
            case DELETE_ALL:
            case SCALE_UP:
                break;
            case DELETE_ONE:
                own_delete_one++;
                break;
            default:
                assert(false);
        }
    }
    static vector<int> candidates;
    candidates.clear();
    auto filtered_pos = filter_next_cards(nc, current_money, current_scale);
    for (int i : filtered_pos) {
        // ターンによる減衰率
        const double decay_rate = min(1.0, (T - turn) / 200.0);
        const int64_t work_card_threshold =
            current_money * GREEDY_PICK_WORK_THRESHOLD;
        const int64_t delete_one_threshold =
            current_money * GREEDY_PICK_DELETE_ONE_THRESHOLD;
        const int64_t scale_up_threshold =
            min(current_money * scale_up_rate_by_current_money[current_scale]
                    * decay_rate,
                (int64_t(1) << current_scale) * 500.0);
        double pf;
        constexpr double work_one_pf_threshold = 1.3;
        constexpr double work_all_pf_threshold = 1.3;
        switch (nc.cards[i].type) {
            case WORK_ONE:
                pf = nc.cards[i].work_amount / double(nc.cards[i].cost + 0.01);
                if (nc.cards[i].cost <= work_card_threshold
                    && pf >= work_one_pf_threshold) {
                    candidates.emplace_back(i);
                }
                break;
            case WORK_ALL:
                pf = nc.cards[i].work_amount * input::field.m
                     / double(nc.cards[i].cost + 0.01);
                if (nc.cards[i].cost <= work_card_threshold
                    && pf >= work_all_pf_threshold) {
                    candidates.emplace_back(i);
                }
                break;
            case DELETE_ONE:
                if (nc.cards[i].cost <= delete_one_threshold) {
                    candidates.emplace_back(i);
                }
                break;
            case DELETE_ALL:
                // candidates.emplace_back(i);
                break;
            case SCALE_UP:
                if (nc.cards[i].cost <= scale_up_threshold
                    && current_scale < 20) {
                    candidates.emplace_back(i);
                }
                break;
            default:
                assert(false);
        }
    }

    // SCALE_UP があるときはSCALE_UPを選ぶ
    for (int card_pos : candidates) {
        if (nc.cards[card_pos].type == SCALE_UP) {
            return card_pos;
        }
    }

    int best_work_card_pos = -1;
    {
        int64_t best_work = -INF;
        for (int cand_pos : candidates) {
            if (nc.cards[cand_pos].type == WORK_ONE) {
                const double pf =
                    nc.cards[cand_pos].work_amount - nc.cards[cand_pos].cost;
                if (pf > best_work) {
                    best_work_card_pos = cand_pos;
                    best_work          = pf;
                }
            }
            else if (nc.cards[cand_pos].type == WORK_ALL) {
                const double pf =
                    nc.cards[cand_pos].work_amount * input::field.m
                    - nc.cards[cand_pos].cost;
                if (pf > best_work) {
                    best_work_card_pos = cand_pos;
                    best_work          = pf;
                }
            }
            else if (nc.cards[cand_pos].type == DELETE_ONE) {
                const double pf =
                    (1 << current_scale) * 0.5 - nc.cards[cand_pos].cost;
                if (pf > best_work) {
                    best_work_card_pos = cand_pos;
                    best_work          = pf;
                }
            }
        }
    }

    return best_work_card_pos;

    assert(nc.cards[0].cost == 0 && nc.cards[0].type == WORK_ONE);
    return candidates[0]; // コスト 0 の WORK_ONE が入るはず
}

void update_field(Field& f, const C& card, int mountain_pos,
                  int64_t& current_money, int& current_scale) {
    switch (card.type) {
        case WORK_ONE:
            f.mountains[mountain_pos].height -= card.work_amount;
            if (f.mountains[mountain_pos].height <= 0) {
                current_money += f.mountains[mountain_pos].value;
                f.mountains[mountain_pos].value = ERASED;
            }
            break;
        case WORK_ALL:
            for (int i = 0; i < f.m; ++i) {
                f.mountains[i].height -= card.work_amount;
                if (f.mountains[i].height <= 0) {
                    current_money += f.mountains[i].value;
                    f.mountains[i].value = ERASED;
                }
            }
            break;
        case DELETE_ONE:
            f.mountains[mountain_pos].value = ERASED;
            break;
        case DELETE_ALL:
            for (int i = 0; i < f.m; ++i) {
                f.mountains[i].value = ERASED;
            }
            break;
        case SCALE_UP:
            current_scale++;
            break;
        default:
            assert(false);
    }
}

struct Estimator {
    vector<NextCards> future_cards;
    InputGenerator input_generator;

    Estimator(int current_turn, int last_turn, double x0, double x1, double x2,
              double x3, double x4) {
        double tot = x0 + x1 + x2 + x3 + x4 + 0.001;
        double p   = min(1.0, tot / double(PROBABILITY_SAMPLES));

        x0  = p * x0 / tot + (1.0 - p) * PROB_MEAN[0];
        x1  = p * x1 / tot + (1.0 - p) * PROB_MEAN[1];
        x2  = p * x2 / tot + (1.0 - p) * PROB_MEAN[2];
        x3  = p * x3 / tot + (1.0 - p) * PROB_MEAN[3];
        x4  = p * x4 / tot + (1.0 - p) * PROB_MEAN[4];
        tot = x0 + x1 + x2 + x3 + x4;

        input_generator.w[0] = x0 / tot;
        input_generator.w[1] = x1 / tot;
        input_generator.w[2] = x2 / tot;
        input_generator.w[3] = x3 / tot;
        input_generator.w[4] = x4 / tot;
        future_cards.resize(last_turn - current_turn + 1);
        input_generator.generate_cards(last_turn - current_turn + 1,
                                       future_cards.data());
    }

    double estimate(int current_turn, int last_turn, int64_t current_money,
                    int current_scale, const Hand& hand_, const Field& field_) {
        Hand h;
        h.n = hand_.n;
        for (int i = 0; i < h.n; ++i) {
            h.cards[i].type        = hand_.cards[i].type;
            h.cards[i].work_amount = hand_.cards[i].work_amount;
        }
        Field f;
        f.m = field_.m;
        for (int i = 0; i < f.m; ++i) {
            f.mountains[i].height = field_.mountains[i].height;
            f.mountains[i].value  = field_.mountains[i].value;
        }

        for (int turn = current_turn; turn < last_turn; ++turn) {
            auto [use_pos, mountain_pos] =
                use_card_greedy(h, f, current_money, current_scale);
            update_field(f, h.cards[use_pos], mountain_pos, current_money,
                         current_scale);
            // assert(current_scale <= 20);
            if (turn < T - 1) {
                for (int i = 0; i < f.m; ++i) {
                    if (f.mountains[i].value == ERASED) {
                        f.mountains[i] =
                            input_generator.generate_mountain(current_scale);
                    }
                }

                static NextCards nc;
                nc.k = future_cards[turn - current_turn].k;
                for (int i = 0; i < nc.k; ++i) {
                    nc.cards[i] = future_cards[turn - current_turn].cards[i];
                    nc.cards[i].cost <<= current_scale;
                    nc.cards[i].work_amount <<= current_scale;
                }
                auto pick_pos =
                    pick_card_greedy(h, nc, current_money, current_scale, turn);
                current_money -= nc.cards[pick_pos].cost;
                h.cards[use_pos] = nc.cards[pick_pos];
            }
        }
        return current_money + C1 * (T - last_turn) * (1 << current_scale);
        // return current_money;
    }
};

int64_t total_ms_pick_card = 0;
int64_t pick_card_call_num = 0;
double avg_ms_pick_card    = 1;

int pick_card(const Hand& h_, int used_pos, const Field& f, const NextCards& nc,
              int64_t current_money, int current_scale, int turn,
              int64_t freq[5]) {
    using namespace std::chrono;
    auto now        = high_resolution_clock::now();
    auto candidates = filter_next_cards(nc, current_money, current_scale);
    if (candidates.size() == 1u) {
        return candidates[0];
    }
    const int turns     = input::next_cards.k <= 2 ? 50 : 30;
    const int last_turn = min(turn + turns, T);
    static vector<Estimator> estimators;
    estimators.clear();
    const int sample_num = avg_ms_pick_card < SIMULATION_MS_THRESHOLD
                               ? SIMULATION_SAMPLES_WHEN_FAST_CASE
                               : SIMULATION_SAMPLES_WHEN_SLOW_CASE;
    for (int i = 0; i < sample_num; ++i) {
        estimators.emplace_back(turn, last_turn, freq[0], freq[1], freq[2],
                                freq[3], freq[4]);
    }

    Hand h;
    h.n = h_.n;
    for (int i = 0; i < h.n; ++i) {
        h.cards[i].type        = h_.cards[i].type;
        h.cards[i].work_amount = h_.cards[i].work_amount;
    }

    UpperConfidenceBound ucb(candidates.size());
    // initialize
    for (size_t i = 0; i < candidates.size(); ++i) {
        const int nc_pos  = candidates[i];
        h.cards[used_pos] = nc.cards[nc_pos];

        for (int j = 0; j < EACH_FIRST_TRIES; j++) {
            const double score = estimators[j].estimate(
                turn + 1, last_turn, current_money - nc.cards[nc_pos].cost,
                current_scale, h, f);
            ucb.update(i, score);
        }
    }

    const int tries = (sample_num - EACH_FIRST_TRIES) * candidates.size();
    // const double c         = (1 << current_scale) * T * UCB_C;
    double total       = 0;
    const double ucb_c = input::next_cards.k <= 2 ? 0.7 : 1.0;
    for (int i = 0; i < tries; ++i) {
        const double c    = (total / ucb.total_count) * ucb_c;
        const int arm     = ucb.select_arm(c);
        const int nc_pos  = candidates[arm];
        h.cards[used_pos] = nc.cards[nc_pos];
        const int round   = ucb.count(arm);
        if (round >= (int)estimators.size()) {
            // estimators.emplace_back(turn, last_turn, freq[0], freq[1],
            // freq[2],
            //                         freq[3], freq[4]);
            break;
        }
        auto& estimator    = estimators[round];
        const double score = estimator.estimate(
            turn + 1, last_turn, current_money - nc.cards[nc_pos].cost,
            current_scale, h, f);
        total += score;
        ucb.update(arm, score);
        if (ucb.check_early_stop(c)) {
            // cout << "# early stop" << i << "/" << tries << "\n";
            break;
        }
    }


    // cout << "# (turn, money, scale, best_score) = (" << turn << ", "
    //      << current_money << ", " << current_scale << ", "
    //      << ucb.average(ucb.best_arm()) << ")"
    //      << "\n";
    // cout << "# ";
    // for (int i = 0; i < (int)candidates.size(); ++i) {
    //     cout << ucb.average(i) << " ";
    // }
    // cout << "\n# ";
    // for (int i = 0; i < (int)candidates.size(); ++i) {
    //     cout << ucb.count(i) << " ";
    // }
    // cout << "->" << ucb.best_arm();
    // cout << endl;
    total_ms_pick_card +=
        duration_cast<milliseconds>(high_resolution_clock::now() - now).count();
    pick_card_call_num++;

    return candidates[ucb.best_arm()];
}


int run() {
    using namespace input;
    int64_t current_money = 0;
    int current_scale     = 0;
    int64_t freq[5]       = {0, 0, 0, 0, 0};
    // CardType last_used    = SCALE_UP;
    for (int turn = 0; turn < T; ++turn) {
        using namespace std::chrono;
        // auto now = high_resolution_clock::now();
        // Estimator estimator(turn, freq[0], freq[1], freq[2], freq[3],
        // freq[4]); const double estimated_money =
        //     estimator.estimate(turn, T, current_money, current_scale,
        //     hand, field);
        // cerr << "(turn, money, scale, est) = (" << turn << ", " <<
        // current_money
        //      << ", " << current_scale << ", " << estimated_money << ")"
        //      << endl;
        auto [use_pos, mountain_pos] =
            use_card_greedy(hand, field, current_money, current_scale);
        io::output_use_card(use_pos, mountain_pos);
        if (0) switch (hand.cards[use_pos].type) {
                case WORK_ONE:
                    cout << "# decrease height " << mountain_pos << " "
                         << field.mountains[mountain_pos].height << " -> "
                         << field.mountains[mountain_pos].height
                                - hand.cards[use_pos].work_amount
                         << endl;
                    break;
                case WORK_ALL:
                    cout << "# decrease height all "
                         << hand.cards[use_pos].work_amount << endl;
                    break;
                case DELETE_ONE:
                    cout << "# delete mountain " << mountain_pos << endl;
                    break;
                case DELETE_ALL:
                    cout << "# delete all mountains" << endl;
                    break;
                case SCALE_UP:
                    cout << "# scale up " << current_scale << " -> "
                         << current_scale + 1 << endl;
                    break;
                default:
                    assert(false);
            }
        // last_used = hand.cards[use_pos].type;
        update_field(field, hand.cards[use_pos], mountain_pos, current_money,
                     current_scale);
        int64_t old_money = current_money;
        io::input_next(current_money, field, next_cards);
        if (old_money != current_money) {
            cerr << "current_money = " << current_money << endl;
            cerr << "old_money = " << old_money << endl;
            exit(1);
        }
        for (int i = 0; i < next_cards.k; ++i) {
            freq[next_cards.cards[i].type]++;
        }
        const auto current_elapsed =
            duration_cast<milliseconds>(high_resolution_clock::now()
                                        - start_time)
                .count();

        bool do_full_search =
            (current_elapsed < TIME_LIMIT_MS - 10)
            && (turn <= SETUP_TURN || turn >= T - TEARDOWN_TURN);
        if (!do_full_search && current_elapsed < TIME_LIMIT_MS - 20) {
            if (pick_card_call_num == 0) pick_card_call_num = 1;
            avg_ms_pick_card = total_ms_pick_card / (double)pick_card_call_num;
            if (avg_ms_pick_card < 1) avg_ms_pick_card = 1;
            auto rest_call_num =
                (TIME_LIMIT_MS - 10 - current_elapsed) / avg_ms_pick_card
                - TEARDOWN_TURN;
            double prob    = rest_call_num / (T - TEARDOWN_TURN - turn);
            do_full_search = xorshift::getDouble() < prob;
        }
        if (turn < T - 1) {
            auto pick_pos =
                do_full_search
                    ? pick_card(hand, use_pos, field, next_cards, current_money,
                                current_scale, turn, freq)
                    : pick_card_greedy(hand, next_cards, current_money,
                                       current_scale, turn);

            io::output_pick_card(next_cards.cards[pick_pos].id);
            current_money -= next_cards.cards[pick_pos].cost;
            int old_id             = hand.cards[use_pos].id;
            hand.cards[use_pos]    = next_cards.cards[pick_pos];
            hand.cards[use_pos].id = old_id;
        }
        else {
            io::output_pick_card(0); // 最後はコスト0のカードを選ぶ
        }
        // cerr << "turn = " << turn << ", ms = "
        //      << duration_cast<milliseconds>(high_resolution_clock::now()
        //      - now)
        //             .count()
        //      << endl;
    }
    double freq_tot = freq[0] + freq[1] + freq[2] + freq[3] + freq[4];
    logger::push("prob(0)", freq[0] / freq_tot);
    logger::push("prob(1)", freq[1] / freq_tot);
    logger::push("prob(2)", freq[2] / freq_tot);
    logger::push("prob(3)", freq[3] / freq_tot);
    logger::push("prob(4)", freq[4] / freq_tot);
    return current_money;
}

int main() {
    using namespace std::chrono;
    using namespace input;
    start_time = high_resolution_clock::now();
    io::input_first(hand, field, next_cards);

    int64_t score = run();

    int64_t elapsed =
        duration_cast<milliseconds>(high_resolution_clock::now() - start_time)
            .count();
    logger::push("time", elapsed);
    logger::push("full_search_called", pick_card_call_num);
    logger::push("score", score);
    logger::flush();
    return 0;
}
