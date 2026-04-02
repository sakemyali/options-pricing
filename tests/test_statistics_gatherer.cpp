#include "catch_amalgamated.hpp"
#include "engine/statistics_gatherer.h"

using Catch::Approx;
using Catch::Matchers::WithinRel;

TEST_CASE("StatisticsGatherer empty state", "[statistics]") {
    engine::StatisticsGatherer stats;
    REQUIRE(stats.count() == 0);
    REQUIRE(stats.mean() == Approx(0.0));
    REQUIRE(stats.standard_error() == Approx(0.0));
}

TEST_CASE("StatisticsGatherer single value", "[statistics]") {
    engine::StatisticsGatherer stats;
    stats.add(5.0);
    REQUIRE(stats.count() == 1);
    REQUIRE(stats.mean() == Approx(5.0));
    REQUIRE(stats.standard_error() == Approx(0.0));
}

TEST_CASE("StatisticsGatherer two values", "[statistics]") {
    engine::StatisticsGatherer stats;
    stats.add(3.0);
    stats.add(7.0);
    REQUIRE(stats.count() == 2);
    REQUIRE(stats.mean() == Approx(5.0));
    REQUIRE(stats.standard_error() == Approx(2.0).margin(1e-10));
}

TEST_CASE("StatisticsGatherer known sequence", "[statistics]") {
    engine::StatisticsGatherer stats;
    for (double v : {2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0}) {
        stats.add(v);
    }
    REQUIRE(stats.count() == 8);
    REQUIRE(stats.mean() == Approx(5.0));
    REQUIRE(stats.standard_error() == Approx(0.75593).margin(1e-4));
}

TEST_CASE("StatisticsGatherer constant values", "[statistics]") {
    engine::StatisticsGatherer stats;
    for (int i = 0; i < 1000; ++i) {
        stats.add(42.0);
    }
    REQUIRE(stats.count() == 1000);
    REQUIRE(stats.mean() == Approx(42.0));
    REQUIRE(stats.standard_error() == Approx(0.0).margin(1e-15));
}

TEST_CASE("StatisticsGatherer large count stability", "[statistics]") {
    engine::StatisticsGatherer stats;
    for (int i = 0; i < 100000; ++i) {
        stats.add(1000000.0 + i * 0.001);
    }
    REQUIRE(stats.count() == 100000);
    REQUIRE_THAT(stats.mean(), WithinRel(1000049.9995, 1e-6));
    REQUIRE(stats.standard_error() > 0.0);
    REQUIRE(std::isfinite(stats.standard_error()));
}
