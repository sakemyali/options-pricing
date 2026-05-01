#include "engine/implied_volatility.h"
#include "engine/black_scholes.h"
#include "engine/analytical_greeks.h"
#include "engine/pay_off.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace engine {

double implied_volatility(const Option& option,
                          const MarketData& market,
                          double market_price,
                          double sigma_init,
                          double tol_abs,
                          int    max_iter) {
    const bool is_call = (dynamic_cast<const PayOffCall*>(&option.pay_off()) != nullptr);

    const double S = market.spot;
    const double K = option.strike();
    const double r = market.rate;
    const double T = option.expiry();
    const double discount = std::exp(-r * T);

    const double intrinsic = is_call
        ? std::max(S - K * discount, 0.0)
        : std::max(K * discount - S, 0.0);
    const double upper = is_call ? S : K * discount;

    if (market_price < intrinsic - 1e-12 || market_price > upper + 1e-12) {
        throw std::runtime_error("market price violates no-arbitrage bound");
    }

    double sigma_lo = 1e-6;
    double sigma_hi = 5.0;
    double sigma    = std::clamp(sigma_init, sigma_lo, sigma_hi);

    auto price_at = [&](double s) {
        MarketData md = market;
        md.volatility = s;
        return is_call
            ? black_scholes_call(option, md).price
            : black_scholes_put(option, md).price;
    };

    auto vega_at = [&](double s) {
        MarketData md = market;
        md.volatility = s;
        return is_call
            ? analytical_vega_call(option, md)
            : analytical_vega_put(option, md);
    };

    double f_lo = price_at(sigma_lo) - market_price;
    double f_hi = price_at(sigma_hi) - market_price;
    if (f_lo * f_hi > 0.0) {
        throw std::runtime_error("implied vol: bracket does not straddle zero");
    }

    for (int iter = 0; iter < max_iter; ++iter) {
        const double f = price_at(sigma) - market_price;
        if (std::abs(f) < tol_abs) {
            return sigma;
        }
        const double v = vega_at(sigma);
        const double newton = sigma - f / (std::abs(v) > 1e-10 ? v : 1.0);
        double sigma_next;
        if (std::abs(v) > 1e-10 && newton > sigma_lo && newton < sigma_hi) {
            sigma_next = newton;
        } else {
            sigma_next = 0.5 * (sigma_lo + sigma_hi);
        }
        if (f * f_lo < 0.0) {
            sigma_hi = sigma;
            f_hi = f;
        } else {
            sigma_lo = sigma;
            f_lo = f;
        }
        sigma = sigma_next;
    }
    throw std::runtime_error("implied vol: iteration cap exceeded");
}

}
