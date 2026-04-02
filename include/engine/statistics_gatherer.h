#ifndef ENGINE_STATISTICS_GATHERER_H
#define ENGINE_STATISTICS_GATHERER_H

namespace engine {

class StatisticsGatherer {
public:
    StatisticsGatherer();

    void add(double value);
    double mean() const;
    double standard_error() const;
    int count() const;

private:
    int count_;
    double mean_;
    double m2_;
};

} // namespace engine
#endif // ENGINE_STATISTICS_GATHERER_H
