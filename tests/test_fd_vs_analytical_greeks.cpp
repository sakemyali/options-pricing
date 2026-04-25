// FD-via-BS Greeks should match analytical Greeks to 4+ sig figs.
#include "catch_amalgamated.hpp"
#include "engine/black_scholes.h"
#include "engine/analytical_greeks.h"
#include "engine/greek_calculator.h"
#include "engine/option.h"
#include "engine/pay_off.h"

using Catch::Matchers::WithinRel;

namespace {
    // Hull Parameter Set 1: K=40, T=0.5, S=42, r=0.10, sigma=0.20
    constexpr double K1 = 40.0, T1 = 0.5, S1 = 42.0;
    constexpr double R1 = 0.10, SIGMA1 = 0.20;

    // Hull Parameter Set 2: K=50, T=20/52, S=49, r=0.05, sigma=0.20
    constexpr double K2 = 50.0, S2 = 49.0;
    constexpr double R2 = 0.05, SIGMA2 = 0.20;

    // BS-call pricer adapter: extracts .price from PricingResult to satisfy
    // GreekCalculator's std::function<double(...)> signature.
    auto bs_call_pricer = [](const engine::Option& o, const engine::MarketData& m) {
        return engine::black_scholes_call(o, m).price;
    };
}

TEST_CASE("FD Greeks via BS call match analytical to 4 sig figs at Hull Set 1",
          "[fd_greeks][fd_analytical]") {
    engine::GreekCalculator gc(bs_call_pricer);
    engine::PayOffCall payoff(K1);
    engine::Option option(K1, T1, payoff);
    engine::MarketData market{S1, R1, SIGMA1};

    SECTION("Delta") {
        REQUIRE_THAT(gc.delta(option, market),
                     WithinRel(engine::analytical_delta_call(option, market), 1e-4));
    }
    SECTION("Gamma") {
        REQUIRE_THAT(gc.gamma(option, market),
                     WithinRel(engine::analytical_gamma_call(option, market), 1e-4));
    }
    SECTION("Vega") {
        REQUIRE_THAT(gc.vega(option, market),
                     WithinRel(engine::analytical_vega_call(option, market), 1e-4));
    }
    SECTION("Theta (negate FD to match financial convention -dV/dT)") {
        // GreekCalculator::theta returns +dV/dT; analytical_theta_call returns -dV/dT.
        REQUIRE_THAT(-gc.theta(option, market),
                     WithinRel(engine::analytical_theta_call(option, market), 1e-4));
    }
    SECTION("Rho") {
        REQUIRE_THAT(gc.rho(option, market),
                     WithinRel(engine::analytical_rho_call(option, market), 1e-4));
    }
}

TEST_CASE("FD Greeks via BS call match analytical to 4 sig figs at Hull Set 2",
          "[fd_greeks][fd_analytical]") {
    const double T2 = 20.0 / 52.0;
    engine::GreekCalculator gc(bs_call_pricer);
    engine::PayOffCall payoff(K2);
    engine::Option option(K2, T2, payoff);
    engine::MarketData market{S2, R2, SIGMA2};

    SECTION("Delta") {
        REQUIRE_THAT(gc.delta(option, market),
                     WithinRel(engine::analytical_delta_call(option, market), 1e-4));
    }
    SECTION("Gamma") {
        REQUIRE_THAT(gc.gamma(option, market),
                     WithinRel(engine::analytical_gamma_call(option, market), 1e-4));
    }
    SECTION("Vega") {
        REQUIRE_THAT(gc.vega(option, market),
                     WithinRel(engine::analytical_vega_call(option, market), 1e-4));
    }
    SECTION("Theta (negate FD)") {
        REQUIRE_THAT(-gc.theta(option, market),
                     WithinRel(engine::analytical_theta_call(option, market), 1e-4));
    }
    SECTION("Rho") {
        REQUIRE_THAT(gc.rho(option, market),
                     WithinRel(engine::analytical_rho_call(option, market), 1e-4));
    }
}
