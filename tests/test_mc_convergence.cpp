#include "catch_amalgamated.hpp"
#include "engine/monte_carlo_pricer.h"
#include "engine/random_gen.h"
#include "engine/black_scholes.h"
#include "engine/option.h"
#include "engine/pay_off.h"
#include <cmath>
#include <cstdint>
#include <vector>

namespace {
    // Hull Ch 15 / Ch 21 reference parameter set.
    // Duplicated from tests/test_monte_carlo_pricer.cpp and tests/test_black_scholes.cpp
    // intentionally -- each test file is self-contained (no cross-test-file fixtures).
    constexpr double        K_HULL     = 40.0;
    constexpr double        T_HULL     = 0.5;
    constexpr double        S_HULL     = 42.0;
    constexpr double        R_HULL     = 0.10;
    constexpr double        SIGMA_HULL = 0.20;
    constexpr std::uint64_t SEED       = 42;

    // 2-variable least-squares slope of ys on xs. No external deps.
    // Source: https://en.wikipedia.org/wiki/Simple_linear_regression
    // For (log N, log SE) points, the expected slope is -0.5 under the theoretical
    // O(1/sqrt(N)) convergence of standard Monte Carlo standard error:
    //   SE(N) = C / sqrt(N)  =>  log SE = log C - 0.5 * log N
    double log_log_slope(const std::vector<double>& xs,
                         const std::vector<double>& ys) {
        const std::size_t k = xs.size();
        double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_xx = 0.0;
        for (std::size_t i = 0; i < k; ++i) {
            sum_x  += xs[i];
            sum_y  += ys[i];
            sum_xy += xs[i] * ys[i];
            sum_xx += xs[i] * xs[i];
        }
        const double k_d = static_cast<double>(k);
        return (k_d * sum_xy - sum_x * sum_y)
             / (k_d * sum_xx - sum_x * sum_x);
    }
}

TEST_CASE("MonteCarloPricer call within 3-sigma of Black-Scholes at N=1M",
          "[mc_convergence][.slow]") {
    // Central limit theorem: |MC - BS| / SE is approximately N(0,1) for an unbiased MC
    // estimator. At |z| < 3 we reject fewer than 0.27% of legitimate runs (two-sided).
    // With fixed seed=42 the realisation is deterministic -- pass/fail is stable across
    // reruns on the same machine (Marsaglia polar + mt19937_64 are bit-reproducible).
    engine::MersenneTwisterGen rng(SEED);
    engine::MonteCarloPricer   pricer(rng, 1'000'000);

    engine::PayOffCall call_payoff(K_HULL);
    engine::Option     option(K_HULL, T_HULL, call_payoff);
    engine::MarketData market{S_HULL, R_HULL, SIGMA_HULL};

    auto mc = pricer.price(option, market);
    auto bs = engine::black_scholes_call(option, market);

    REQUIRE(std::abs(mc.price - bs.price) <= 3.0 * mc.standard_error);
    REQUIRE(mc.path_count == 1'000'000);
    REQUIRE(mc.standard_error > 0.0);
}

TEST_CASE("MonteCarloPricer put within 3-sigma of Black-Scholes at N=1M",
          "[mc_convergence][.slow]") {
    engine::MersenneTwisterGen rng(SEED);
    engine::MonteCarloPricer   pricer(rng, 1'000'000);

    engine::PayOffPut  put_payoff(K_HULL);
    engine::Option     option(K_HULL, T_HULL, put_payoff);
    engine::MarketData market{S_HULL, R_HULL, SIGMA_HULL};

    auto mc = pricer.price(option, market);
    auto bs = engine::black_scholes_put(option, market);

    REQUIRE(std::abs(mc.price - bs.price) <= 3.0 * mc.standard_error);
    REQUIRE(mc.path_count == 1'000'000);
    REQUIRE(mc.standard_error > 0.0);
}

TEST_CASE("MonteCarloPricer standard error decays as O(1/sqrt(N))",
          "[mc_convergence][.slow]") {
    // Sweep N across 3 decades -- minimum log-lever for a well-determined 4-point slope.
    // Theoretical slope on log(SE) vs log(N) is exactly -0.5.
    const std::vector<std::size_t> path_counts = {
        1'000, 10'000, 100'000, 1'000'000
    };
    std::vector<double> log_n;
    std::vector<double> log_se;

    engine::PayOffCall call_payoff(K_HULL);
    engine::Option     option(K_HULL, T_HULL, call_payoff);
    engine::MarketData market{S_HULL, R_HULL, SIGMA_HULL};

    for (std::size_t N : path_counts) {
        // Fresh RNG per N -- each (N, SE) measurement starts from the SAME deterministic
        // mt19937_64 state. seed() invalidates the Marsaglia cache, so reconstructing
        // is equivalent to rng.seed(SEED) but clearer.
        engine::MersenneTwisterGen rng(SEED);
        engine::MonteCarloPricer   pricer(rng, N);
        auto mc = pricer.price(option, market);

        REQUIRE(mc.standard_error > 0.0);  // guard std::log(0) -- theoretical only

        log_n.push_back(std::log(static_cast<double>(N)));
        log_se.push_back(std::log(mc.standard_error));
    }

    const double slope = log_log_slope(log_n, log_se);

    // Primary gate: slope of log(SE) on log(N) must be in [-0.6, -0.4].
    // Theoretical -0.5; +/-0.1 absorbs 4-point finite-sample noise at seed=42 AND
    // libm ULP differences across platforms.
    // Tight enough to catch a catastrophic rate bug (O(1) would give slope ~0,
    // O(1/N) would give slope ~-1 -- both many std devs outside [-0.6, -0.4]).
    REQUIRE(slope < -0.4);
    REQUIRE(slope > -0.6);

    // Belt-and-braces: per-consecutive-pair ratio SE(10N)/SE(N) should approx 1/sqrt(10).
    // Catches a single-pair violation the full-range slope estimator might average over.
    // +/-30% tolerance because per-pair SE-of-SE variance is larger than the 4-point
    // slope estimator variance.
    for (std::size_t i = 1; i < path_counts.size(); ++i) {
        const double ratio    = std::exp(log_se[i] - log_se[i - 1]);
        const double expected = 1.0 / std::sqrt(10.0);  // approx 0.3162
        REQUIRE(ratio > 0.7 * expected);
        REQUIRE(ratio < 1.3 * expected);
    }
}
