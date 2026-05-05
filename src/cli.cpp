#include "engine/cli.h"
#include "engine/black_scholes.h"
#include "engine/analytical_greeks.h"
#include "engine/implied_volatility.h"
#include "engine/monte_carlo_pricer.h"
#include "engine/random_gen.h"
#include "engine/option.h"
#include "engine/pay_off.h"
#include "engine/pricing_result.h"
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
    constexpr double K_HULL = 40.0, T_HULL = 0.5, S_HULL = 42.0, R_HULL = 0.10, SIGMA_HULL = 0.20;
    constexpr std::uint64_t SEED_DEFAULT = 42;
    constexpr std::size_t   PATHS_DEFAULT = 1'000'000;

    std::optional<std::string_view> get_flag_value(int argc, char** argv, std::string_view name) {
        for (int i = 1; i + 1 < argc; ++i) {
            if (name == argv[i]) {
                return std::string_view{argv[i + 1]};
            }
        }
        return std::nullopt;
    }

    std::optional<double> get_flag_double(int argc, char** argv, std::string_view name) {
        auto v = get_flag_value(argc, argv, name);
        if (!v) return std::nullopt;
        try {
            return std::stod(std::string{*v});
        } catch (...) {
            return std::nullopt;
        }
    }

    std::optional<std::uint64_t> get_flag_uint64(int argc, char** argv, std::string_view name) {
        auto v = get_flag_value(argc, argv, name);
        if (!v) return std::nullopt;
        try {
            return static_cast<std::uint64_t>(std::stoull(std::string{*v}));
        } catch (...) {
            return std::nullopt;
        }
    }

    bool has_flag(int argc, char** argv, std::string_view name) {
        for (int i = 1; i < argc; ++i) {
            if (name == argv[i]) return true;
        }
        return false;
    }

    bool is_put_requested(int argc, char** argv) {
        if (has_flag(argc, argv, "--put")) return true;
        auto t = get_flag_value(argc, argv, "--type");
        return t && (*t == "put" || *t == "Put" || *t == "PUT");
    }

    struct Fixture {
        double S, K, T, r, sigma;
        std::uint64_t seed;
        std::size_t paths;
        bool is_put;
    };

    Fixture parse_fixture(int argc, char** argv) {
        Fixture f{S_HULL, K_HULL, T_HULL, R_HULL, SIGMA_HULL, SEED_DEFAULT, PATHS_DEFAULT, false};
        if (auto v = get_flag_double(argc, argv, "--S"))      f.S = *v;
        if (auto v = get_flag_double(argc, argv, "--spot"))   f.S = *v;
        if (auto v = get_flag_double(argc, argv, "--K"))      f.K = *v;
        if (auto v = get_flag_double(argc, argv, "--strike")) f.K = *v;
        if (auto v = get_flag_double(argc, argv, "--T"))      f.T = *v;
        if (auto v = get_flag_double(argc, argv, "--expiry")) f.T = *v;
        if (auto v = get_flag_double(argc, argv, "--r"))      f.r = *v;
        if (auto v = get_flag_double(argc, argv, "--rate"))   f.r = *v;
        if (auto v = get_flag_double(argc, argv, "--sigma"))  f.sigma = *v;
        if (auto v = get_flag_double(argc, argv, "--vol"))    f.sigma = *v;
        if (auto v = get_flag_uint64(argc, argv, "--seed"))   f.seed = *v;
        if (auto v = get_flag_uint64(argc, argv, "--paths"))  f.paths = static_cast<std::size_t>(*v);
        f.is_put = is_put_requested(argc, argv);
        return f;
    }

    engine::Option build_option(const Fixture& f) {
        if (f.is_put) {
            engine::PayOffPut po(f.K);
            return engine::Option(f.K, f.T, po);
        }
        engine::PayOffCall po(f.K);
        return engine::Option(f.K, f.T, po);
    }
}

int engine::cli::run_price(std::ostream& out, int argc, char** argv) {
    const Fixture f = parse_fixture(argc, argv);
    engine::Option       option = build_option(f);
    engine::MarketData   market{f.S, f.r, f.sigma};

    engine::PricingResult bs = f.is_put
        ? engine::black_scholes_put_with_greeks(option, market)
        : engine::black_scholes_call_with_greeks(option, market);

    engine::MersenneTwisterGen rng(f.seed);
    engine::MonteCarloPricer   mcp(rng, f.paths);
    engine::PricingResult mc = mcp.price(option, market);

    out << "Black-Scholes European " << (f.is_put ? "Put" : "Call")
        << " (Hull Set 1 default)\n";
    out << "------------------------------------------------\n";
    out << std::left  << std::setw(24) << "Parameter"
        << std::right << std::setw(20) << "Value" << "\n";
    out << "------------------------------------------------\n";
    out << std::fixed << std::setprecision(6);
    out << std::left << std::setw(24) << "Spot (S)"      << std::right << std::setw(20) << f.S    << "\n";
    out << std::left << std::setw(24) << "Strike (K)"    << std::right << std::setw(20) << f.K    << "\n";
    out << std::left << std::setw(24) << "Rate (r)"      << std::right << std::setw(20) << f.r    << "\n";
    out << std::left << std::setw(24) << "Vol (sigma)"   << std::right << std::setw(20) << f.sigma<< "\n";
    out << std::left << std::setw(24) << "Expiry (T)"    << std::right << std::setw(20) << f.T    << "\n";
    out << "------------------------------------------------\n";
    out << std::left << std::setw(24) << "BS Price"      << std::right << std::setw(20) << bs.price << "\n";
    out << std::left << std::setw(24) << "MC Price"      << std::right << std::setw(20) << mc.price << "\n";
    out << std::left << std::setw(24) << "MC SE";
    out << std::right << std::setw(20) << std::scientific << std::setprecision(3) << mc.standard_error << "\n";
    out << std::fixed << std::setprecision(6);
    if (bs.greeks.has_value()) {
        const auto& g = *bs.greeks;
        out << std::left << std::setw(24) << "Delta"     << std::right << std::setw(20) << g.delta << "\n";
        out << std::left << std::setw(24) << "Gamma"     << std::right << std::setw(20) << g.gamma << "\n";
        out << std::left << std::setw(24) << "Vega"      << std::right << std::setw(20) << g.vega  << "\n";
        out << std::left << std::setw(24) << "Theta"     << std::right << std::setw(20) << g.theta << "\n";
        out << std::left << std::setw(24) << "Rho"       << std::right << std::setw(20) << g.rho   << "\n";
    }
    return 0;
}

int engine::cli::run_convergence(std::ostream& out, int argc, char** argv) {
    const Fixture f = parse_fixture(argc, argv);
    engine::Option     option = build_option(f);
    engine::MarketData market{f.S, f.r, f.sigma};

    const double bs_price = f.is_put
        ? engine::black_scholes_put(option, market).price
        : engine::black_scholes_call(option, market).price;

    out << "Convergence demo (" << (f.is_put ? "Put" : "Call")
        << ", Hull Set 1, seed=" << f.seed << ")\n";
    out << "------------------------------------------------------------------------------\n";
    out << std::left  << std::setw(12) << "N"
        << std::right << std::setw(14) << "plain price"
        << std::right << std::setw(14) << "plain SE"
        << std::right << std::setw(14) << "anti price"
        << std::right << std::setw(14) << "anti SE"
        << std::right << std::setw(14) << "|MC-BS|" << "\n";
    out << "------------------------------------------------------------------------------\n";
    const std::vector<std::size_t> Ns = {1'000, 10'000, 100'000, 1'000'000, 10'000'000};
    for (std::size_t N : Ns) {
        engine::MersenneTwisterGen rng_plain(f.seed);
        engine::MonteCarloPricer pricer_plain(rng_plain, N);
        const auto plain = pricer_plain.price(option, market);

        engine::MersenneTwisterGen rng_base_anti(f.seed);
        engine::AntitheticGen      anti(rng_base_anti);
        engine::MonteCarloPricer pricer_anti(anti, N);
        const auto a = pricer_anti.price(option, market);

        out << std::left  << std::setw(12) << N
            << std::right << std::setw(14) << std::fixed << std::setprecision(6) << plain.price
            << std::right << std::setw(14) << std::scientific << std::setprecision(3) << plain.standard_error
            << std::right << std::setw(14) << std::fixed << std::setprecision(6) << a.price
            << std::right << std::setw(14) << std::scientific << std::setprecision(3) << a.standard_error
            << std::right << std::setw(14) << std::scientific << std::setprecision(3) << std::abs(plain.price - bs_price)
            << "\n";
    }
    out << "------------------------------------------------------------------------------\n";
    out << "BS analytical price: " << std::fixed << std::setprecision(6) << bs_price << "\n";
    return 0;
}

int engine::cli::run_iv(std::ostream& out, std::ostream& err, int argc, char** argv) {
    const Fixture f = parse_fixture(argc, argv);
    const auto market_price_opt = get_flag_double(argc, argv, "--market-price");
    if (!market_price_opt) {
        err << "error: iv requires --market-price <number>\n";
        return 2;
    }
    const double market_price = *market_price_opt;
    const double sigma_init = get_flag_double(argc, argv, "--sigma-init").value_or(0.20);

    engine::Option option = build_option(f);
    engine::MarketData market{f.S, f.r, 0.0};

    try {
        const double iv = engine::implied_volatility(option, market, market_price, sigma_init);
        out << "Implied volatility (round-trip)\n";
        out << "------------------------------------------------\n";
        out << std::fixed << std::setprecision(8);
        out << std::left << std::setw(24) << "Market price"      << std::right << std::setw(20) << market_price << "\n";
        out << std::left << std::setw(24) << "Initial guess"     << std::right << std::setw(20) << sigma_init << "\n";
        out << std::left << std::setw(24) << "Recovered sigma"   << std::right << std::setw(20) << iv << "\n";
        return 0;
    } catch (const std::runtime_error& e) {
        err << "error: implied volatility solver failed: " << e.what() << "\n";
        return 3;
    }
}

int engine::cli::run_compare_opt_internal(std::ostream& out, int argc, char** argv) {
    const Fixture f = parse_fixture(argc, argv);
    const std::string label = std::string{
        get_flag_value(argc, argv, "--label").value_or(std::string_view{"unspecified"})
    };

    engine::PayOffCall po(f.K);
    engine::Option option(f.K, f.T, po);
    engine::MarketData market{f.S, f.r, f.sigma};

    engine::MersenneTwisterGen rng(f.seed);
    engine::MonteCarloPricer pricer(rng, f.paths);

    const auto t0 = std::chrono::steady_clock::now();
    const auto result = pricer.price(option, market);
    const auto t1 = std::chrono::steady_clock::now();

    volatile double sink = result.price;
    (void)sink;

    const double elapsed_s    = std::chrono::duration<double>(t1 - t0).count();
    const double sims_per_sec = static_cast<double>(f.paths) / elapsed_s;
    const double m_sims_per_sec = sims_per_sec / 1.0e6;

    out << "[" << label << "] paths=" << f.paths
        << " elapsed=" << std::fixed << std::setprecision(3) << (elapsed_s * 1000.0) << " ms"
        << " throughput=" << std::fixed << std::setprecision(3) << m_sims_per_sec << " M sims/sec"
        << "\n";
    return 0;
}

void engine::cli::print_usage(std::ostream& out) {
    out << "pricing_engine -- options pricing engine demo\n"
        << "\n"
        << "Usage: pricing_engine [SUBCOMMAND] [FLAGS]\n"
        << "\n"
        << "Subcommands:\n"
        << "  price                   Price + Greeks for an option (default)\n"
        << "  convergence             MC convergence sweep at N in {1K, 10K, 100K, 1M, 10M}\n"
        << "  iv --market-price P     Solve for implied volatility from a market price\n"
        << "  compare-opt-internal    (worker) -- single timed MC run for `make compare-opt`\n"
        << "  --help | -h | help      Show this banner\n"
        << "\n"
        << "Common flags:\n"
        << "  --S | --spot N          Spot price (default 42)\n"
        << "  --K | --strike N        Strike (default 40)\n"
        << "  --T | --expiry N        Expiry in years (default 0.5)\n"
        << "  --r | --rate N          Risk-free rate (default 0.10)\n"
        << "  --sigma | --vol N       Volatility (default 0.20)\n"
        << "  --paths N               MC paths (default 1000000)\n"
        << "  --seed N                RNG seed (default 42)\n"
        << "  --type call|put | --put Option type (default call)\n"
        << "  --label NAME            (compare-opt-internal) build label, e.g. O0, O2\n"
        << "  --market-price N        (iv) target market price\n"
        << "  --sigma-init N          (iv) Newton initial guess (default 0.20)\n";
}
