#include "core/Engine.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <filesystem>

void printHelp() {
    std::cout << "Bitshrink Compression Engine CLI\n";
    std::cout << "Usage:\n";
    std::cout << "  compression-engine compress -a <algo> [-m <text|image|audio|video>] -i <input> -o <output> [--json]\n";
    std::cout << "  compression-engine decompress -i <input> -o <output> [-a <algo>] [--json]\n";
    std::cout << "  compression-engine info -i <input> [--json]\n";
    std::cout << "  compression-engine compare -i <input> [--json]\n";
}

void printJsonMetrics(const adsc::CompressionMetrics& m, const std::string& op, const std::string& inPath, const std::string& outPath) {
    std::cout << "{"
              << "\"success\":" << (m.success ? "true" : "false") << ","
              << "\"operation\":\"" << op << "\","
              << "\"algorithm\":\"" << m.algorithm << "\","
              << "\"modality\":\"" << m.modality << "\","
              << "\"originalSizeBytes\":" << m.originalSizeBytes << ","
              << "\"compressedSizeBytes\":" << m.compressedSizeBytes << ","
              << "\"compressionRatio\":" << std::fixed << std::setprecision(4) << m.compressionRatio << ","
              << "\"spaceSavedPercent\":" << std::fixed << std::setprecision(2) << m.spaceSavedPercent << ","
              << "\"durationMs\":" << std::fixed << std::setprecision(2) << m.executionTimeMs << ","
              << "\"inputPath\":\"" << inPath << "\","
              << "\"outputPath\":\"" << outPath << "\""
              << "}\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 1;
    }

    std::string command = argv[1];
    std::string algoStr = "";
    std::string modalityStr = "text";
    std::string inputPath = "";
    std::string outputPath = "";
    bool jsonOutput = false;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-a" || arg == "--algo") && i + 1 < argc) {
            algoStr = argv[++i];
        } else if ((arg == "-m" || arg == "--modality") && i + 1 < argc) {
            modalityStr = argv[++i];
        } else if ((arg == "-i" || arg == "--input") && i + 1 < argc) {
            inputPath = argv[++i];
        } else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            outputPath = argv[++i];
        } else if (arg == "--json") {
            jsonOutput = true;
        } else if (arg == "-h" || arg == "--help") {
            printHelp();
            return 0;
        }
    }

    adsc::CompressionEngine engine;

    try {
        if (command == "compress") {
            if (inputPath.empty() || outputPath.empty()) {
                std::cerr << "Error: --input and --output are required for compress\n";
                return 1;
            }

            if (algoStr.empty()) {
                algoStr = "lzw";
            }
            adsc::Algorithm algo = adsc::stringToAlgorithm(algoStr);
            adsc::CompressionMetrics m = engine.compressFile(inputPath, outputPath, algo);
            m.modality = modalityStr;

            if (jsonOutput) {
                printJsonMetrics(m, "compress", inputPath, outputPath);
            } else {
                std::cout << "Successfully compressed with " << m.algorithm << ":\n"
                          << "  Original Size   : " << m.originalSizeBytes << " bytes\n"
                          << "  Compressed Size : " << m.compressedSizeBytes << " bytes\n"
                          << "  Ratio           : " << std::fixed << std::setprecision(2) << m.compressionRatio << ":1\n"
                          << "  Space Saved     : " << std::fixed << std::setprecision(2) << m.spaceSavedPercent << "%\n"
                          << "  Execution Time  : " << std::fixed << std::setprecision(2) << m.executionTimeMs << " ms\n";
            }
            return 0;
        } else if (command == "decompress") {
            if (inputPath.empty() || outputPath.empty()) {
                std::cerr << "Error: --input and --output are required for decompress\n";
                return 1;
            }

            std::optional<adsc::Algorithm> algoOpt;
            if (!algoStr.empty() && algoStr != "auto") {
                try {
                    algoOpt = adsc::stringToAlgorithm(algoStr);
                } catch (...) {
                    // Let engine auto-detect from header
                }
            }

            adsc::CompressionMetrics m = engine.decompressFile(inputPath, outputPath, algoOpt);

            if (jsonOutput) {
                printJsonMetrics(m, "decompress", inputPath, outputPath);
            } else {
                std::cout << "Successfully decompressed with " << m.algorithm << ":\n"
                          << "  Compressed Size : " << m.compressedSizeBytes << " bytes\n"
                          << "  Restored Size   : " << m.originalSizeBytes << " bytes\n"
                          << "  Execution Time  : " << std::fixed << std::setprecision(2) << m.executionTimeMs << " ms\n";
            }
            return 0;
        } else if (command == "info") {
            if (inputPath.empty()) {
                std::cerr << "Error: --input is required for info\n";
                return 1;
            }

            adsc::FileInfo info = engine.inspectFile(inputPath);
            if (jsonOutput) {
                std::cout << "{"
                          << "\"isCompressed\":" << (info.isCompressed ? "true" : "false") << ","
                          << "\"algorithm\":\"" << info.algorithm << "\","
                          << "\"version\":" << static_cast<int>(info.version) << ","
                          << "\"originalSizeBytes\":" << info.originalSize << ","
                          << "\"fileSizeBytes\":" << info.fileSize
                          << "}\n";
            } else {
                std::cout << "File Information for " << inputPath << ":\n"
                          << "  Is Compressed : " << (info.isCompressed ? "Yes" : "No") << "\n";
                if (info.isCompressed) {
                    std::cout << "  Algorithm     : " << info.algorithm << "\n"
                              << "  Version       : " << static_cast<int>(info.version) << "\n"
                              << "  Original Size : " << info.originalSize << " bytes\n";
                }
                std::cout << "  File Size     : " << info.fileSize << " bytes\n";
            }
            return 0;
        } else if (command == "compare") {
            if (inputPath.empty()) {
                std::cerr << "Error: --input is required for compare\n";
                return 1;
            }

            std::vector<adsc::Algorithm> algos = {
                adsc::Algorithm::LZW,
                adsc::Algorithm::HUFFMAN,
                adsc::Algorithm::BWT_MTF
            };

            std::string tempPrefix = std::filesystem::temp_directory_path().string() + "/adsc_compare_";

            if (jsonOutput) {
                std::cout << "{\"inputPath\":\"" << inputPath << "\",\"results\":[";
            } else {
                std::cout << "Algorithm Comparison for: " << inputPath << "\n";
                std::cout << "\n";
                std::cout << std::left << std::setw(15) << "Algorithm"
                          << std::setw(16) << "Original (B)"
                          << std::setw(16) << "Compressed (B)"
                          << std::setw(14) << "Ratio"
                          << std::setw(14) << "Saved (%)"
                          << std::setw(12) << "Time (ms)" << "\n";
                std::cout << "\n";
            }

            bool first = true;
            for (auto a : algos) {
                std::string tmpOut = tempPrefix + adsc::algorithmToString(a) + ".adsc";
                try {
                    adsc::CompressionMetrics m = engine.compressFile(inputPath, tmpOut, a);
                    if (jsonOutput) {
                        if (!first) std::cout << ",";
                        printJsonMetrics(m, "compare", inputPath, tmpOut);
                        first = false;
                    } else {
                        std::cout << std::left << std::setw(15) << m.algorithm
                                  << std::setw(16) << m.originalSizeBytes
                                  << std::setw(16) << m.compressedSizeBytes
                                  << std::setw(14) << (std::to_string(m.compressionRatio).substr(0, 5) + ":1")
                                  << std::setw(14) << (std::to_string(m.spaceSavedPercent).substr(0, 5) + "%")
                                  << std::setw(12) << std::fixed << std::setprecision(2) << m.executionTimeMs << "\n";
                    }
                    std::filesystem::remove(tmpOut);
                } catch (const std::exception& e) {
                    if (jsonOutput) {
                        if (!first) std::cout << ",";
                        std::cout << "{\"algorithm\":\"" << adsc::algorithmToString(a)
                                  << "\",\"success\":false,\"error\":\"" << e.what() << "\"}";
                        first = false;
                    } else {
                        std::cout << std::left << std::setw(15) << adsc::algorithmToString(a)
                                  << "Error: " << e.what() << "\n";
                    }
                }
            }

            if (jsonOutput) {
                std::cout << "]}\n";
            } else {
                std::cout << "\n";
            }
            return 0;
        } else {
            std::cerr << "Unknown command: " << command << "\n";
            printHelp();
            return 1;
        }
    } catch (const std::exception& e) {
        if (jsonOutput) {
            std::cout << "{\"success\":false,\"error\":\"" << e.what() << "\"}\n";
        } else {
            std::cerr << "Error: " << e.what() << "\n";
        }
        return 1;
    }
}
