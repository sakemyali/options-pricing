#include "engine/analytical_greeks.h"
#include "engine/black_scholes.h"
#include "engine/normal_cdf.h"
#include <cmath>

namespace engine {

double analytical_delta_call(const Option& option, const MarketData& market) {
    double d1 = black_scholes_d1(market.spot, option.strike(),
                                  market.rate, market.volatility, option.expiry());
    return normal_cdf(d1);
}

double analytical_delta_put(const Option& option, const MarketData& market) {
    double d1 = black_scholes_d1(market.spot, option.strike(),
                                  market.rate, market.volatility, option.expiry());
    return normal_cdf(d1) - 1.0;
}

double analytical_gamma_call(const Option& option, const MarketData& market) {
    double S = market.spot;
    double sigma = market.volatility;
    double T = option.expiry();
    double d1 = black_scholes_d1(S, option.strike(), market.rate, sigma, T);
    return normal_pdf(d1) / (S * sigma * std::sqrt(T));
}

double analytical_gamma_put(const Option& option, const MarketData& market) {
    double S = market.spot;
    double sigma = market.volatility;
    double T = option.expiry();
    double d1 = black_scholes_d1(S, option.strike(), market.rate, sigma, T);
    return normal_pdf(d1) / (S * sigma * std::sqrt(T));
}

double analytical_vega_call(const Option& option, const MarketData& market) {
    double S = market.spot;
    double T = option.expiry();
    double d1 = black_scholes_d1(S, option.strike(), market.rate, market.volatility, T);
    return S * normal_pdf(d1) * std::sqrt(T);
}

double analytical_vega_put(const Option& option, const MarketData& market) {
    double S = market.spot;
    double T = option.expiry();
    double d1 = black_scholes_d1(S, option.strike(), market.rate, market.volatility, T);
    return S * normal_pdf(d1) * std::sqrt(T);
}

double analytical_theta_call(const Option& option, const MarketData& market) {
    double S = market.spot;
    double K = option.strike();
    double r = market.rate;
    double sigma = market.volatility;
    double T = option.expiry();
    double d1 = black_scholes_d1(S, K, r, sigma, T);
    double d2 = d1 - sigma * std::sqrt(T);
    return -(S * normal_pdf(d1) * sigma) / (2.0 * std::sqrt(T))
           - r * K * std::exp(-r * T) * normal_cdf(d2);
}

double analytical_theta_put(const Option& option, const MarketData& market) {
    double S = market.spot;
    double K = option.strike();
    double r = market.rate;
    double sigma = market.volatility;
    double T = option.expiry();
    double d1 = black_scholes_d1(S, K, r, sigma, T);
    double d2 = d1 - sigma * std::sqrt(T);
    return -(S * normal_pdf(d1) * sigma) / (2.0 * std::sqrt(T))
           + r * K * std::exp(-r * T) * normal_cdf(-d2);
}

double analytical_rho_call(const Option& option, const MarketData& market) {
    double K = option.strike();
    double r = market.rate;
    double sigma = market.volatility;
    double T = option.expiry();
    double d1 = black_scholes_d1(market.spot, K, r, sigma, T);
    double d2 = d1 - sigma * std::sqrt(T);
    return K * T * std::exp(-r * T) * normal_cdf(d2);
}

double analytical_rho_put(const Option& option, const MarketData& market) {
    double K = option.strike();
    double r = market.rate;
    double sigma = market.volatility;
    double T = option.expiry();
    double d1 = black_scholes_d1(market.spot, K, r, sigma, T);
    double d2 = d1 - sigma * std::sqrt(T);
    return -K * T * std::exp(-r * T) * normal_cdf(-d2);
}

} // namespace engine
