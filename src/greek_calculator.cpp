#include "engine/greek_calculator.h"

namespace engine {

GreekCalculator::GreekCalculator(PricerFn pricer)
    : pricer_(std::move(pricer)), bumps_() {}

GreekCalculator::GreekCalculator(PricerFn pricer, BumpSizes bumps)
    : pricer_(std::move(pricer)), bumps_(bumps) {}

double GreekCalculator::delta(const Option& option, const MarketData& market) const {
    const double h = market.spot * bumps_.S;
    MarketData up = market;     up.spot += h;
    MarketData dn = market;     dn.spot -= h;
    return (pricer_(option, up) - pricer_(option, dn)) / (2.0 * h);
}

double GreekCalculator::gamma(const Option& option, const MarketData& market) const {
    const double h = market.spot * bumps_.S;
    MarketData up = market;     up.spot += h;
    MarketData dn = market;     dn.spot -= h;
    const double V_up  = pricer_(option, up);
    const double V_mid = pricer_(option, market);
    const double V_dn  = pricer_(option, dn);
    return (V_up - 2.0 * V_mid + V_dn) / (h * h);
}

double GreekCalculator::vega(const Option& option, const MarketData& market) const {
    const double h = bumps_.sigma;
    MarketData up = market;     up.volatility += h;
    MarketData dn = market;     dn.volatility -= h;
    return (pricer_(option, up) - pricer_(option, dn)) / (2.0 * h);
}

double GreekCalculator::theta(const Option& option, const MarketData& market) const {
    const double h = bumps_.T;
    Option up(option.strike(), option.expiry() + h, option.pay_off());
    Option dn(option.strike(), option.expiry() - h, option.pay_off());
    return (pricer_(up, market) - pricer_(dn, market)) / (2.0 * h);
}

double GreekCalculator::rho(const Option& option, const MarketData& market) const {
    const double h = bumps_.r;
    MarketData up = market;     up.rate += h;
    MarketData dn = market;     dn.rate -= h;
    return (pricer_(option, up) - pricer_(option, dn)) / (2.0 * h);
}

} // namespace engine
