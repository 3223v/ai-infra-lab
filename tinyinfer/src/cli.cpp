#include "tinyinfer/cli.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace tinyinfer {

void print_usage(const char* program_name) {
    std::cout << "TinyInfer v0.1.0 — Lightweight LLM Inference Runtime\n\n";
    std::cout << "Usage: " << program_name << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --help                  Show this help\n";
    std::cout << "  --model <path>          Model directory (contains model.json + weights.bin)\n";
    std::cout << "  --input-ids <ids>       Comma-separated token ids, e.g. \"1,2,3\"\n";
    std::cout << "  --max-new-tokens <n>    Max tokens to generate (default: 32)\n";
    std::cout << "  --device <cpu|cuda>     Device to use (default: cpu)\n";
    std::cout << std::endl;
}

CliArgs parse_cli_args(int argc, char* argv[]) {
    CliArgs args;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help") {
            args.show_help = true;
            return args;
        } else if (arg == "--model") {
            if (i + 1 < argc) {
                args.model_path = argv[++i];
            } else {
                std::cerr << "Error: --model requires a path argument" << std::endl;
            }
        } else if (arg == "--input-ids") {
            if (i + 1 < argc) {
                std::string ids_str = argv[++i];
                std::stringstream ss(ids_str);
                std::string token;
                while (std::getline(ss, token, ',')) {
                    args.input_ids.push_back(std::stoi(token));
                }
            } else {
                std::cerr << "Error: --input-ids requires a comma-separated list" << std::endl;
            }
        } else if (arg == "--max-new-tokens") {
            if (i + 1 < argc) {
                args.max_new_tokens = std::stoi(argv[++i]);
            } else {
                std::cerr << "Error: --max-new-tokens requires a number" << std::endl;
            }
        } else if (arg == "--device") {
            if (i + 1 < argc) {
                args.device = argv[++i];
            } else {
                std::cerr << "Error: --device requires cpu or cuda" << std::endl;
            }
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
        }
    }

    return args;
}

} // namespace tinyinfer
