#include "catch_amalgamated.hpp"
#include "engine/implied_volatility.h"
#include "engine/black_scholes.h"
#include "engine/option.h"
#include "engine/pay_off.h"
#include <cmath>
#include <stdexcept>

namespace {
    constexpr double K1 = 40.0, T1 = 0.5,         S1 = 42.0, R1 = 0.10, SIGMA1 = 0.20;
    constexpr double K2 = 50.0,                              S2 = 49.0, R2 = 0.05, SIGMA2 = 0.20;
    const     double T2 = 20.0 / 52.0;
}

TEST_CASE("implied_volatility round-trip at Hull Set 1 call",
          "[implied_volatility][round_trip]") {
    engine::PayOffCall  po(K1);
    engine::Option      option(K1, T1, po);
    engine::MarketData  market{S1, R1, SIGMA1};

    const double bs_price = engine::black_scholes_call(option, market).price;
    const double iv = engine::implied_volatility(option, market, bs_price);
    REQUIRE_THAT(iv, Catch::Matchers::WithinRel(SIGMA1, 1e-8));
}

TEST_CASE("implied_volatility round-trip at Hull Set 1 put",
          "[implied_volatility][round_trip]") {
    engine::PayOffPut   po(K1);
    engine::Option      option(K1, T1, po);
    engine::MarketData  market{S1, R1, SIGMA1};

    const double bs_price = engine::black_scholes_put(option, market).price;
    const double iv = engine::implied_volatility(option, market, bs_price);
    REQUIRE_THAT(iv, Catch::Matchers::WithinRel(SIGMA1, 1e-8));
}

TEST_CASE("implied_volatility round-trip at Hull Set 2 call",
          "[implied_volatility][round_trip]") {
    engine::PayOffCall  po(K2);
    engine::Option      option(K2, T2, po);
    engine::MarketData  market{S2, R2, SIGMA2};

    const double bs_price = engine::black_scholes_call(option, market).price;
    const double iv = engine::implied_volatility(option, market, bs_price);
    REQUIRE_THAT(iv, Catch::Matchers::WithinRel(SIGMA2, 1e-8));
}

TEST_CASE("implied_volatility round-trip at Hull Set 2 put",
          "[implied_volatility][round_trip]") {
    engine::PayOffPut   po(K2);
    engine::Option      option(K2, T2, po);
    engine::MarketData  market{S2, R2, SIGMA2};

    const double bs_price = engine::black_scholes_put(option, market).price;
    const double iv = engine::implied_volatility(option, market, bs_price);
    REQUIRE_THAT(iv, Catch::Matchers::WithinRel(SIGMA2, 1e-8));
}

TEST_CASE("implied_volatility robust across sigma_init range",
          "[implied_volatility][robustness]") {
    engine::PayOffCall  po(K1);
    engine::Option      option(K1, T1, po);
    engine::MarketData  market{S1, R1, SIGMA1};
    const double bs_price = engine::black_scholes_call(option, market).price;

    SECTION("sigma_init = 0.05") {
        const double iv = engine::implied_volatility(option, market, bs_price, 0.05);
        REQUIRE_THAT(iv, Catch::Matchers::WithinRel(SIGMA1, 1e-8));
    }
    SECTION("sigma_init = 0.20") {
        const double iv = engine::implied_volatility(option, market, bs_price, 0.20);
        REQUIRE_THAT(iv, Catch::Matchers::WithinRel(SIGMA1, 1e-8));
    }
    SECTION("sigma_init = 0.50") {
        const double iv = engine::implied_volatility(option, market, bs_price, 0.50);
        REQUIRE_THAT(iv, Catch::Matchers::WithinRel(SIGMA1, 1e-8));
    }
    SECTION("sigma_init = 1.00") {
        const double iv = engine::implied_volatility(option, market, bs_price, 1.00);
        REQUIRE_THAT(iv, Catch::Matchers::WithinRel(SIGMA1, 1e-8));
    }
}

TEST_CASE("implied_volatility throws on no-arbitrage violation Hull Set 1",
          "[implied_volatility][edge_case]") {
    engine::PayOffCall  po(K1);
    engine::Option      option(K1, T1, po);
    engine::MarketData  market{S1, R1, 0.0};

    SECTION("price below intrinsic value throws") {
        const double intrinsic = S1 - K1 * std::exp(-R1 * T1);
        REQUIRE_THROWS_AS(
            engine::implied_volatility(option, market, intrinsic - 0.5),
            std::runtime_error);
    }
    SECTION("price above spot (call upper bound) throws") {
        REQUIRE_THROWS_AS(
            engine::implied_volatility(option, market, S1 + 1.0),
            std::runtime_error);
    }
}

TEST_CASE("implied_volatility deep-ITM regime falls back to bisection",
          "[implied_volatility][edge_case]") {
    constexpr double K_OTM = 100.0;
    constexpr double T_OTM = 0.05;
    constexpr double S_OTM = 80.0;
    constexpr double R_OTM = 0.10;
    constexpr double SIGMA_OTM = 0.40;

    engine::PayOffCall  po(K_OTM);
    engine::Option      option(K_OTM, T_OTM, po);
    engine::MarketData  market_with_sigma{S_OTM, R_OTM, SIGMA_OTM};
    const double bs_price = engine::black_scholes_call(option, market_with_sigma).price;

    engine::MarketData  market{S_OTM, R_OTM, 0.0};
    const double iv = engine::implied_volatility(option, market, bs_price, 0.05);
    REQUIRE_THAT(iv, Catch::Matchers::WithinRel(SIGMA_OTM, 1e-6));
}
