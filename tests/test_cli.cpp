#include "catch_amalgamated.hpp"
#include "engine/cli.h"
#include <sstream>
#include <string>

TEST_CASE("price subcommand emits Hull Set 1 BS call price",
          "[cli][price]") {
    char prog[]   = "pricing_engine";
    char sub[]    = "price";
    char* argv[]  = {prog, sub};
    std::ostringstream oss;
    REQUIRE(engine::cli::run_price(oss, 2, argv) == 0);
    REQUIRE_THAT(oss.str(), Catch::Matchers::ContainsSubstring("4.759422"));
    REQUIRE_THAT(oss.str(), Catch::Matchers::ContainsSubstring("Delta"));
    REQUIRE_THAT(oss.str(), Catch::Matchers::ContainsSubstring("Gamma"));
    REQUIRE_THAT(oss.str(), Catch::Matchers::ContainsSubstring("Vega"));
    REQUIRE_THAT(oss.str(), Catch::Matchers::ContainsSubstring("Theta"));
    REQUIRE_THAT(oss.str(), Catch::Matchers::ContainsSubstring("Rho"));
    REQUIRE_THAT(oss.str(), Catch::Matchers::ContainsSubstring("MC SE"));
}

TEST_CASE("price subcommand --put emits Hull Set 1 BS put price",
          "[cli][price]") {
    char prog[]   = "pricing_engine";
    char sub[]    = "price";
    char put_flag[] = "--put";
    char* argv[]  = {prog, sub, put_flag};
    std::ostringstream oss;
    REQUIRE(engine::cli::run_price(oss, 3, argv) == 0);
    REQUIRE_THAT(oss.str(), Catch::Matchers::ContainsSubstring("0.808599"));
    REQUIRE_THAT(oss.str(), Catch::Matchers::ContainsSubstring("Put"));
}

TEST_CASE("convergence subcommand emits all five N values",
          "[cli][convergence][.slow]") {
    char prog[] = "pricing_engine";
    char sub[]  = "convergence";
    char* argv[] = {prog, sub};
    std::ostringstream oss;
    REQUIRE(engine::cli::run_convergence(oss, 2, argv) == 0);
    for (const char* n : {"1000", "10000", "100000", "1000000", "10000000"}) {
        REQUIRE_THAT(oss.str(), Catch::Matchers::ContainsSubstring(std::string{n}));
    }
    REQUIRE_THAT(oss.str(), Catch::Matchers::ContainsSubstring("plain"));
    REQUIRE_THAT(oss.str(), Catch::Matchers::ContainsSubstring("anti"));
}

TEST_CASE("iv subcommand round-trips Hull Set 1 sigma",
          "[cli][iv]") {
    char prog[]    = "pricing_engine";
    char sub[]     = "iv";
    char flag[]    = "--market-price";
    char value[]   = "4.759422392871535";
    char* argv[]   = {prog, sub, flag, value};
    std::ostringstream oss, err;
    REQUIRE(engine::cli::run_iv(oss, err, 4, argv) == 0);
    REQUIRE_THAT(oss.str(), Catch::Matchers::ContainsSubstring("Recovered sigma"));
    REQUIRE_THAT(oss.str(), Catch::Matchers::ContainsSubstring("0.20"));
}

TEST_CASE("iv subcommand prints friendly error on missing --market-price",
          "[cli][iv]") {
    char prog[] = "pricing_engine";
    char sub[]  = "iv";
    char* argv[] = {prog, sub};
    std::ostringstream oss, err;
    REQUIRE(engine::cli::run_iv(oss, err, 2, argv) == 2);
    REQUIRE_THAT(err.str(), Catch::Matchers::ContainsSubstring("error"));
}

TEST_CASE("compare-opt-internal emits elapsed + throughput + label",
          "[cli][compare_opt]") {
    char prog[]      = "pricing_engine";
    char sub[]       = "compare-opt-internal";
    char label_flag[]= "--label";
    char label_val[] = "TEST_LBL";
    char paths_flag[]= "--paths";
    char paths_val[] = "10000";
    char* argv[]     = {prog, sub, label_flag, label_val, paths_flag, paths_val};
    std::ostringstream oss;
    REQUIRE(engine::cli::run_compare_opt_internal(oss, 6, argv) == 0);
    REQUIRE_THAT(oss.str(), Catch::Matchers::ContainsSubstring("[TEST_LBL]"));
    REQUIRE_THAT(oss.str(), Catch::Matchers::ContainsSubstring("elapsed"));
    REQUIRE_THAT(oss.str(), Catch::Matchers::ContainsSubstring("throughput"));
}
