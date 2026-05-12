#ifndef ENGINE_VIZ_STATE_H
#define ENGINE_VIZ_STATE_H

#include <vector>

namespace engine {

struct VizState {
    float S     = 100.0f;
    float K     = 100.0f;
    float r     = 0.05f;
    float sigma = 0.25f;
    float T     = 1.0f;
    bool  is_put = false;

    int spot_grid_n = 120;   // points for Greek-vs-spot sweep
    int surface_n   = 60;    // grid resolution for the heatmap
    int surface_metric = 0;  // 0 price, 1 delta, 2 gamma, 3 vega, 4 theta, 5 rho

    int mc_paths_max = 1'000'000;
};

struct McConvergenceData {
    bool has_run = false;
    std::vector<double> N;
    std::vector<double> plain_se;
    std::vector<double> anti_se;
    std::vector<double> plain_price;
    std::vector<double> anti_price;
    double bs_price = 0.0;
};

void draw_ui(VizState& state, McConvergenceData& mc);

} // namespace engine

#endif // ENGINE_VIZ_STATE_H
