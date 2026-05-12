# options-pricing

A small C++17 options pricing engine. European options under Black-Scholes,
priced two ways (closed-form + Monte Carlo) with both analytical and
finite-difference Greeks, antithetic variance reduction, and a
Newton-with-bisection-fallback implied volatility solver.

Cross-validated against Hull's *Options, Futures, and Other Derivatives*
(reference parameter sets from chapters 15, 19, and 21).

## Build

```
make           # builds build/pricing_engine
make test      # builds and runs the Catch2 test suite
make debug     # rebuild with -O0 -g + ASan/UBSan
make clean
```

Only dependency is Catch2, vendored in `third_party/`.

## CLI

```
$ ./build/pricing_engine price
Black-Scholes European Call (Hull Set 1 default)
------------------------------------------------
Parameter                              Value
------------------------------------------------
Spot (S)                           42.000000
Strike (K)                         40.000000
Rate (r)                            0.100000
Vol (sigma)                         0.200000
Expiry (T)                          0.500000
------------------------------------------------
BS Price                            4.759422
MC Price                            4.767208
MC SE                              4.970e-03
Delta                               0.779131
Gamma                               0.049963
Vega                                8.813415
Theta                              -4.559092
Rho                                13.982046
```

Subcommands:

- `price` — Black-Scholes price + Greeks, plus a Monte Carlo cross-check (default).
- `convergence` — MC price + standard error swept across `N ∈ {1K, 10K, 100K, 1M, 10M}`,
  shown side by side for plain and antithetic variants against the analytical price.
- `iv --market-price P` — solve for the volatility implied by a given option price.
- `--help` — full flag reference.

Common flags work for any subcommand: `--spot`, `--strike`, `--rate`, `--vol`,
`--expiry`, `--paths`, `--seed`, `--type call|put`. Defaults are Hull Set 1
(S=42, K=40, r=0.10, σ=0.20, T=0.5).

Examples:

```
./build/pricing_engine price --type put --spot 100 --strike 95 --vol 0.3
./build/pricing_engine convergence
./build/pricing_engine iv --market-price 4.76
```

## Layout

```
include/engine/   public headers (one per component)
src/              implementations + main.cpp dispatcher
tests/            Catch2 tests (one file per component, plus integration)
third_party/      vendored Catch2 amalgamated
```

Components:

| Header                      | What it does                                          |
| --------------------------- | ----------------------------------------------------- |
| `option.h`                  | `Option` container + `MarketData` struct              |
| `pay_off.h`                 | `PayOff` abstract base + Call/Put with clone pattern  |
| `pricing_result.h`          | `PricingResult` (price, SE, optional Greeks)          |
| `normal_cdf.h`              | Standard normal CDF/PDF via `std::erfc`               |
| `black_scholes.h`           | Closed-form European call/put pricing                 |
| `analytical_greeks.h`       | All five Greeks for call/put + price-with-Greeks     |
| `random_gen.h`              | RNG interface, Mersenne Twister, antithetic wrapper   |
| `statistics_gatherer.h`     | Welford's online mean/variance                        |
| `monte_carlo_pricer.h`      | GBM-terminal-distribution MC pricer                   |
| `greek_calculator.h`        | Pricer-agnostic central-difference Greeks             |
| `implied_volatility.h`      | Newton with bisection-fallback IV solver              |
| `cli.h`                     | Subcommand dispatch and flag parsing                  |

## Testing

`make test` runs everything (~290 assertions across ~70 cases). A few tests
are tagged `[.slow]` (MC convergence at N=1M, antithetic ensemble) and
`[.benchmark]` (throughput numbers); they're included by default. Add a
Catch2 tag selector to skip:

```
./build/test_runner "~[.slow]" "~[.benchmark]"
```

## Performance check

`make compare-opt` builds two copies of the engine — one at `-O0`, one at
`-O2` — and runs the same Monte Carlo workload through both, reporting
throughput in M sims/sec. Useful sanity check that the inner loop is
actually being optimized.

## References

- Hull, *Options, Futures, and Other Derivatives*, 11th ed.
- Glasserman, *Monte Carlo Methods in Financial Engineering*, ch. 4 (variance reduction).
- Joshi, *C++ Design Patterns and Derivatives Pricing* (the PayOff hierarchy
  and pricer/RNG injection patterns are directly inspired by this book).
