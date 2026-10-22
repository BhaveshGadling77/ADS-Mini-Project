
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <vector>

using namespace std;

struct Node {
    uint8_t symbol;
    uint64_t frequency;
    int minSymbol;
    shared_ptr<Node> left, right;

    Node(uint8_t s, uint64_t f)
        : symbol(s), frequency(f), minSymbol(s) {}

    Node(shared_ptr<Node> l, shared_ptr<Node> r)
        : symbol(0),
          frequency(l->frequency + r->frequency),
          minSymbol(min(l->minSymbol, r->minSymbol)),
          left(l), right(r) {}

    bool isLeaf() const {
        return !left && !right;
    }
};

struct CompareNodes {
    bool operator()(const shared_ptr<Node>& a,
                    const shared_ptr<Node>& b) const {
        if (a->frequency != b->frequency)
            return a->frequency > b->frequency;

        return a->minSymbol > b->minSymbol;
    }
};

void generateCodes(const shared_ptr<Node>& node,
                   const string& code,
                   array<string, 256>& codes) {
    if (!node) return;

    if (node->isLeaf()) {
        codes[node->symbol] = code.empty() ? "0" : code;
        return;
    }

    generateCodes(node->left, code + "0", codes);
    generateCodes(node->right, code + "1", codes);
}

bool runFFmpeg(const string& input, const string& output) {
    // Intended for ordinary local paths without embedded quotes.
    string command =
        "ffmpeg -nostdin -hide_banner -loglevel error -y "
        "-i \"" + input + "\" "
        "-map 0:a:0 -vn -f s16le -acodec pcm_s16le "
        "-ar 44100 -ac 2 \"" + output + "\"";

    cout << "Converting MP3 to raw PCM...\n";
    return system(command.c_str()) == 0;
}

bool compressPCM(const string& inputFile,
                 const string& outputFile) {
    ifstream input(inputFile, ios::binary);
    if (!input) {
        cerr << "Cannot open PCM input file.\n";
        return false;
    }

    array<uint64_t, 256> frequencies{};
    uint64_t originalSize = 0;
    char ch;

    while (input.get(ch)) {
        uint8_t byte = static_cast<uint8_t>(
            static_cast<unsigned char>(ch));
        frequencies[byte]++;
        originalSize++;
    }

    if (input.bad()) {
        cerr << "Error reading PCM file.\n";
        return false;
    }
    input.close();

    ofstream output(outputFile, ios::binary);
    if (!output) {
        cerr << "Cannot create compressed file.\n";
        return false;
    }

    output.write("HUF1", 4);

    auto writeU64 = [&](uint64_t value) {
        for (int i = 0; i < 8; ++i) {
            output.put(static_cast<char>(value & 0xFF));
            value >>= 8;
        }
    };

    writeU64(originalSize);

    for (uint64_t f : frequencies)
        writeU64(f);

    if (originalSize == 0) {
        output.close();
        cout << "PCM file is empty.\n";
        return true;
    }

    priority_queue<shared_ptr<Node>,
                   vector<shared_ptr<Node>>,
                   CompareNodes> minHeap;

    for (int i = 0; i < 256; ++i) {
        if (frequencies[i])
            minHeap.push(make_shared<Node>(
                static_cast<uint8_t>(i), frequencies[i]));
    }

    while (minHeap.size() > 1) {
        auto left = minHeap.top();
        minHeap.pop();
        auto right = minHeap.top();
        minHeap.pop();
        minHeap.push(make_shared<Node>(left, right));
    }

    auto root = minHeap.top();
    array<string, 256> codes{};
    generateCodes(root, "", codes);

    input.open(inputFile, ios::binary);
    if (!input) {
        cerr << "Cannot reopen PCM file.\n";
        return false;
    }

    uint8_t buffer = 0;
    int bitsFilled = 0;

    while (input.get(ch)) {
        uint8_t byte = static_cast<uint8_t>(
            static_cast<unsigned char>(ch));

        for (char bit : codes[byte]) {
            buffer = static_cast<uint8_t>(
                (buffer << 1) | (bit == '1'));
            bitsFilled++;

            if (bitsFilled == 8) {
                output.put(static_cast<char>(buffer));
                buffer = 0;
                bitsFilled = 0;
            }
        }
    }

    if (bitsFilled > 0) {
        buffer = static_cast<uint8_t>(
            buffer << (8 - bitsFilled));
        output.put(static_cast<char>(buffer));
    }

    input.close();
    output.close();

    if (!output) {
        cerr << "Error writing compressed file.\n";
        return false;
    }

    ifstream compressed(outputFile, ios::binary | ios::ate);
    if (!compressed) return false;

    uint64_t compressedSize =
        static_cast<uint64_t>(compressed.tellg());

    cout << "\nHuffman compression completed.\n";
    cout << "Original PCM size: " << originalSize << " bytes\n";
    cout << "Compressed size: " << compressedSize << " bytes\n";

    if (compressedSize > 0) {
        cout << "Compression ratio: "
             << static_cast<double>(originalSize) / compressedSize
             << ":1\n";

        cout << "Space saved: "
             << (1.0 - static_cast<double>(compressedSize) /
                       originalSize) * 100.0
             << "%\n";
    }

    return true;
}

int main(int argc, char* argv[]) {
    string mp3File = argc > 1 ? argv[1] : "sample.mp3";
    string compressedFile =
        argc > 2 ? argv[2] : "compressed_audio.huf";
    string pcmFile = "audio_temp_pcm.pcm";

    if (!runFFmpeg(mp3File, pcmFile)) {
        cerr << "FFmpeg conversion failed. Check the MP3 path "
             << "and FFmpeg installation.\n";
        return 1;
    }

    if (!compressPCM(pcmFile, compressedFile))
        return 1;

    cout << "Compressed audio saved to: "
         << compressedFile << '\n';

    return 0;
}
