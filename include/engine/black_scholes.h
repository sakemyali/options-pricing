#ifndef ENGINE_BLACK_SCHOLES_H
#define ENGINE_BLACK_SCHOLES_H

#include "engine/option.h"
#include "engine/pricing_result.h"

namespace engine {

// Raw-double helpers. Precondition: sigma > 0 && T > 0.
double black_scholes_d1(double S, double K, double r, double sigma, double T);
double black_scholes_d2(double S, double K, double r, double sigma, double T);

// Typed wrappers. Return PricingResult with standard_error=0.0, path_count=0, greeks=nullopt.
PricingResult black_scholes_call(const Option& option, const MarketData& market);
PricingResult black_scholes_put(const Option& option, const MarketData& market);

} // namespace engine

#endif // ENGINE_BLACK_SCHOLES_H
