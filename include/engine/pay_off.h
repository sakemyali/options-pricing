#ifndef ENGINE_PAY_OFF_H
#define ENGINE_PAY_OFF_H

#include <memory>

namespace engine {

class PayOff {
public:
    virtual ~PayOff() = default;
    virtual double compute(double spot) const = 0;
    virtual std::unique_ptr<PayOff> clone() const = 0;
};

class PayOffCall : public PayOff {
public:
    explicit PayOffCall(double strike);
    double compute(double spot) const override;
    std::unique_ptr<PayOff> clone() const override;
private:
    double strike_;
};

class PayOffPut : public PayOff {
public:
    explicit PayOffPut(double strike);
    double compute(double spot) const override;
    std::unique_ptr<PayOff> clone() const override;
private:
    double strike_;
};

} // namespace engine
#endif // ENGINE_PAY_OFF_H
