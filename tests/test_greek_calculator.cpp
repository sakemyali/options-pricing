// Source: Standard pedagogical FD test -- f(S)=S^2 has analytical d/dS = 2S.
// Pattern matches established Catch2 idioms from tests/test_analytical_greeks.cpp.
#include "catch_amalgamated.hpp"
#include "engine/greek_calculator.h"
#include "engine/option.h"
#include "engine/pay_off.h"

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

TEST_CASE("GreekCalculator delta matches analytic d/dS of f(S)=S^2",
          "[fd_greeks][greek_calculator]") {
    auto pricer = [](const engine::Option&, const engine::MarketData& m) {
        return m.spot * m.spot;   // f(S) = S^2
    };
    engine::GreekCalculator gc(pricer);
    engine::PayOffCall payoff(40.0);
    engine::Option option(40.0, 0.5, payoff);
    engine::MarketData market{42.0, 0.10, 0.20};

    // Analytic d/dS [S^2] = 2S = 84.0 at S=42.
    REQUIRE_THAT(gc.delta(option, market), WithinRel(84.0, 1e-10));
}

TEST_CASE("GreekCalculator delta is stable across bump sizes (sensitivity test)",
          "[fd_greeks][greek_calculator]") {
    auto pricer = [](const engine::Option&, const engine::MarketData& m) {
        return m.spot * m.spot;
    };
    engine::PayOffCall payoff(40.0);
    engine::Option option(40.0, 0.5, payoff);
    engine::MarketData market{42.0, 0.10, 0.20};

    for (double rel_h : {1e-2, 1e-3, 1e-4, 1e-5, 1e-6}) {
        engine::BumpSizes bumps; bumps.S = rel_h;
        engine::GreekCalculator gc(pricer, bumps);
        // f(S)=S^2 is exact for central FD (truncation 0); only roundoff at h~1e-6.
        REQUIRE_THAT(gc.delta(option, market), WithinRel(84.0, 1e-6));
    }
}
