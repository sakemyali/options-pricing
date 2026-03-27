#ifndef ENGINE_NORMAL_CDF_H
#define ENGINE_NORMAL_CDF_H

namespace engine {

/// Cumulative distribution function of the standard normal distribution.
/// N(x) = 0.5 * erfc(-x / sqrt(2))
double normal_cdf(double x);

/// Probability density function of the standard normal distribution.
/// n(x) = (1/sqrt(2*pi)) * exp(-x^2/2)
double normal_pdf(double x);

} // namespace engine

#endif // ENGINE_NORMAL_CDF_H
