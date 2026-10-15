#include <CLI/CLI.hpp>

int main(int argc, char** argv) {
    CLI::App app{"meshtool — Geomera CLI"};
    app.require_subcommand(0, 1);
    CLI11_PARSE(app, argc, argv);
    return 0;
}
