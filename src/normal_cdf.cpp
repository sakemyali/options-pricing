#include "engine/normal_cdf.h"
#include <cmath>

namespace engine {

static constexpr double INV_SQRT_2 = 0.7071067811865475244;
static constexpr double INV_SQRT_2PI = 0.3989422804014326779;

double normal_cdf(double x) {
    return 0.5 * std::erfc(-x * INV_SQRT_2);
}

double normal_pdf(double x) {
    return INV_SQRT_2PI * std::exp(-0.5 * x * x);
}

} // namespace engine
