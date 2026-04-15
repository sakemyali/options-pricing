#include "catch_amalgamated.hpp"
#include "engine/monte_carlo_pricer.h"
#include "engine/random_gen.h"
#include "engine/black_scholes.h"
#include "engine/option.h"
#include "engine/pay_off.h"
#include <cmath>

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

namespace {
    // Hull Ch 15 / Ch 21 reference parameter set (same as test_black_scholes.cpp).
    constexpr double K_HULL      = 40.0;
    constexpr double T_HULL      = 0.5;
    constexpr double S_HULL      = 42.0;
    constexpr double R_HULL      = 0.10;
    constexpr double SIGMA_HULL  = 0.20;
}

TEST_CASE("MonteCarloPricer call fixed-seed regression", "[monte_carlo]") {
    engine::MersenneTwisterGen rng(42);
    engine::MonteCarloPricer pricer(rng, 10000);

    engine::PayOffCall call_payoff(K_HULL);
    engine::Option option(K_HULL, T_HULL, call_payoff);
    engine::MarketData market{S_HULL, R_HULL, SIGMA_HULL};

    auto result = pricer.price(option, market);
    // Regression anchor: seed=42, N=10000, Hull Set 1.
    // Bitwise-reproducible on same machine (Marsaglia polar + deterministic mt19937_64).
    REQUIRE_THAT(result.price, WithinRel(4.8278543443124411, 1e-10));
    REQUIRE(result.standard_error > 0.0);
    REQUIRE(result.path_count == 10000);
    REQUIRE_FALSE(result.greeks.has_value());
}

TEST_CASE("MonteCarloPricer put fixed-seed regression", "[monte_carlo]") {
    engine::MersenneTwisterGen rng(42);
    engine::MonteCarloPricer pricer(rng, 10000);

    engine::PayOffPut put_payoff(K_HULL);
    engine::Option option(K_HULL, T_HULL, put_payoff);
    engine::MarketData market{S_HULL, R_HULL, SIGMA_HULL};

    auto result = pricer.price(option, market);
    // Regression anchor: seed=42, N=10000, Hull Set 1.
    REQUIRE_THAT(result.price, WithinRel(0.78070493191468437, 1e-10));
    REQUIRE(result.standard_error > 0.0);
    REQUIRE(result.path_count == 10000);
    REQUIRE_FALSE(result.greeks.has_value());
}

TEST_CASE("MonteCarloPricer call converges to Black-Scholes within 3*SE", "[monte_carlo][convergence]") {
    engine::MersenneTwisterGen rng(42);
    engine::MonteCarloPricer pricer(rng, 100000);

    engine::PayOffCall call_payoff(K_HULL);
    engine::Option option(K_HULL, T_HULL, call_payoff);
    engine::MarketData market{S_HULL, R_HULL, SIGMA_HULL};

    auto mc = pricer.price(option, market);
    auto bs = engine::black_scholes_call(option, market);

    REQUIRE(std::abs(mc.price - bs.price) <= 3.0 * mc.standard_error);
}

TEST_CASE("MonteCarloPricer put converges to Black-Scholes within 3*SE", "[monte_carlo][convergence]") {
    engine::MersenneTwisterGen rng(42);
    engine::MonteCarloPricer pricer(rng, 100000);

    engine::PayOffPut put_payoff(K_HULL);
    engine::Option option(K_HULL, T_HULL, put_payoff);
    engine::MarketData market{S_HULL, R_HULL, SIGMA_HULL};

    auto mc = pricer.price(option, market);
    auto bs = engine::black_scholes_put(option, market);

    REQUIRE(std::abs(mc.price - bs.price) <= 3.0 * mc.standard_error);
}

TEST_CASE("MonteCarloPricer sign and sentinel invariants", "[monte_carlo]") {
    engine::MersenneTwisterGen rng(123);
    engine::MonteCarloPricer pricer(rng, 5000);

    engine::PayOffPut put_payoff(100.0);
    engine::Option option(100.0, 1.0, put_payoff);
    engine::MarketData market{100.0, 0.05, 0.25};

    auto result = pricer.price(option, market);
    REQUIRE(result.price >= 0.0);
    REQUIRE(result.standard_error >= 0.0);
    REQUIRE(result.path_count == 5000);
    REQUIRE_FALSE(result.greeks.has_value());
}

TEST_CASE("MonteCarloPricer accepts RandomGen& injection", "[monte_carlo]") {
    engine::MersenneTwisterGen concrete(42);
    engine::RandomGen& base = concrete;
    engine::MonteCarloPricer pricer(base, 1000);

    engine::PayOffCall call_payoff(K_HULL);
    engine::Option option(K_HULL, T_HULL, call_payoff);
    engine::MarketData market{S_HULL, R_HULL, SIGMA_HULL};

    auto result = pricer.price(option, market);
    REQUIRE(result.path_count == 1000);
}
