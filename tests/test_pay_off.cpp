#include "catch_amalgamated.hpp"
#include "engine/pay_off.h"

using Catch::Approx;

TEST_CASE("PayOffCall computes terminal payoff", "[pay_off]") {
    engine::PayOffCall call(100.0);

    SECTION("ITM: spot > strike") {
        REQUIRE(call.compute(110.0) == Approx(10.0));
    }
    SECTION("ATM: spot == strike") {
        REQUIRE(call.compute(100.0) == Approx(0.0));
    }
    SECTION("OTM: spot < strike") {
        REQUIRE(call.compute(90.0) == Approx(0.0));
    }
}

TEST_CASE("PayOffPut computes terminal payoff", "[pay_off]") {
    engine::PayOffPut put(100.0);

    SECTION("ITM: spot < strike") {
        REQUIRE(put.compute(90.0) == Approx(10.0));
    }
    SECTION("ATM: spot == strike") {
        REQUIRE(put.compute(100.0) == Approx(0.0));
    }
    SECTION("OTM: spot > strike") {
        REQUIRE(put.compute(110.0) == Approx(0.0));
    }
}

TEST_CASE("PayOff clone produces independent copy", "[pay_off]") {
    SECTION("Call clone computes same values") {
        engine::PayOffCall original(100.0);
        auto cloned = original.clone();
        REQUIRE(cloned->compute(110.0) == Approx(10.0));
    }
    SECTION("Call clone is different address") {
        engine::PayOffCall original(100.0);
        auto cloned = original.clone();
        REQUIRE(cloned.get() != &original);
    }
    SECTION("Put clone computes same values") {
        engine::PayOffPut original(100.0);
        auto cloned = original.clone();
        REQUIRE(cloned->compute(90.0) == Approx(10.0));
    }
    SECTION("Put clone is different address") {
        engine::PayOffPut original(100.0);
        auto cloned = original.clone();
        REQUIRE(cloned.get() != &original);
    }
}
