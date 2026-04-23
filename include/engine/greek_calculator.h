#ifndef ENGINE_GREEK_CALCULATOR_H
#define ENGINE_GREEK_CALCULATOR_H

#include "engine/option.h"
#include <functional>

namespace engine {

struct BumpSizes {
    double S     = 1e-4;
    double sigma = 1e-4;
    double r     = 1e-4;
    double T     = 1e-5;
};

class GreekCalculator {
public:
    using PricerFn = std::function<double(const Option&, const MarketData&)>;

    explicit GreekCalculator(PricerFn pricer);
    GreekCalculator(PricerFn pricer, BumpSizes bumps);

    double delta(const Option& option, const MarketData& market) const;

    double gamma(const Option& option, const MarketData& market) const;

    double vega(const Option& option, const MarketData& market) const;

    // Returns dV/dT (RAW central difference). For financial Theta = -dV/dT, negate at call site.
    double theta(const Option& option, const MarketData& market) const;

    double rho(const Option& option, const MarketData& market) const;

private:
    PricerFn  pricer_;
    BumpSizes bumps_;
};

} // namespace engine
#endif // ENGINE_GREEK_CALCULATOR_H
