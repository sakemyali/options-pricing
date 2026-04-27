// Pricer-agnostic seam: GreekCalculator should work with the MC pricer too,
// not only with the analytical BS pricer. Uses common random numbers (CRN):
// re-seed BEFORE every pricer call so both bumped evaluations consume the
// bit-exact same z_1, ..., z_N sequence. Tolerance is 6*SE (loose vs the
// analytical FD test, which is what enforces tight numerical agreement).
#include "catch_amalgamated.hpp"
#include "engine/analytical_greeks.h"
#include "engine/greek_calculator.h"
#include "engine/monte_carlo_pricer.h"
#include "engine/option.h"
#include "engine/pay_off.h"
#include "engine/random_gen.h"
#include <cmath>
#include <cstdint>

namespace {
    constexpr double        K_HULL = 40.0, T_HULL = 0.5, S_HULL = 42.0;
    constexpr double        R_HULL = 0.10, SIGMA_HULL = 0.20;
    constexpr std::size_t   N_PATHS = 100'000;
    constexpr std::uint64_t SEED    = 42;
}

TEST_CASE("GreekCalculator works with MC pricer (pricer-agnostic seam) using CRN",
          "[fd_mc_greeks][.slow]") {
    engine::MersenneTwisterGen rng(SEED);

    // CRN: re-seed BEFORE every pricer call so both bumped evaluations consume
    // the bit-exact same z_1, ..., z_N sequence. Without this, FD-via-MC is
    // dominated by O(SE) sampling noise and the test is meaningless.
    // seed() resets BOTH the mt19937_64 engine AND the Marsaglia cache.
    auto mc_call_pricer_crn = [&rng](const engine::Option& o, const engine::MarketData& m) {
        rng.seed(SEED);
        engine::MonteCarloPricer pricer(rng, N_PATHS);
        return pricer.price(o, m).price;
    };

    engine::GreekCalculator gc(mc_call_pricer_crn);
    engine::PayOffCall payoff(K_HULL);
    engine::Option option(K_HULL, T_HULL, payoff);
    engine::MarketData market{S_HULL, R_HULL, SIGMA_HULL};

    // Sample SE at canonical parameters once to size the Delta tolerance.
    rng.seed(SEED);
    engine::MonteCarloPricer probe(rng, N_PATHS);
    const double se = probe.price(option, market).standard_error;
    REQUIRE(se > 0.0);

    // Tolerance: 3-sigma * 2 evaluations per Greek. With CRN, the residual is
    // dominated by the small variance contribution that survives cancellation
    // (typically ~SE per Greek; 6*SE is conservative).
    const double tol_delta = 6.0 * se;

    SECTION("Delta within 6*SE of analytical (CRN-controlled)") {
        const double mc_delta = gc.delta(option, market);
        const double an_delta = engine::analytical_delta_call(option, market);
        REQUIRE(std::isfinite(mc_delta));
        REQUIRE(std::abs(mc_delta - an_delta) < tol_delta);
        REQUIRE(mc_delta > 0.0);   // call delta positive
        REQUIRE(mc_delta < 1.0);   // call delta < 1
    }

    SECTION("Vega sign sanity (positive for call)") {
        const double mc_vega = gc.vega(option, market);
        REQUIRE(std::isfinite(mc_vega));
        REQUIRE(mc_vega > 0.0);
    }

    SECTION("Gamma sign sanity (positive for call)") {
        const double mc_gamma = gc.gamma(option, market);
        REQUIRE(std::isfinite(mc_gamma));
        REQUIRE(mc_gamma > 0.0);
    }

    SECTION("Rho sign sanity (positive for call)") {
        const double mc_rho = gc.rho(option, market);
        REQUIRE(std::isfinite(mc_rho));
        REQUIRE(mc_rho > 0.0);
    }

    SECTION("Theta sign sanity (financial -dV/dT < 0 for typical long call)") {
        // gc.theta returns +dV/dT (raw); negate for financial convention.
        const double mc_theta_financial = -gc.theta(option, market);
        REQUIRE(std::isfinite(mc_theta_financial));
        REQUIRE(mc_theta_financial < 0.0);
    }
}
