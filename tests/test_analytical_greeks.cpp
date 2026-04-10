#include "catch_amalgamated.hpp"
#include "engine/analytical_greeks.h"
#include "engine/black_scholes.h"
#include "engine/option.h"
#include "engine/pay_off.h"

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

// Hull Parameter Set 1: S=42, K=40, r=0.10, sigma=0.20, T=0.5

TEST_CASE("Analytical Delta", "[greeks]") {
    engine::PayOffCall call_payoff(40.0);
    engine::PayOffPut put_payoff(40.0);
    engine::Option call_option(40.0, 0.5, call_payoff);
    engine::Option put_option(40.0, 0.5, put_payoff);
    engine::MarketData market{42.0, 0.10, 0.20};

    SECTION("call value matches Hull") {
        REQUIRE_THAT(engine::analytical_delta_call(call_option, market),
                     WithinRel(0.7791312909426690, 1e-10));
    }
    SECTION("put value matches Hull") {
        REQUIRE_THAT(engine::analytical_delta_put(put_option, market),
                     WithinRel(-0.2208687090573310, 1e-10));
    }
    SECTION("call delta in [0, 1]") {
        double d = engine::analytical_delta_call(call_option, market);
        REQUIRE(d >= 0.0);
        REQUIRE(d <= 1.0);
    }
    SECTION("put delta in [-1, 0]") {
        double d = engine::analytical_delta_put(put_option, market);
        REQUIRE(d >= -1.0);
        REQUIRE(d <= 0.0);
    }
    SECTION("put delta == call delta - 1") {
        double dc = engine::analytical_delta_call(call_option, market);
        double dp = engine::analytical_delta_put(put_option, market);
        REQUIRE_THAT(dp - (dc - 1.0), WithinAbs(0.0, 1e-14));
    }
}

TEST_CASE("Analytical Gamma", "[greeks]") {
    engine::PayOffCall call_payoff(40.0);
    engine::PayOffPut put_payoff(40.0);
    engine::Option call_option(40.0, 0.5, call_payoff);
    engine::Option put_option(40.0, 0.5, put_payoff);
    engine::MarketData market{42.0, 0.10, 0.20};

    SECTION("call value matches Hull") {
        REQUIRE_THAT(engine::analytical_gamma_call(call_option, market),
                     WithinRel(0.0499626704059119, 1e-10));
    }
    SECTION("put value matches Hull") {
        REQUIRE_THAT(engine::analytical_gamma_put(put_option, market),
                     WithinRel(0.0499626704059119, 1e-10));
    }
    SECTION("call == put bitwise") {
        REQUIRE(engine::analytical_gamma_call(call_option, market) ==
                engine::analytical_gamma_put(put_option, market));
    }
}

TEST_CASE("Analytical Vega", "[greeks]") {
    engine::PayOffCall call_payoff(40.0);
    engine::PayOffPut put_payoff(40.0);
    engine::Option call_option(40.0, 0.5, call_payoff);
    engine::Option put_option(40.0, 0.5, put_payoff);
    engine::MarketData market{42.0, 0.10, 0.20};

    SECTION("call value matches Hull") {
        REQUIRE_THAT(engine::analytical_vega_call(call_option, market),
                     WithinRel(8.8134150596028533, 1e-10));
    }
    SECTION("put value matches Hull") {
        REQUIRE_THAT(engine::analytical_vega_put(put_option, market),
                     WithinRel(8.8134150596028533, 1e-10));
    }
    SECTION("call == put bitwise") {
        REQUIRE(engine::analytical_vega_call(call_option, market) ==
                engine::analytical_vega_put(put_option, market));
    }
}

TEST_CASE("Analytical Theta", "[greeks]") {
    engine::PayOffCall call_payoff(40.0);
    engine::PayOffPut put_payoff(40.0);
    engine::Option call_option(40.0, 0.5, call_payoff);
    engine::Option put_option(40.0, 0.5, put_payoff);
    engine::MarketData market{42.0, 0.10, 0.20};

    SECTION("call value matches Hull") {
        REQUIRE_THAT(engine::analytical_theta_call(call_option, market),
                     WithinRel(-4.5590921945926262, 1e-10));
    }
    SECTION("put value matches Hull") {
        REQUIRE_THAT(engine::analytical_theta_put(put_option, market),
                     WithinRel(-0.7541744965897703, 1e-10));
    }
    SECTION("deep ITM put has positive theta") {
        engine::PayOffPut deep_put(100.0);
        engine::Option deep_option(100.0, 1.0, deep_put);
        engine::MarketData deep_market{30.0, 0.10, 0.20};
        double theta = engine::analytical_theta_put(deep_option, deep_market);
        REQUIRE(theta > 0.0);
    }
}

TEST_CASE("Analytical Rho", "[greeks]") {
    engine::PayOffCall call_payoff(40.0);
    engine::PayOffPut put_payoff(40.0);
    engine::Option call_option(40.0, 0.5, call_payoff);
    engine::Option put_option(40.0, 0.5, put_payoff);
    engine::MarketData market{42.0, 0.10, 0.20};

    SECTION("call value matches Hull") {
        REQUIRE_THAT(engine::analytical_rho_call(call_option, market),
                     WithinRel(13.9820459133602810, 1e-10));
    }
    SECTION("put value matches Hull") {
        REQUIRE_THAT(engine::analytical_rho_put(put_option, market),
                     WithinRel(-5.0425425766540002, 1e-10));
    }
    SECTION("call rho positive") {
        REQUIRE(engine::analytical_rho_call(call_option, market) > 0.0);
    }
    SECTION("put rho negative") {
        REQUIRE(engine::analytical_rho_put(put_option, market) < 0.0);
    }
}

TEST_CASE("Convenience wrapper call", "[greeks][wrapper]") {
    engine::PayOffCall call_payoff(40.0);
    engine::Option option(40.0, 0.5, call_payoff);
    engine::MarketData market{42.0, 0.10, 0.20};

    SECTION("returns PricingResult with greeks populated") {
        auto result = engine::black_scholes_call_with_greeks(option, market);
        REQUIRE(result.greeks.has_value());
    }
    SECTION("price matches standalone BS call") {
        auto result = engine::black_scholes_call_with_greeks(option, market);
        auto standalone = engine::black_scholes_call(option, market);
        REQUIRE(result.price == standalone.price);
    }
    SECTION("greeks match standalone per-Greek functions") {
        auto result = engine::black_scholes_call_with_greeks(option, market);
        REQUIRE(result.greeks->delta == engine::analytical_delta_call(option, market));
        REQUIRE(result.greeks->gamma == engine::analytical_gamma_call(option, market));
        REQUIRE(result.greeks->vega == engine::analytical_vega_call(option, market));
        REQUIRE(result.greeks->theta == engine::analytical_theta_call(option, market));
        REQUIRE(result.greeks->rho == engine::analytical_rho_call(option, market));
    }
    SECTION("analytical sentinels") {
        auto result = engine::black_scholes_call_with_greeks(option, market);
        REQUIRE(result.standard_error == 0.0);
        REQUIRE(result.path_count == 0);
    }
}

TEST_CASE("Convenience wrapper put", "[greeks][wrapper]") {
    engine::PayOffPut put_payoff(40.0);
    engine::Option option(40.0, 0.5, put_payoff);
    engine::MarketData market{42.0, 0.10, 0.20};

    SECTION("returns PricingResult with greeks populated") {
        auto result = engine::black_scholes_put_with_greeks(option, market);
        REQUIRE(result.greeks.has_value());
    }
    SECTION("price matches standalone BS put") {
        auto result = engine::black_scholes_put_with_greeks(option, market);
        auto standalone = engine::black_scholes_put(option, market);
        REQUIRE(result.price == standalone.price);
    }
    SECTION("greeks match standalone per-Greek functions") {
        auto result = engine::black_scholes_put_with_greeks(option, market);
        REQUIRE(result.greeks->delta == engine::analytical_delta_put(option, market));
        REQUIRE(result.greeks->gamma == engine::analytical_gamma_put(option, market));
        REQUIRE(result.greeks->vega == engine::analytical_vega_put(option, market));
        REQUIRE(result.greeks->theta == engine::analytical_theta_put(option, market));
        REQUIRE(result.greeks->rho == engine::analytical_rho_put(option, market));
    }
    SECTION("analytical sentinels") {
        auto result = engine::black_scholes_put_with_greeks(option, market);
        REQUIRE(result.standard_error == 0.0);
        REQUIRE(result.path_count == 0);
    }
}

// Hull Parameter Set 2: S=49, K=50, r=0.05, sigma=0.20, T=20/52

TEST_CASE("Greeks Hull Chapter 19 cross-validation", "[greeks][hull19]") {
    double T = 20.0 / 52.0;
    engine::PayOffCall call_payoff(50.0);
    engine::PayOffPut put_payoff(50.0);
    engine::Option call_opt(50.0, T, call_payoff);
    engine::Option put_opt(50.0, T, put_payoff);
    engine::MarketData market{49.0, 0.05, 0.20};

    SECTION("call delta") {
        REQUIRE_THAT(engine::analytical_delta_call(call_opt, market),
                     WithinRel(0.5216046610663964, 1e-10));
    }
    SECTION("put delta") {
        REQUIRE_THAT(engine::analytical_delta_put(put_opt, market),
                     WithinRel(-0.4783953389336036, 1e-10));
    }
    SECTION("gamma (same for call/put)") {
        REQUIRE_THAT(engine::analytical_gamma_call(call_opt, market),
                     WithinRel(0.0655440393478444, 1e-10));
    }
    SECTION("vega (same for call/put)") {
        REQUIRE_THAT(engine::analytical_vega_call(call_opt, market),
                     WithinRel(12.1054798826288010, 1e-10));
    }
    SECTION("call theta") {
        REQUIRE_THAT(engine::analytical_theta_call(call_opt, market),
                     WithinRel(-4.3053298229325732, 1e-10));
    }
    SECTION("put theta") {
        REQUIRE_THAT(engine::analytical_theta_put(put_opt, market),
                     WithinRel(-1.8529474170320668, 1e-10));
    }
    SECTION("call rho") {
        REQUIRE_THAT(engine::analytical_rho_call(call_opt, market),
                     WithinRel(8.9069619496083483, 1e-10));
    }
    SECTION("put rho") {
        REQUIRE_THAT(engine::analytical_rho_put(put_opt, market),
                     WithinRel(-9.9575180957801646, 1e-10));
    }
    SECTION("call wrapper price") {
        auto result = engine::black_scholes_call_with_greeks(call_opt, market);
        REQUIRE_THAT(result.price, WithinRel(2.4005273232717137, 1e-10));
    }
    SECTION("put wrapper price") {
        auto result = engine::black_scholes_put_with_greeks(put_opt, market);
        REQUIRE_THAT(result.price, WithinRel(2.4481754412818497, 1e-10));
    }
}
