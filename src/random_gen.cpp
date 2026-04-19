#include "engine/random_gen.h"
#include <cmath>

namespace engine {

MersenneTwisterGen::MersenneTwisterGen(std::uint64_t s)
    : engine_(s), has_cached_(false), cached_(0.0) {}

void MersenneTwisterGen::seed(std::uint64_t s) {
    engine_.seed(s);
    has_cached_ = false;
    cached_ = 0.0;
}

double MersenneTwisterGen::next_normal() {
    if (has_cached_) {
        has_cached_ = false;
        return cached_;
    }
    double u, v, s;
    do {
        // Marsaglia polar: uniform [0,1) via portable bit-shift idiom, then scale to (-1, 1).
        const double r1 = static_cast<double>(engine_() >> 11)
                        * (1.0 / static_cast<double>(1ULL << 53));
        const double r2 = static_cast<double>(engine_() >> 11)
                        * (1.0 / static_cast<double>(1ULL << 53));
        u = 2.0 * r1 - 1.0;
        v = 2.0 * r2 - 1.0;
        s = u * u + v * v;
    } while (s >= 1.0 || s == 0.0);
    const double factor = std::sqrt(-2.0 * std::log(s) / s);
    cached_ = v * factor;
    has_cached_ = true;
    return u * factor;
}

AntitheticGen::AntitheticGen(RandomGen& inner)
    : inner_(inner), has_partner_(false), partner_(0.0) {}

double AntitheticGen::next_normal() {
    if (has_partner_) {
        has_partner_ = false;
        return -partner_;
    }
    partner_ = inner_.next_normal();
    has_partner_ = true;
    return partner_;
}

void AntitheticGen::seed(std::uint64_t s) {
    inner_.seed(s);
    has_partner_ = false;
    partner_ = 0.0;
}

} // namespace engine
