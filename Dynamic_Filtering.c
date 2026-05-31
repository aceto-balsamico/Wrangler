#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//---------------------------------------------
//  外れ値判定方式（AST）で使用する定数
//---------------------------------------------

// 初期学習期間（この期間は Spec を固定値にする）
static const long INITIAL_SAMPLE_THRESHOLD = 50;

// 初期 Spec（学習前の暫定値）
static const double INITIAL_SPEC_VALUE = 300.0;

// 外れ値判定に使うシグマ倍率（3σ）
static const double SIGMA_MULTIPLIER = 3.0;

// 標準偏差の下限（σ が小さすぎると過剰反応するため）
static const double MINIMUM_SIGMA = 1.0;


//---------------------------------------------
//  統計情報を保持する構造体
//---------------------------------------------
typedef struct
{
    long sample_count;            // これまでに観測したサンプル数
    double mean_value;            // 平均値 μ
    double M2_accumulator;        // 分散計算用の累積値（Welford法）
    double standard_deviation;    // 標準偏差 σ
    double dynamic_spec_threshold;// μ + kσ または 初期Spec
} StatisticalState;


//---------------------------------------------
//  新しいサンプル x を統計状態に反映する（Welford法）
//---------------------------------------------
void update_statistical_state(StatisticalState *state, double new_sample)
{
    // サンプル数を更新
    state->sample_count++;

    // 平均値の更新（Welford法）
    double delta_from_mean = new_sample - state->mean_value;
    state->mean_value += delta_from_mean / state->sample_count;

    // 分散計算用 M2 の更新
    double delta_after_mean_update = new_sample - state->mean_value;
    state->M2_accumulator += delta_from_mean * delta_after_mean_update;

    // 標準偏差の計算（サンプル数が2以上のとき）
    if (state->sample_count > 1)
    {
        double variance = state->M2_accumulator / (state->sample_count - 1);
        if (variance < 0.0)
            variance = 0.0; // 浮動小数点誤差対策

        state->standard_deviation = sqrt(variance);
    }
    else
    {
        state->standard_deviation = 0.0;
    }

    //---------------------------------------------
    //  Spec（外れ値判定閾値）の決定
    //---------------------------------------------
    if (state->sample_count < INITIAL_SAMPLE_THRESHOLD)
    {
        // 初期学習期間中は固定Specを使用
        state->dynamic_spec_threshold = INITIAL_SPEC_VALUE;
    }
    else
    {
        // 標準偏差が小さすぎる場合は下限値を適用
        double sigma_for_spec = state->standard_deviation;
        if (sigma_for_spec < MINIMUM_SIGMA)
            sigma_for_spec = MINIMUM_SIGMA;

        // μ + kσ の計算
        state->dynamic_spec_threshold =
            state->mean_value + SIGMA_MULTIPLIER * sigma_for_spec;
    }
}


int main()
{
    StatisticalState st = {0};
    srand((unsigned)time(NULL));

    // ループ回数
    const int LOOP = 200;

    for (int i = 1; i <= LOOP; i++)
    {
        // Failbit をランダム生成（例：0〜60）
        double x = rand() % 61;

        update_statistical_state(&st, x);

        // 10回ごとに状態を出力
        if (i % 10 == 0)
        {
            printf("n=%ld  mean=%.2f  sigma=%.2f  spec=%.2f  last_x=%.0f\n",
                   st.sample_count, st.mean_value, st.standard_deviation, st.dynamic_spec_threshold, x);
        }
    }

    return 0;
}
