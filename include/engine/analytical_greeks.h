#ifndef ENGINE_ANALYTICAL_GREEKS_H
#define ENGINE_ANALYTICAL_GREEKS_H

#include "engine/option.h"
#include "engine/pricing_result.h"

namespace engine {

double analytical_delta_call(const Option& option, const MarketData& market);
double analytical_delta_put(const Option& option, const MarketData& market);

double analytical_gamma_call(const Option& option, const MarketData& market);
double analytical_gamma_put(const Option& option, const MarketData& market);

double analytical_vega_call(const Option& option, const MarketData& market);
double analytical_vega_put(const Option& option, const MarketData& market);

double analytical_theta_call(const Option& option, const MarketData& market);
double analytical_theta_put(const Option& option, const MarketData& market);

double analytical_rho_call(const Option& option, const MarketData& market);
double analytical_rho_put(const Option& option, const MarketData& market);

} // namespace engine

#endif // ENGINE_ANALYTICAL_GREEKS_H
