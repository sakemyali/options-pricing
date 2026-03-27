#include "catch_amalgamated.hpp"
#include "engine/normal_cdf.h"

using Catch::Approx;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

TEST_CASE("NormalCDF known values", "[normal_cdf]") {
    SECTION("N(0) = 0.5 exactly") {
        REQUIRE(engine::normal_cdf(0.0) == Approx(0.5).margin(1e-15));
    }
    SECTION("N(1) matches table") {
        REQUIRE_THAT(engine::normal_cdf(1.0), WithinRel(0.841344746068543, 1e-10));
    }
    SECTION("N(-1) matches table") {
        REQUIRE_THAT(engine::normal_cdf(-1.0), WithinRel(0.158655253931457, 1e-10));
    }
    SECTION("N(1.96) approx 0.975") {
        REQUIRE_THAT(engine::normal_cdf(1.96), WithinRel(0.975002104851780, 1e-10));
    }
    SECTION("N(-1.96) matches table") {
        REQUIRE_THAT(engine::normal_cdf(-1.96), WithinRel(0.024997895148220, 1e-10));
    }
    SECTION("N(3.0) deep in tail") {
        REQUIRE_THAT(engine::normal_cdf(3.0), WithinRel(0.998650101968370, 1e-10));
    }
}

TEST_CASE("NormalCDF edge cases", "[normal_cdf]") {
    SECTION("Large positive saturates to 1") {
        REQUIRE(engine::normal_cdf(10.0) == Approx(1.0).margin(1e-15));
    }
    SECTION("Large negative saturates to 0") {
        REQUIRE(engine::normal_cdf(-10.0) == Approx(0.0).margin(1e-15));
    }
}

TEST_CASE("NormalPDF known values", "[normal_pdf]") {
    SECTION("n(0) is peak of standard normal") {
        REQUIRE_THAT(engine::normal_pdf(0.0), WithinRel(0.3989422804014327, 1e-10));
    }
    SECTION("n(1) matches known value") {
        REQUIRE_THAT(engine::normal_pdf(1.0), WithinRel(0.2419707245191434, 1e-10));
    }
    SECTION("n(-1) == n(1) exact symmetry") {
        REQUIRE(engine::normal_pdf(-1.0) == engine::normal_pdf(1.0));
    }
    SECTION("large |x| approaches 0") {
        REQUIRE(engine::normal_pdf(5.0) < 1e-5);
        REQUIRE(engine::normal_pdf(-5.0) < 1e-5);
    }
}

TEST_CASE("NormalCDF symmetry", "[normal_cdf]") {
    SECTION("N(x) + N(-x) = 1") {
        for (double x : {0.5, 1.0, 1.5, 2.0, 3.0}) {
            REQUIRE_THAT(engine::normal_cdf(x) + engine::normal_cdf(-x),
                         WithinAbs(1.0, 1e-15));
        }
    }
}
