#include "engine/monte_carlo_pricer.h"
#include "engine/pay_off.h"
#include "engine/statistics_gatherer.h"
#include <cmath>

namespace engine {

MonteCarloPricer::MonteCarloPricer(RandomGen& rng, std::size_t num_paths)
    : rng_(rng), num_paths_(num_paths) {}

PricingResult MonteCarloPricer::price(const Option& option,
                                      const MarketData& market) const {
    const double S0        = market.spot;
    const double r         = market.rate;
    const double sigma     = market.volatility;
    const double T         = option.expiry();

    // Hoist drift/diffusion/discount out of the loop. The -0.5*sigma^2 Ito correction is mandatory.
    const double drift     = (r - 0.5 * sigma * sigma) * T;
    const double diffusion = sigma * std::sqrt(T);
    const double discount  = std::exp(-r * T);

    StatisticsGatherer stats;
    for (std::size_t i = 0; i < num_paths_; ++i) {
        const double z   = rng_.next_normal();
        const double S_T = S0 * std::exp(drift + diffusion * z);
        stats.add(option.pay_off().compute(S_T));
    }

    PricingResult result;
    result.price          = discount * stats.mean();
    result.standard_error = discount * stats.standard_error();
    result.path_count     = static_cast<int>(num_paths_);
    result.greeks         = std::nullopt;
    return result;
}

} // namespace engine
