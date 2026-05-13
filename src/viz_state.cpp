#include "engine/viz_state.h"
#include "engine/black_scholes.h"
#include "engine/analytical_greeks.h"
#include "engine/option.h"
#include "engine/pay_off.h"
#include "engine/pricing_result.h"
#include "engine/monte_carlo_pricer.h"
#include "engine/random_gen.h"

#include "imgui.h"
#include "implot.h"
#include "implot3d.h"

#include <algorithm>
#include <cmath>
#include <vector>
#include <cstdio>

namespace {

using engine::Option;
using engine::PayOffCall;
using engine::PayOffPut;
using engine::MarketData;
using engine::PricingResult;

PricingResult price_with_greeks_at(float S, float K, float r, float sigma, float T, bool is_put) {
    MarketData md{S, r, sigma};
    if (is_put) {
        PayOffPut po(K);
        Option opt(K, T, po);
        return engine::black_scholes_put_with_greeks(opt, md);
    } else {
        PayOffCall po(K);
        Option opt(K, T, po);
        return engine::black_scholes_call_with_greeks(opt, md);
    }
}

double price_at(float S, float K, float r, float sigma, float T, bool is_put) {
    MarketData md{S, r, sigma};
    if (is_put) {
        PayOffPut po(K);
        Option opt(K, T, po);
        return engine::black_scholes_put(opt, md).price;
    } else {
        PayOffCall po(K);
        Option opt(K, T, po);
        return engine::black_scholes_call(opt, md).price;
    }
}

double greek_at(int which, float S, float K, float r, float sigma, float T, bool is_put) {
    MarketData md{S, r, sigma};
    PayOffCall pc(K);  PayOffPut pp(K);
    Option opt_c(K, T, pc); Option opt_p(K, T, pp);
    const Option& opt = is_put ? opt_p : opt_c;
    switch (which) {
        case 1: return is_put ? engine::analytical_delta_put(opt, md) : engine::analytical_delta_call(opt, md);
        case 2: return is_put ? engine::analytical_gamma_put(opt, md) : engine::analytical_gamma_call(opt, md);
        case 3: return is_put ? engine::analytical_vega_put (opt, md) : engine::analytical_vega_call (opt, md);
        case 4: return is_put ? engine::analytical_theta_put(opt, md) : engine::analytical_theta_call(opt, md);
        case 5: return is_put ? engine::analytical_rho_put  (opt, md) : engine::analytical_rho_call  (opt, md);
        default: return price_at(S, K, r, sigma, T, is_put);
    }
}

void run_mc_sweep(const engine::VizState& s, engine::McConvergenceData& out) {
    const std::vector<double> Ns = {1'000.0, 10'000.0, 100'000.0, 1'000'000.0};
    out.N.clear(); out.plain_se.clear(); out.anti_se.clear();
    out.plain_price.clear(); out.anti_price.clear();

    out.bs_price = price_at(s.S, s.K, s.r, s.sigma, s.T, s.is_put);

    PayOffCall pc(s.K); PayOffPut pp(s.K);
    Option opt_c(s.K, s.T, pc); Option opt_p(s.K, s.T, pp);
    const Option& opt = s.is_put ? opt_p : opt_c;
    MarketData md{s.S, s.r, s.sigma};

    for (double n : Ns) {
        const std::size_t N = static_cast<std::size_t>(n);

        engine::MersenneTwisterGen rng_p(42);
        engine::MonteCarloPricer pricer_p(rng_p, N);
        const auto rp = pricer_p.price(opt, md);

        engine::MersenneTwisterGen rng_a(42);
        engine::AntitheticGen      anti(rng_a);
        engine::MonteCarloPricer   pricer_a(anti, N);
        const auto ra = pricer_a.price(opt, md);

        out.N.push_back(n);
        out.plain_se.push_back(rp.standard_error);
        out.anti_se.push_back(ra.standard_error);
        out.plain_price.push_back(rp.price);
        out.anti_price.push_back(ra.price);
    }
    out.has_run = true;
}

void draw_greeks_vs_spot(const engine::VizState& s) {
    const int N = std::max(2, s.spot_grid_n);
    const float lo = 0.25f * s.K;
    const float hi = 1.75f * s.K;

    std::vector<double> spot(N), delta(N), gamma(N), vega(N), theta(N), rho(N), price(N);
    for (int i = 0; i < N; ++i) {
        const float S = lo + (hi - lo) * (float)i / (float)(N - 1);
        spot[i] = S;
        const auto pr = price_with_greeks_at(S, s.K, s.r, s.sigma, s.T, s.is_put);
        price[i] = pr.price;
        if (pr.greeks) {
            delta[i] = pr.greeks->delta;
            gamma[i] = pr.greeks->gamma;
            vega[i]  = pr.greeks->vega;
            theta[i] = pr.greeks->theta;
            rho[i]   = pr.greeks->rho;
        }
    }

    auto subplot = [&](const char* title, const std::vector<double>& y) {
        if (ImPlot::BeginPlot(title, ImVec2(-1, 180), ImPlotFlags_NoLegend)) {
            ImPlot::SetupAxes("S", "", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
            ImPlot::PlotLine("##v", spot.data(), y.data(), N);
            const double xs[2] = {(double)s.S, (double)s.S};
            const double ys[2] = {*std::min_element(y.begin(), y.end()),
                                  *std::max_element(y.begin(), y.end())};
            ImPlot::PushStyleColor(ImPlotCol_Line, IM_COL32(255, 200, 80, 200));
            ImPlot::PlotLine("##now", xs, ys, 2);
            ImPlot::PopStyleColor();
            ImPlot::EndPlot();
        }
    };

    subplot("price",  price);
    subplot("delta",  delta);
    subplot("gamma",  gamma);
    subplot("vega",   vega);
    subplot("theta",  theta);
    subplot("rho",    rho);
}

struct SurfaceGrid {
    int N = 0;
    float spot_lo = 0.0f, spot_hi = 0.0f;
    float t_lo = 0.0f, t_hi = 0.0f;
    std::vector<double> values;       // size N*N, indexed [row*N + col], row = T, col = S
    double vmin = 0.0, vmax = 0.0;
    const char* metric_name = "price";
};

SurfaceGrid build_surface_grid(const engine::VizState& s) {
    SurfaceGrid g;
    g.N = std::max(8, s.surface_n);
    g.spot_lo = 0.5f * s.K;
    g.spot_hi = 1.5f * s.K;
    g.t_lo = 0.01f;
    g.t_hi = std::max(0.05f, 2.0f * s.T);
    const char* metric_names[] = {"price","delta","gamma","vega","theta","rho"};
    g.metric_name = metric_names[s.surface_metric];

    g.values.assign(g.N * g.N, 0.0);
    g.vmin = 1e300; g.vmax = -1e300;
    for (int j = 0; j < g.N; ++j) {
        const float T = g.t_lo + (g.t_hi - g.t_lo) * (float)j / (float)(g.N - 1);
        for (int i = 0; i < g.N; ++i) {
            const float S = g.spot_lo + (g.spot_hi - g.spot_lo) * (float)i / (float)(g.N - 1);
            const double v = greek_at(s.surface_metric, S, s.K, s.r, s.sigma, T, s.is_put);
            g.values[j * g.N + i] = v;
            if (v < g.vmin) g.vmin = v;
            if (v > g.vmax) g.vmax = v;
        }
    }
    return g;
}

void draw_surface_metric_combo(engine::VizState& s) {
    const char* metrics[] = {"price","delta","gamma","vega","theta","rho"};
    ImGui::SetNextItemWidth(160);
    ImGui::Combo("metric", &s.surface_metric, metrics, IM_ARRAYSIZE(metrics));
}

void draw_surface(engine::VizState& s) {
    draw_surface_metric_combo(s);
    const SurfaceGrid g = build_surface_grid(s);

    ImPlot::PushColormap(ImPlotColormap_Plasma);
    if (ImPlot::BeginPlot("##surface", ImVec2(-1, -1))) {
        ImPlot::SetupAxes("Spot S", "Time T",
                          ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        ImPlot::PlotHeatmap("##h", g.values.data(), g.N, g.N,
                            g.vmin, g.vmax, nullptr,
                            ImPlotPoint(g.spot_lo, g.t_lo),
                            ImPlotPoint(g.spot_hi, g.t_hi));
        ImPlot::EndPlot();
    }
    ImGui::SameLine();
    ImPlot::ColormapScale("##scale", g.vmin, g.vmax, ImVec2(80, -1));
    ImPlot::PopColormap();
}

void draw_surface_3d(engine::VizState& s) {
    draw_surface_metric_combo(s);
    ImGui::SameLine();
    ImGui::TextDisabled("(drag to rotate, scroll to zoom)");

    const SurfaceGrid g = build_surface_grid(s);

    // ImPlot3D::PlotSurface wants three flat N*N arrays of equal length.
    const int M = g.N * g.N;
    std::vector<float> xs(M), ys(M), zs(M);
    for (int j = 0; j < g.N; ++j) {
        const float T = g.t_lo + (g.t_hi - g.t_lo) * (float)j / (float)(g.N - 1);
        for (int i = 0; i < g.N; ++i) {
            const float S = g.spot_lo + (g.spot_hi - g.spot_lo) * (float)i / (float)(g.N - 1);
            const int idx = j * g.N + i;
            xs[idx] = S;
            ys[idx] = T;
            zs[idx] = (float)g.values[idx];
        }
    }

    ImPlot3D::PushColormap(ImPlot3DColormap_Plasma);
    if (ImPlot3D::BeginPlot("##surface3d", ImVec2(-1, -1), ImPlot3DFlags_NoClip)) {
        ImPlot3D::SetupAxes("Spot S", "Time T", g.metric_name);
        ImPlot3D::SetupAxesLimits(g.spot_lo, g.spot_hi,
                                  g.t_lo,    g.t_hi,
                                  g.vmin,    g.vmax);
        ImPlot3DSurfaceFlags flags = ImPlot3DSurfaceFlags_NoMarkers;
        ImPlot3DSpec spec;
        spec.Flags = flags;
        spec.FillAlpha = 0.85f;
        ImPlot3D::PlotSurface("surface", xs.data(), ys.data(), zs.data(),
                              g.N, g.N, g.vmin, g.vmax, spec);
        ImPlot3D::EndPlot();
    }
    ImPlot3D::PopColormap();
}

void draw_payoff(const engine::VizState& s) {
    const int N = 200;
    const float lo = 0.25f * s.K, hi = 1.75f * s.K;
    std::vector<double> spot(N), payoff(N);
    PayOffCall pc(s.K); PayOffPut pp(s.K);
    for (int i = 0; i < N; ++i) {
        const float S = lo + (hi - lo) * (float)i / (float)(N - 1);
        spot[i] = S;
        payoff[i] = s.is_put ? pp.compute(S) : pc.compute(S);
    }
    if (ImPlot::BeginPlot("Payoff at expiry", ImVec2(-1, -1))) {
        ImPlot::SetupAxes("S_T", "max(0, ...)",
                          ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        ImPlot::PlotLine("intrinsic", spot.data(), payoff.data(), N);
        const double xk[2] = {s.K, s.K};
        const double yk[2] = {0.0, *std::max_element(payoff.begin(), payoff.end())};
        ImPlot::PushStyleColor(ImPlotCol_Line, IM_COL32(180, 180, 180, 160));
        ImPlot::PlotLine("strike", xk, yk, 2);
        ImPlot::PopStyleColor();
        ImPlot::EndPlot();
    }
}

void draw_mc_convergence(const engine::McConvergenceData& mc) {
    if (!mc.has_run) {
        ImGui::TextDisabled("Click 'Run MC sweep' on the left.");
        return;
    }
    if (ImPlot::BeginPlot("MC standard error vs N", ImVec2(-1, -1))) {
        ImPlot::SetupAxes("N (paths)", "Standard error");
        ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
        ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
        ImPlot::PlotLine("plain",      mc.N.data(), mc.plain_se.data(),  (int)mc.N.size());
        ImPlot::PlotLine("antithetic", mc.N.data(), mc.anti_se.data(),   (int)mc.N.size());
        // Theoretical -1/2 slope through the first plain point
        if (!mc.N.empty()) {
            std::vector<double> ref(mc.N.size());
            const double c = mc.plain_se[0] * std::sqrt(mc.N[0]);
            for (size_t i = 0; i < mc.N.size(); ++i) ref[i] = c / std::sqrt(mc.N[i]);
            ImPlot::PushStyleColor(ImPlotCol_Line, IM_COL32(180, 180, 180, 120));
            ImPlot::PlotLine("1/sqrt(N) ref", mc.N.data(), ref.data(), (int)mc.N.size());
            ImPlot::PopStyleColor();
        }
        ImPlot::EndPlot();
    }
    ImGui::Text("BS analytical price: %.6f", mc.bs_price);
    for (size_t i = 0; i < mc.N.size(); ++i) {
        ImGui::Text("N=%-9.0f  plain=%.6f (SE %.3e)   anti=%.6f (SE %.3e)",
                    mc.N[i], mc.plain_price[i], mc.plain_se[i],
                    mc.anti_price[i], mc.anti_se[i]);
    }
}

} // namespace

namespace engine {

void draw_ui(VizState& state, McConvergenceData& mc) {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::Begin("##main", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    if (ImGui::BeginTable("layout", 2,
            ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn("controls", ImGuiTableColumnFlags_WidthFixed, 340);
        ImGui::TableSetupColumn("plots",    ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow();

        // ---------- LEFT COLUMN: parameters + readout + mc button ----------
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Parameters");
        ImGui::Separator();
        ImGui::SliderFloat("Spot (S)",   &state.S,     1.0f, 300.0f, "%.2f");
        ImGui::SliderFloat("Strike (K)", &state.K,     1.0f, 300.0f, "%.2f");
        ImGui::SliderFloat("Rate (r)",   &state.r,    -0.05f, 0.20f, "%.4f");
        ImGui::SliderFloat("Vol (sigma)",&state.sigma, 0.01f, 1.50f, "%.4f");
        ImGui::SliderFloat("Expiry (T)", &state.T,     0.01f, 3.00f, "%.4f");
        const char* types[] = {"Call", "Put"};
        int t = state.is_put ? 1 : 0;
        if (ImGui::Combo("Type", &t, types, IM_ARRAYSIZE(types))) state.is_put = (t == 1);
        if (ImGui::Button("Reset to Hull Set 1")) {
            state.S = 42.0f; state.K = 40.0f; state.r = 0.10f;
            state.sigma = 0.20f; state.T = 0.5f; state.is_put = false;
        }

        ImGui::Spacing(); ImGui::Separator();
        ImGui::TextUnformatted("Live readout (analytical)");
        ImGui::Separator();
        const auto pr = price_with_greeks_at(state.S, state.K, state.r,
                                             state.sigma, state.T, state.is_put);
        ImGui::Text("Price : %12.6f", pr.price);
        if (pr.greeks) {
            ImGui::Text("Delta : %12.6f", pr.greeks->delta);
            ImGui::Text("Gamma : %12.6f", pr.greeks->gamma);
            ImGui::Text("Vega  : %12.6f", pr.greeks->vega);
            ImGui::Text("Theta : %12.6f", pr.greeks->theta);
            ImGui::Text("Rho   : %12.6f", pr.greeks->rho);
        }
        const double intrinsic = state.is_put
            ? std::max((double)state.K - state.S, 0.0)
            : std::max((double)state.S - state.K, 0.0);
        ImGui::Text("Intrinsic: %9.6f", intrinsic);
        ImGui::Text("Time val : %9.6f", pr.price - intrinsic);

        ImGui::Spacing(); ImGui::Separator();
        ImGui::TextUnformatted("Monte Carlo");
        ImGui::Separator();
        if (ImGui::Button("Run MC sweep (N up to 1M)")) {
            run_mc_sweep(state, mc);
        }
        if (mc.has_run) ImGui::TextDisabled("done");

        ImGui::Spacing(); ImGui::Separator();
        ImGui::TextUnformatted("Plot resolution");
        ImGui::SliderInt("spot pts",   &state.spot_grid_n, 20, 400);
        ImGui::SliderInt("surface NxN",&state.surface_n,    8, 120);

        // ---------- RIGHT COLUMN: tabbed plots ----------
        ImGui::TableSetColumnIndex(1);
        if (ImGui::BeginTabBar("plots")) {
            if (ImGui::BeginTabItem("Greeks vs spot")) {
                draw_greeks_vs_spot(state);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Surface (heatmap)")) {
                draw_surface(state);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Surface (3D)")) {
                draw_surface_3d(state);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Payoff at expiry")) {
                draw_payoff(state);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("MC convergence")) {
                draw_mc_convergence(mc);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::EndTable();
    }
    ImGui::End();
}

} // namespace engine
