#include "catch_amalgamated.hpp"
#include "engine/random_gen.h"
#include <cstdint>

namespace {
    constexpr std::uint64_t SEED = 42;
}

TEST_CASE("AntitheticGen emits paired normals on consecutive calls",
          "[random_gen][antithetic]") {
    engine::MersenneTwisterGen base(SEED);
    engine::AntitheticGen anti(base);

    for (int i = 0; i < 10; ++i) {
        const double z1 = anti.next_normal();
        const double z2 = anti.next_normal();
        REQUIRE(z2 == -z1);
    }
}

TEST_CASE("AntitheticGen deterministic across reconstruction at same seed",
          "[random_gen][antithetic]") {
    engine::MersenneTwisterGen base_a(SEED);
    engine::AntitheticGen a(base_a);

    engine::MersenneTwisterGen base_b(SEED);
    engine::AntitheticGen b(base_b);

    for (int i = 0; i < 20; ++i) {
        REQUIRE(a.next_normal() == b.next_normal());
    }
}

TEST_CASE("AntitheticGen seed() forwards to inner and invalidates partner cache",
          "[random_gen][antithetic]") {
    engine::MersenneTwisterGen base(SEED);
    engine::AntitheticGen anti(base);

    (void)anti.next_normal();

    anti.seed(7);

    engine::MersenneTwisterGen fresh_base(7);
    engine::AntitheticGen fresh(fresh_base);
    for (int i = 0; i < 10; ++i) {
        REQUIRE(anti.next_normal() == fresh.next_normal());
    }
}

TEST_CASE("AntitheticGen polymorphic dispatch through RandomGen&",
          "[random_gen][antithetic]") {
    engine::MersenneTwisterGen base_a(SEED);
    engine::AntitheticGen concrete(base_a);
    engine::RandomGen& poly = concrete;

    engine::MersenneTwisterGen base_b(SEED);
    engine::AntitheticGen direct(base_b);

    for (int i = 0; i < 10; ++i) {
        REQUIRE(poly.next_normal() == direct.next_normal());
    }
}
