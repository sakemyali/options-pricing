#ifndef ENGINE_IMPLIED_VOLATILITY_H
#define ENGINE_IMPLIED_VOLATILITY_H

#include "engine/option.h"

namespace engine {

double implied_volatility(const Option& option,
                          const MarketData& market,
                          double market_price,
                          double sigma_init = 0.20,
                          double tol_abs    = 1e-8,
                          int    max_iter   = 50);

}

#endif
