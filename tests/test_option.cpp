#include "catch_amalgamated.hpp"
#include "engine/option.h"
#include "engine/pay_off.h"

using Catch::Approx;

TEST_CASE("Option stores contract parameters", "[option]") {
    SECTION("strike accessor") {
        engine::Option opt(100.0, 0.5, engine::PayOffCall(100.0));
        REQUIRE(opt.strike() == Approx(100.0));
    }
    SECTION("expiry accessor") {
        engine::Option opt(100.0, 0.5, engine::PayOffCall(100.0));
        REQUIRE(opt.expiry() == Approx(0.5));
    }
}

TEST_CASE("Option owns independent PayOff copy", "[option]") {
    std::unique_ptr<engine::Option> opt_ptr;

    {
        engine::PayOffCall call(100.0);
        auto opt = std::make_unique<engine::Option>(100.0, 0.5, call);
        opt_ptr = std::move(opt);
    }

    REQUIRE(opt_ptr->pay_off().compute(110.0) == Approx(10.0));
}

TEST_CASE("Option works with put payoff", "[option]") {
    engine::Option opt(100.0, 0.5, engine::PayOffPut(100.0));
    REQUIRE(opt.pay_off().compute(90.0) == Approx(10.0));
}

TEST_CASE("MarketData stores market conditions", "[option]") {
    engine::MarketData md{42.0, 0.1, 0.2};
    REQUIRE(md.spot == Approx(42.0));
    REQUIRE(md.rate == Approx(0.1));
    REQUIRE(md.volatility == Approx(0.2));
}
