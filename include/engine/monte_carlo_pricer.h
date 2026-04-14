#ifndef ENGINE_MONTE_CARLO_PRICER_H
#define ENGINE_MONTE_CARLO_PRICER_H

#include "engine/option.h"
#include "engine/pricing_result.h"
#include "engine/random_gen.h"
#include <cstddef>

namespace engine {

class MonteCarloPricer {
public:
    MonteCarloPricer(RandomGen& rng, std::size_t num_paths);
    PricingResult price(const Option& option, const MarketData& market) const;

private:
    RandomGen& rng_;
    std::size_t num_paths_;
};

} // namespace engine
#endif // ENGINE_MONTE_CARLO_PRICER_H
