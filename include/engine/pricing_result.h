#ifndef ENGINE_PRICING_RESULT_H
#define ENGINE_PRICING_RESULT_H

#include <optional>

namespace engine {

struct Greeks {
    double delta = 0.0;
    double gamma = 0.0;
    double vega  = 0.0;
    double theta = 0.0;
    double rho   = 0.0;
};

struct PricingResult {
    double price = 0.0;
    double standard_error = 0.0;   // 0.0 sentinel for analytical
    int path_count = 0;            // 0 sentinel for analytical
    std::optional<Greeks> greeks = std::nullopt;
};

} // namespace engine
#endif // ENGINE_PRICING_RESULT_H
