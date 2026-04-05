#include "catch_amalgamated.hpp"
#include "engine/black_scholes.h"
#include "engine/option.h"
#include "engine/pay_off.h"
#include <cmath>

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

TEST_CASE("Black-Scholes d1 and d2", "[black_scholes]") {
    SECTION("d1 for Hull example") {
        double d1 = engine::black_scholes_d1(42.0, 40.0, 0.1, 0.2, 0.5);
        REQUIRE_THAT(d1, WithinRel(0.769262628106032, 1e-10));
    }
    SECTION("d2 for Hull example") {
        double d2 = engine::black_scholes_d2(42.0, 40.0, 0.1, 0.2, 0.5);
        REQUIRE_THAT(d2, WithinRel(0.627841271868722, 1e-10));
    }
    SECTION("d2 equals d1 minus sigma*sqrt(T)") {
        double d1 = engine::black_scholes_d1(42.0, 40.0, 0.1, 0.2, 0.5);
        double d2 = engine::black_scholes_d2(42.0, 40.0, 0.1, 0.2, 0.5);
        REQUIRE_THAT(d2, WithinAbs(d1 - 0.2 * std::sqrt(0.5), 1e-15));
    }
}

TEST_CASE("Black-Scholes call pricing", "[black_scholes]") {
    engine::PayOffCall call_payoff(40.0);
    engine::Option option(40.0, 0.5, call_payoff);
    engine::MarketData market{42.0, 0.10, 0.20};

    SECTION("Hull textbook call price") {
        auto result = engine::black_scholes_call(option, market);
        REQUIRE_THAT(result.price, WithinRel(4.759422392871535, 1e-10));
    }
    SECTION("analytical sentinel values") {
        auto result = engine::black_scholes_call(option, market);
        REQUIRE(result.standard_error == 0.0);
        REQUIRE(result.path_count == 0);
        REQUIRE_FALSE(result.greeks.has_value());
    }
}

TEST_CASE("Black-Scholes put pricing", "[black_scholes]") {
    engine::PayOffPut put_payoff(40.0);
    engine::Option option(40.0, 0.5, put_payoff);
    engine::MarketData market{42.0, 0.10, 0.20};

    SECTION("Hull textbook put price") {
        auto result = engine::black_scholes_put(option, market);
        REQUIRE_THAT(result.price, WithinRel(0.808599372900096, 1e-10));
    }
    SECTION("analytical sentinel values") {
        auto result = engine::black_scholes_put(option, market);
        REQUIRE(result.standard_error == 0.0);
        REQUIRE(result.path_count == 0);
        REQUIRE_FALSE(result.greeks.has_value());
    }
}

TEST_CASE("Put-call parity", "[black_scholes]") {
    SECTION("Hull example") {
        double K = 40.0;
        double T = 0.5;
        engine::PayOffCall call_payoff(K);
        engine::PayOffPut put_payoff(K);
        engine::Option call_option(K, T, call_payoff);
        engine::Option put_option(K, T, put_payoff);
        engine::MarketData market{42.0, 0.10, 0.20};

        auto call_result = engine::black_scholes_call(call_option, market);
        auto put_result = engine::black_scholes_put(put_option, market);
        double lhs = call_result.price - put_result.price;
        double rhs = market.spot - K * std::exp(-market.rate * T);
        REQUIRE_THAT(lhs - rhs, WithinAbs(0.0, 1e-10));
    }
    SECTION("ATM") {
        double K = 100.0;
        double T = 1.0;
        engine::PayOffCall call_payoff(K);
        engine::PayOffPut put_payoff(K);
        engine::Option call_option(K, T, call_payoff);
        engine::Option put_option(K, T, put_payoff);
        engine::MarketData market{100.0, 0.05, 0.25};

        auto call_result = engine::black_scholes_call(call_option, market);
        auto put_result = engine::black_scholes_put(put_option, market);
        double lhs = call_result.price - put_result.price;
        double rhs = market.spot - K * std::exp(-market.rate * T);
        REQUIRE_THAT(lhs - rhs, WithinAbs(0.0, 1e-10));
    }
    SECTION("Deep ITM call") {
        double K = 60.0;
        double T = 0.25;
        engine::PayOffCall call_payoff(K);
        engine::PayOffPut put_payoff(K);
        engine::Option call_option(K, T, call_payoff);
        engine::Option put_option(K, T, put_payoff);
        engine::MarketData market{100.0, 0.05, 0.30};

        auto call_result = engine::black_scholes_call(call_option, market);
        auto put_result = engine::black_scholes_put(put_option, market);
        double lhs = call_result.price - put_result.price;
        double rhs = market.spot - K * std::exp(-market.rate * T);
        REQUIRE_THAT(lhs - rhs, WithinAbs(0.0, 1e-10));
    }
    SECTION("Deep OTM call") {
        double K = 150.0;
        double T = 0.25;
        engine::PayOffCall call_payoff(K);
        engine::PayOffPut put_payoff(K);
        engine::Option call_option(K, T, call_payoff);
        engine::Option put_option(K, T, put_payoff);
        engine::MarketData market{100.0, 0.05, 0.30};

        auto call_result = engine::black_scholes_call(call_option, market);
        auto put_result = engine::black_scholes_put(put_option, market);
        double lhs = call_result.price - put_result.price;
        double rhs = market.spot - K * std::exp(-market.rate * T);
        REQUIRE_THAT(lhs - rhs, WithinAbs(0.0, 1e-10));
    }
}
