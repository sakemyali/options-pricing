#include "catch_amalgamated.hpp"
#include "engine/random_gen.h"
#include <cmath>
#include <vector>

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

TEST_CASE("MersenneTwisterGen deterministic sequence across constructions", "[random_gen]") {
    engine::MersenneTwisterGen a(42);
    engine::MersenneTwisterGen b(42);
    for (int i = 0; i < 10; ++i) {
        REQUIRE(a.next_normal() == b.next_normal());
    }
}

TEST_CASE("MersenneTwisterGen first draw regression", "[random_gen]") {
    engine::MersenneTwisterGen rng(42);
    double first = rng.next_normal();
    // Regression anchor: first Marsaglia-polar draw for seed=42 on std::mt19937_64.
    // Portable bit-shift uniform idiom + hand-rolled Marsaglia polar make this value
    // bitwise-reproducible across libstdc++, libc++, and MSVC.
    REQUIRE_THAT(first, WithinRel(1.2938204232729367, 1e-12));
}

TEST_CASE("MersenneTwisterGen seed() resets sequence", "[random_gen]") {
    engine::MersenneTwisterGen fresh(42);
    std::vector<double> reference;
    for (int i = 0; i < 10; ++i) reference.push_back(fresh.next_normal());

    engine::MersenneTwisterGen reseeded(99);
    for (int i = 0; i < 5; ++i) (void)reseeded.next_normal();
    reseeded.seed(42);
    for (int i = 0; i < 10; ++i) {
        REQUIRE(reseeded.next_normal() == reference[i]);
    }
}

TEST_CASE("MersenneTwisterGen seed() invalidates cached Marsaglia pair", "[random_gen]") {
    engine::MersenneTwisterGen rng(42);
    (void)rng.next_normal();  // leaves a cached pair value (when Task 2 implements Marsaglia)
    rng.seed(7);
    engine::MersenneTwisterGen fresh(7);
    REQUIRE(rng.next_normal() == fresh.next_normal());
}

TEST_CASE("MersenneTwisterGen distributional sanity -- mean", "[random_gen]") {
    engine::MersenneTwisterGen rng(12345);
    constexpr int N = 100000;
    double sum = 0.0;
    for (int i = 0; i < N; ++i) sum += rng.next_normal();
    double mean = sum / static_cast<double>(N);
    REQUIRE(std::abs(mean) < 0.02);
}

TEST_CASE("MersenneTwisterGen distributional sanity -- stddev", "[random_gen]") {
    engine::MersenneTwisterGen rng(54321);
    constexpr int N = 100000;
    double sum = 0.0;
    double sum_sq = 0.0;
    for (int i = 0; i < N; ++i) {
        double x = rng.next_normal();
        sum += x;
        sum_sq += x * x;
    }
    double mean = sum / static_cast<double>(N);
    double variance = (sum_sq / static_cast<double>(N)) - mean * mean;
    double stddev = std::sqrt(variance);
    REQUIRE(std::abs(stddev - 1.0) < 0.02);
}

TEST_CASE("RandomGen polymorphic dispatch via base reference", "[random_gen]") {
    engine::MersenneTwisterGen concrete(42);
    engine::RandomGen& base = concrete;

    engine::MersenneTwisterGen direct(42);
    for (int i = 0; i < 10; ++i) {
        REQUIRE(base.next_normal() == direct.next_normal());
    }
}
