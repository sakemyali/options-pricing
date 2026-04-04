#include "engine/black_scholes.h"
#include "engine/normal_cdf.h"
#include <cmath>

namespace engine {

double black_scholes_d1(double S, double K, double r, double sigma, double T) {
    double sigma_sqrt_T = sigma * std::sqrt(T);
    return (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / sigma_sqrt_T;
}

double black_scholes_d2(double S, double K, double r, double sigma, double T) {
    return black_scholes_d1(S, K, r, sigma, T) - sigma * std::sqrt(T);
}

PricingResult black_scholes_call(const Option& option, const MarketData& market) {
    double S = market.spot;
    double K = option.strike();
    double r = market.rate;
    double sigma = market.volatility;
    double T = option.expiry();

    double d1_val = black_scholes_d1(S, K, r, sigma, T);
    double d2_val = d1_val - sigma * std::sqrt(T);

    double price = S * normal_cdf(d1_val) - K * std::exp(-r * T) * normal_cdf(d2_val);
    return PricingResult{price, 0.0, 0, std::nullopt};
}

PricingResult black_scholes_put(const Option& option, const MarketData& market) {
    double S = market.spot;
    double K = option.strike();
    double r = market.rate;
    double sigma = market.volatility;
    double T = option.expiry();

    double d1_val = black_scholes_d1(S, K, r, sigma, T);
    double d2_val = d1_val - sigma * std::sqrt(T);

    double price = K * std::exp(-r * T) * normal_cdf(-d2_val) - S * normal_cdf(-d1_val);
    return PricingResult{price, 0.0, 0, std::nullopt};
}

} // namespace engine
