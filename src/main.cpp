#include "engine/cli.h"
#include <iostream>
#include <string_view>

int main(int argc, char** argv) {
    const std::string_view subcommand = (argc >= 2)
        ? std::string_view{argv[1]}
        : std::string_view{"price"};

    if (subcommand == "price") {
        return engine::cli::run_price(std::cout, argc, argv);
    } else if (subcommand == "convergence") {
        return engine::cli::run_convergence(std::cout, argc, argv);
    } else if (subcommand == "iv") {
        return engine::cli::run_iv(std::cout, std::cerr, argc, argv);
    } else if (subcommand == "compare-opt-internal") {
        return engine::cli::run_compare_opt_internal(std::cout, argc, argv);
    } else if (subcommand == "--help" || subcommand == "-h" || subcommand == "help") {
        engine::cli::print_usage(std::cout);
        return 0;
    } else {
        engine::cli::print_usage(std::cerr);
        return 1;
    }
}
