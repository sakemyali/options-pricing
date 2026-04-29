#include "catch_amalgamated.hpp"
#include "engine/monte_carlo_pricer.h"
#include "engine/random_gen.h"
#include "engine/option.h"
#include "engine/pay_off.h"
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace {
    // Hull Ch 15 / Ch 21 reference parameter set.
    // Same fixture used in tests/test_mc_convergence.cpp and tests/test_mc_antithetic.cpp.
    constexpr double        K_HULL     = 40.0;
    constexpr double        T_HULL     = 0.5;
    constexpr double        S_HULL     = 42.0;
    constexpr double        R_HULL     = 0.10;
    constexpr double        SIGMA_HULL = 0.20;
    constexpr std::uint64_t SEED       = 42;
    constexpr std::size_t   N_BENCH    = 1'000'000;

    // Soft-gate floor: catastrophic-regression guard (forgotten -O2, accidental
    // per-path allocation, debug assert in inner loop, etc.). Reported throughput
    // is informational; we only fail on a 1M sims/sec floor since hardware variance
    // dominates and CI portability matters more than fragile machine-specific numbers.
    constexpr double        THROUGHPUT_FLOOR_SIMS_PER_SEC = 1'000'000.0;
}

TEST_CASE("MonteCarloPricer plain MC throughput at N=1M",
          "[mc_benchmark][.benchmark]") {
    engine::PayOffCall call_payoff(K_HULL);
    engine::Option     option(K_HULL, T_HULL, call_payoff);
    engine::MarketData market{S_HULL, R_HULL, SIGMA_HULL};

    // Catch2 BENCHMARK block: variance + mean + stddev tabular reporting.
    // Returning the price triggers Catch2::Benchmark::invoke_deoptimized
    // (catch_amalgamated.hpp:1471), defeating dead-code elimination.
    BENCHMARK("plain MC pricer.price() N=1M") {
        engine::MersenneTwisterGen rng(SEED);
        engine::MonteCarloPricer   pricer(rng, N_BENCH);
        return pricer.price(option, market).price;
    };

    // Separately-timed single call -> the explicit sims/sec number used in
    // INFO() and the soft REQUIRE. Constructors are BEFORE t0 so their cost
    // is excluded from the timed window.
    engine::MersenneTwisterGen rng(SEED);
    engine::MonteCarloPricer   pricer(rng, N_BENCH);
    const auto t0 = std::chrono::steady_clock::now();
    const auto result = pricer.price(option, market);
    const auto t1 = std::chrono::steady_clock::now();

    // Optimizer-defeat for the chrono-timed block (Catch2 auto-deopt does NOT
    // apply outside BENCHMARK). volatile forces the store, which forces the
    // entire pricer.price() dependency chain to be live.
    volatile double sink = result.price;
    (void)sink;

    const double elapsed_s =
        std::chrono::duration<double>(t1 - t0).count();
    const double sims_per_sec =
        static_cast<double>(N_BENCH) / elapsed_s;
    const double m_sims_per_sec = sims_per_sec / 1.0e6;

    INFO("plain MC: N=" << N_BENCH
         << ", elapsed=" << elapsed_s << " s"
         << ", throughput=" << m_sims_per_sec << " M sims/sec");

    REQUIRE(std::isfinite(sims_per_sec));
    REQUIRE(sims_per_sec > THROUGHPUT_FLOOR_SIMS_PER_SEC);
}

TEST_CASE("MonteCarloPricer with AntitheticGen throughput at N=1M",
          "[mc_benchmark][.benchmark]") {
    engine::PayOffCall call_payoff(K_HULL);
    engine::Option     option(K_HULL, T_HULL, call_payoff);
    engine::MarketData market{S_HULL, R_HULL, SIGMA_HULL};

    BENCHMARK("antithetic MC pricer.price() N=1M") {
        engine::MersenneTwisterGen base(SEED);
        engine::AntitheticGen      anti(base);
        engine::MonteCarloPricer   pricer(anti, N_BENCH);
        return pricer.price(option, market).price;
    };

    engine::MersenneTwisterGen base(SEED);
    engine::AntitheticGen      anti(base);
    engine::MonteCarloPricer   pricer(anti, N_BENCH);
    const auto t0 = std::chrono::steady_clock::now();
    const auto result = pricer.price(option, market);
    const auto t1 = std::chrono::steady_clock::now();

    volatile double sink = result.price;
    (void)sink;

    const double elapsed_s =
        std::chrono::duration<double>(t1 - t0).count();
    const double sims_per_sec =
        static_cast<double>(N_BENCH) / elapsed_s;
    const double m_sims_per_sec = sims_per_sec / 1.0e6;

    INFO("antithetic MC: N=" << N_BENCH
         << ", elapsed=" << elapsed_s << " s"
         << ", throughput=" << m_sims_per_sec << " M sims/sec");

    REQUIRE(std::isfinite(sims_per_sec));
    REQUIRE(sims_per_sec > THROUGHPUT_FLOOR_SIMS_PER_SEC);
}
