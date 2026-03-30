#include "engine/option.h"

namespace engine {

Option::Option(double strike, double expiry, const PayOff& pay_off)
    : strike_(strike), expiry_(expiry), pay_off_(pay_off.clone()) {}

double Option::strike() const { return strike_; }
double Option::expiry() const { return expiry_; }
const PayOff& Option::pay_off() const { return *pay_off_; }

} // namespace engine
