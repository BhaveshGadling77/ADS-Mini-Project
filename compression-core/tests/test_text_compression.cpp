#include "core/Engine.hpp"
#include "datastructures/trie/Trie.hpp"
#include "datastructures/heap/MinHeap.hpp"
#include <iostream>
#include <cassert>
#include <sstream>
#include <string>
#include <vector>

void testTrie() {
    std::cout << "[TEST] Testing Trie Data Structure..." << std::endl;
    adsc::LzwTrie trie;
    auto root = trie.getRoot();
    assert(root != nullptr);

    // Initial 256 children must exist
    for (int i = 0; i < 256; ++i) {
        auto child = root->findChild(static_cast<uint8_t>(i));
        assert(child != nullptr);
        assert(child->code == i);
    }

    // Insert new multi-byte sequence
    auto childA = root->findChild('A');
    assert(childA != nullptr);
    auto childAB = trie.insertChild(childA, 'B', 256);
    assert(childAB != nullptr);
    assert(childAB->code == 256);

    auto foundAB = trie.findChild(childA, 'B');
    assert(foundAB == childAB);
    assert(foundAB->code == 256);

    std::cout << "  ✓ Trie tests passed!\n";
}

void testMinHeap() {
    std::cout << "[TEST] Testing MinHeap Data Structure..." << std::endl;
    adsc::MinHeap<int, std::less<int>> heap;
    assert(heap.empty());

    heap.push(42);
    heap.push(10);
    heap.push(50);
    heap.push(5);
    heap.push(20);

    assert(heap.size() == 5);
    assert(!heap.empty());

    std::vector<int> expected = {5, 10, 20, 42, 50};
    for (int exp : expected) {
        assert(heap.top() == exp);
        heap.pop();
    }
    assert(heap.empty());

    std::cout << "  ✓ MinHeap tests passed!\n";
}

void testAlgorithmRoundTrip(adsc::Algorithm algo, const std::string& algoName, const std::string& inputData) {
    auto compressor = adsc::CompressionEngine::createCompressor(algo);

    std::istringstream in(inputData);
    std::ostringstream compressed;

    adsc::CompressionMetrics metrics = compressor->compress(in, compressed);
    assert(metrics.success);
    assert(metrics.originalSizeBytes == inputData.size());

    std::string compressedStr = compressed.str();
    std::istringstream compIn(compressedStr);

    adsc::Header hdr = adsc::Header::read(compIn);
    assert(hdr.originalSize == inputData.size());

    std::ostringstream decompressed;
    try {
        compressor->decompress(compIn, decompressed, hdr.originalSize);
    } catch (const std::exception& e) {
        std::cerr << "Decompress EXCEPTION in " << algoName << ": " << e.what()
                  << " (inputData len=" << inputData.size() << ", comp len=" << compressedStr.size() << ")\n";
        throw;
    }

    std::string decompressedStr = decompressed.str();
    if (decompressedStr != inputData) {
        std::cerr << "FAILED for " << algoName << "!\nExpected len=" << inputData.size()
                  << ", got len=" << decompressedStr.size() << "\n";
        assert(false);
    }
}

void runRoundTripTests(const std::string& label, const std::string& data) {
    std::cout << "[TEST] " << label << " (len = " << data.size() << ")..." << std::endl;
    testAlgorithmRoundTrip(adsc::Algorithm::LZW, "LZW", data);
    testAlgorithmRoundTrip(adsc::Algorithm::HUFFMAN, "Huffman", data);
    testAlgorithmRoundTrip(adsc::Algorithm::BWT_MTF, "BWT", data);
    std::cout << "  ✓ LZW, Huffman, and BWT all passed for " << label << "!\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Running ADSC Compression Core Test Suite\n";
    std::cout << "========================================\n";

    testTrie();
    testMinHeap();

    // 1. Empty string
    runRoundTripTests("Empty String", "");

    // 2. Single character repeated
    runRoundTripTests("Repeated Character", std::string(1000, 'A'));

    // 3. Simple sentence
    runRoundTripTests("Simple Text", "Hello world! This is a test for data compression algorithms.");

    // 4. Pattern with high redundancy
    std::string pattern = "";
    for (int i = 0; i < 200; ++i) {
        pattern += "TOBEORNOTTOBEORTOBEORNOT";
    }
    runRoundTripTests("Repetitive Pattern (TOBEORNOT...)", pattern);

    // 5. Code sample
    std::string codeSample = R"(
#include <iostream>
int main() {
    for (int i = 0; i < 100; ++i) {
        std::cout << "Iteration: " << i << std::endl;
    }
    return 0;
}
)";
    runRoundTripTests("C++ Code Sample", codeSample);

    std::cout << "\n========================================\n";
    std::cout << "All Compression Tests Passed Successfully!\n";
    std::cout << "========================================\n";
    return 0;
}
