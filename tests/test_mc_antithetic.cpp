#include "catch_amalgamated.hpp"
#include "engine/monte_carlo_pricer.h"
#include "engine/random_gen.h"
#include "engine/black_scholes.h"
#include "engine/option.h"
#include "engine/pay_off.h"
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace {
    // Hull Ch 15 / Ch 21 reference parameter set -- same fixture used by
    // tests/test_mc_convergence.cpp and tests/test_monte_carlo_pricer.cpp.
    constexpr double        K_HULL     = 40.0;
    constexpr double        T_HULL     = 0.5;
    constexpr double        S_HULL     = 42.0;
    constexpr double        R_HULL     = 0.10;
    constexpr double        SIGMA_HULL = 0.20;
    constexpr std::uint64_t SEED       = 42;
}

TEST_CASE("MonteCarloPricer with AntitheticGen converges to BS call within 3-sigma at N=1M",
          "[mc_antithetic][.slow]") {
    // Central limit theorem: |MC - BS| / SE is approximately N(0,1) for an unbiased MC
    // estimator. Antithetic pairing preserves unbiasedness (E[(f(Z)+f(-Z))/2] = E[f(Z)] when
    // -Z is also N(0,1)), so the 3-sigma envelope applies just as it does for plain MC.
    //
    // Note: the SE reported by StatisticsGatherer here treats the paired-payoff stream as IID,
    // which over-estimates the true paired-estimator SE when pairs are negatively correlated.
    // The 3-sigma gate therefore remains conservative -- if anything we are even less likely
    // to flake than with plain MC at the same N. Determinism is preserved (seed=42,
    // mt19937_64 + Marsaglia polar + bit-exact IEEE-754 negation).
    engine::MersenneTwisterGen base_rng(SEED);
    engine::AntitheticGen      rng(base_rng);
    engine::MonteCarloPricer   pricer(rng, 1'000'000);

    engine::PayOffCall call_payoff(K_HULL);
    engine::Option     option(K_HULL, T_HULL, call_payoff);
    engine::MarketData market{S_HULL, R_HULL, SIGMA_HULL};

    auto mc = pricer.price(option, market);
    auto bs = engine::black_scholes_call(option, market);

    REQUIRE(std::abs(mc.price - bs.price) <= 3.0 * mc.standard_error);
    REQUIRE(mc.path_count == 1'000'000);
    REQUIRE(mc.standard_error > 0.0);
}

TEST_CASE("MonteCarloPricer with AntitheticGen reduces estimator variance vs plain MC (ensemble at N=10K, M=40)",
          "[mc_antithetic][.slow]") {
    // Ensemble-based variance-reduction gate (Option A -- replaces the original single-run
    // SE-ratio formulation that the plan proposed).
    //
    // Why not a single-run SE comparison?  StatisticsGatherer computes the sample SE of an
    // IID stream via Welford's online algorithm.  When an antithetic-paired stream
    // (z, -z, z', -z', ...) feeds the pricer, the resulting *payoff* stream has negative
    // pair correlation, so the reported sample SE (which assumes independence) is a biased
    // estimator of the true estimator SE -- it reports roughly the plain-MC SE at the same N,
    // not the SE of the paired mean.  Measured at seed=42 during Task 4 planning:
    // plain SE approximately 0.01577, antithetic single-run SE approximately 0.01569 (ratio approximately 0.995),
    // yet the true estimator variance reduction at these parameters is ~30-70%.  A single-run
    // gate of the form (anti.SE / plain.SE < 0.9) would therefore fail even though the
    // variance reduction is real.
    //
    // The correct approach: run the pricer M times with different seeds and measure the
    // sample variance of the M resulting price estimates.  That directly measures
    // Var[estimator], which is what antithetic pairing actually reduces.
    //
    // Ensemble parameters: M_RUNS = 40 independent seeds at N_PER_RUN = 10'000 paths each.
    // Total work 40 * 10'000 * 2 = 800K pricer paths -- well under the 1M of the 3-sigma
    // test above and still comfortably inside the [.slow] budget.  Even N avoids any
    // odd-N trailing-pair edge: every run consumes exactly N_PER_RUN / 2 inner draws.
    //
    // Expected empirical ratio for ATM-ish Hull Set 1 European call: anti_var / plain_var
    // in [0.30, 0.70] (Glasserman Ch 4, Joshi Ch 4).  The 0.81 threshold equals 0.9^2
    // translated to variance space -- equivalent to the plan's original SE ratio < 0.9
    // success criterion, now expressed against the correct statistic.
    constexpr std::size_t   N_PER_RUN = 10'000;
    constexpr int           M_RUNS    = 40;
    constexpr std::uint64_t BASE_SEED = SEED;

    engine::PayOffCall call_payoff(K_HULL);
    engine::Option     option(K_HULL, T_HULL, call_payoff);
    engine::MarketData market{S_HULL, R_HULL, SIGMA_HULL};

    // Plain MC ensemble: M independent seeds, one MersenneTwisterGen per run.
    double plain_sum    = 0.0;
    double plain_sum_sq = 0.0;
    for (int k = 0; k < M_RUNS; ++k) {
        engine::MersenneTwisterGen rng(BASE_SEED + static_cast<std::uint64_t>(k));
        engine::MonteCarloPricer   pricer(rng, N_PER_RUN);
        const double p = pricer.price(option, market).price;
        plain_sum    += p;
        plain_sum_sq += p * p;
    }
    const double plain_mean = plain_sum / static_cast<double>(M_RUNS);
    const double plain_var  = (plain_sum_sq / static_cast<double>(M_RUNS))
                            - plain_mean * plain_mean;

    // Antithetic MC ensemble: SAME seeds, but each base generator is wrapped by an
    // AntitheticGen decorator.  Lifetime management: both the base generator and the
    // decorator live on the stack within the per-iteration scope; they die together at
    // the end of each loop body so the reference inside AntitheticGen is always valid.
    double anti_sum    = 0.0;
    double anti_sum_sq = 0.0;
    for (int k = 0; k < M_RUNS; ++k) {
        engine::MersenneTwisterGen base(BASE_SEED + static_cast<std::uint64_t>(k));
        engine::AntitheticGen      anti(base);
        engine::MonteCarloPricer   pricer(anti, N_PER_RUN);
        const double p = pricer.price(option, market).price;
        anti_sum    += p;
        anti_sum_sq += p * p;
    }
    const double anti_mean = anti_sum / static_cast<double>(M_RUNS);
    const double anti_var  = (anti_sum_sq / static_cast<double>(M_RUNS))
                           - anti_mean * anti_mean;

    REQUIRE(plain_var > 0.0);
    REQUIRE(anti_var  > 0.0);

    // Variance-ratio gate: antithetic variance of the estimator must be < 0.81 * plain
    // variance (equivalent to SE ratio < 0.9 after sqrt).  A borderline or failing ratio
    // should be investigated before relaxing -- it would indicate pairing is not actually
    // reducing estimator variance at these parameters.
    const double var_ratio = anti_var / plain_var;
    REQUIRE(var_ratio < 0.81);

    // Both ensembles estimate the same BS price; the means must agree within a few plain
    // estimator SE.  Approximate plain-estimator SE = sqrt(plain_var / M_RUNS).
    const double plain_se_est = std::sqrt(plain_var / static_cast<double>(M_RUNS));
    REQUIRE(std::abs(anti_mean - plain_mean) < 3.0 * plain_se_est);
}
