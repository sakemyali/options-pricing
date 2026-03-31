#include "catch_amalgamated.hpp"
#include "engine/pricing_result.h"

using Catch::Approx;

TEST_CASE("PricingResult default construction", "[pricing_result]") {
    engine::PricingResult result{};
    REQUIRE(result.price == Approx(0.0));
    REQUIRE(result.standard_error == Approx(0.0));
    REQUIRE(result.path_count == 0);
    REQUIRE(result.greeks.has_value() == false);
}

TEST_CASE("PricingResult analytical result", "[pricing_result]") {
    engine::PricingResult result{4.76, 0.0, 0, std::nullopt};
    REQUIRE(result.price == Approx(4.76));
    REQUIRE(result.standard_error == Approx(0.0));
    REQUIRE(result.path_count == 0);
    REQUIRE(result.greeks.has_value() == false);
}

TEST_CASE("PricingResult with Greeks", "[pricing_result]") {
    engine::Greeks greeks{0.6, 0.02, 0.15, -0.03, 0.1};
    engine::PricingResult result{4.76, 0.0, 0, greeks};
    REQUIRE(result.greeks.has_value() == true);
    REQUIRE(result.greeks->delta == Approx(0.6));
    REQUIRE(result.greeks->gamma == Approx(0.02));
    REQUIRE(result.greeks->vega == Approx(0.15));
    REQUIRE(result.greeks->theta == Approx(-0.03));
    REQUIRE(result.greeks->rho == Approx(0.1));
}

TEST_CASE("PricingResult Monte Carlo result", "[pricing_result]") {
    engine::PricingResult result{4.80, 0.05, 100000, std::nullopt};
    REQUIRE(result.price == Approx(4.80));
    REQUIRE(result.standard_error == Approx(0.05));
    REQUIRE(result.path_count == 100000);
}

TEST_CASE("Greeks default construction", "[pricing_result]") {
    engine::Greeks g{};
    REQUIRE(g.delta == Approx(0.0));
    REQUIRE(g.gamma == Approx(0.0));
    REQUIRE(g.vega == Approx(0.0));
    REQUIRE(g.theta == Approx(0.0));
    REQUIRE(g.rho == Approx(0.0));
}
