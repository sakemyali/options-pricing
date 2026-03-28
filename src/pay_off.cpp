#include "engine/pay_off.h"
#include <algorithm>

namespace engine {

PayOffCall::PayOffCall(double strike) : strike_(strike) {}

double PayOffCall::compute(double spot) const {
    return std::max(spot - strike_, 0.0);
}

std::unique_ptr<PayOff> PayOffCall::clone() const {
    return std::make_unique<PayOffCall>(*this);
}

PayOffPut::PayOffPut(double strike) : strike_(strike) {}

double PayOffPut::compute(double spot) const {
    return std::max(strike_ - spot, 0.0);
}

std::unique_ptr<PayOff> PayOffPut::clone() const {
    return std::make_unique<PayOffPut>(*this);
}

} // namespace engine
