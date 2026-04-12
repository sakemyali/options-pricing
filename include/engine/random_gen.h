#ifndef ENGINE_RANDOM_GEN_H
#define ENGINE_RANDOM_GEN_H

#include <cstdint>
#include <random>

namespace engine {

class RandomGen {
public:
    virtual ~RandomGen() = default;
    virtual double next_normal() = 0;
    virtual void seed(std::uint64_t s) = 0;
};

class MersenneTwisterGen : public RandomGen {
public:
    explicit MersenneTwisterGen(std::uint64_t s);
    double next_normal() override;
    void seed(std::uint64_t s) override;
private:
    std::mt19937_64 engine_;
    bool has_cached_;
    double cached_;
};

} // namespace engine
#endif // ENGINE_RANDOM_GEN_H
