#ifndef ENGINE_OPTION_H
#define ENGINE_OPTION_H

#include "engine/pay_off.h"
#include <memory>

namespace engine {

struct MarketData {
    double spot;
    double rate;
    double volatility;
};

class Option {
public:
    Option(double strike, double expiry, const PayOff& pay_off);

    double strike() const;
    double expiry() const;
    const PayOff& pay_off() const;

private:
    double strike_;
    double expiry_;
    std::unique_ptr<PayOff> pay_off_;
};

} // namespace engine
#endif // ENGINE_OPTION_H
