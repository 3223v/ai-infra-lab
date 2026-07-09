#include "tinyinfer/cli.hpp"

#include <iostream>

int main(int argc, char* argv[]) {
    tinyinfer::CliArgs args = tinyinfer::parse_cli_args(argc, argv);

    if (args.show_help) {
        tinyinfer::print_usage(argv[0]);
        return 0;
    }

    std::cout << "TinyInfer v0.1.0 — Lightweight LLM Inference Runtime" << std::endl;
    std::cout << "model:      " << (args.model_path.empty() ? "(not specified)" : args.model_path) << std::endl;
    std::cout << "device:     " << args.device << std::endl;
    std::cout << "max_tokens: " << args.max_new_tokens << std::endl;

    if (!args.input_ids.empty()) {
        std::cout << "input_ids:  ";
        for (size_t i = 0; i < args.input_ids.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << args.input_ids[i];
        }
        std::cout << std::endl;
    }

    return 0;
}
