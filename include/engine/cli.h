#ifndef ENGINE_CLI_H
#define ENGINE_CLI_H

#include <iosfwd>

namespace engine::cli {

int  run_price                (std::ostream& out, int argc, char** argv);
int  run_convergence          (std::ostream& out, int argc, char** argv);
int  run_iv                   (std::ostream& out, std::ostream& err, int argc, char** argv);
int  run_compare_opt_internal (std::ostream& out, int argc, char** argv);
void print_usage              (std::ostream& out);

}

#endif
