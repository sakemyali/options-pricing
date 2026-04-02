#include "engine/statistics_gatherer.h"
#include <cmath>

namespace engine {

StatisticsGatherer::StatisticsGatherer()
    : count_(0), mean_(0.0), m2_(0.0) {}

void StatisticsGatherer::add(double value) {
    ++count_;
    double delta = value - mean_;
    mean_ += delta / count_;
    double delta2 = value - mean_;
    m2_ += delta * delta2;
}

double StatisticsGatherer::mean() const {
    return (count_ > 0) ? mean_ : 0.0;
}

double StatisticsGatherer::standard_error() const {
    if (count_ < 2) return 0.0;
    return std::sqrt(m2_ / (static_cast<double>(count_) * (count_ - 1)));
}

int StatisticsGatherer::count() const {
    return count_;
}

} // namespace engine
