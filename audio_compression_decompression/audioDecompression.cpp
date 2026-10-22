
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
#include <limits>

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

bool readU64(ifstream& input, uint64_t& value) {
    value = 0;

    for (int i = 0; i < 8; ++i) {
        char ch;
        if (!input.get(ch)) return false;

        uint8_t byte = static_cast<uint8_t>(
            static_cast<unsigned char>(ch));

        value |= static_cast<uint64_t>(byte) << (8 * i);
    }

    return true;
}

bool decompressHuffman(const string& inputFile,
                       const string& outputFile) {
    ifstream input(inputFile, ios::binary);
    if (!input) {
        cerr << "Cannot open compressed file.\n";
        return false;
    }

    char magic[4];
    if (!input.read(magic, 4) ||
        string(magic, 4) != "HUF1") {
        cerr << "Invalid compressed file format.\n";
        return false;
    }

    uint64_t originalSize;
    if (!readU64(input, originalSize)) {
        cerr << "Cannot read original file size.\n";
        return false;
    }

    array<uint64_t, 256> frequencies{};
    uint64_t frequencySum = 0;

    for (int i = 0; i < 256; ++i) {
        if (!readU64(input, frequencies[i])) {
            cerr << "Incomplete frequency table.\n";
            return false;
        }

        if (frequencies[i] >
            numeric_limits<uint64_t>::max() - frequencySum) {
            cerr << "Frequency table overflow.\n";
            return false;
        }

        frequencySum += frequencies[i];
    }

    if (frequencySum != originalSize) {
        cerr << "Frequency table does not match original size.\n";
        return false;
    }

    ofstream output(outputFile, ios::binary);
    if (!output) {
        cerr << "Cannot create reconstructed PCM file.\n";
        return false;
    }

    if (originalSize == 0) {
        output.close();
        cout << "Empty PCM file reconstructed.\n";
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

    if (minHeap.empty()) {
        cerr << "Invalid Huffman tree.\n";
        return false;
    }

    auto root = minHeap.top();

    if (root->isLeaf()) {
        for (uint64_t i = 0; i < originalSize; ++i) {
            output.put(static_cast<char>(root->symbol));
            if (!output) {
                cerr << "Error writing reconstructed PCM.\n";
                return false;
            }
        }

        output.close();
        cout << "Reconstructed " << originalSize << " bytes.\n";
        return true;
    }

    uint64_t decodedCount = 0;
    auto current = root;
    char ch;

    while (decodedCount < originalSize && input.get(ch)) {
        uint8_t byte = static_cast<uint8_t>(
            static_cast<unsigned char>(ch));

        for (int bitIndex = 7;
             bitIndex >= 0 && decodedCount < originalSize;
             --bitIndex) {

            bool bit = (byte >> bitIndex) & 1;
            current = bit ? current->right : current->left;

            if (!current) {
                cerr << "Invalid Huffman bitstream.\n";
                return false;
            }

            if (current->isLeaf()) {
                output.put(static_cast<char>(current->symbol));

                if (!output) {
                    cerr << "Error writing reconstructed PCM.\n";
                    return false;
                }

                ++decodedCount;
                current = root;
            }
        }
    }

    if (decodedCount != originalSize) {
        cerr << "Compressed file ended before decoding "
             << "the expected number of bytes.\n";
        return false;
    }

    output.close();
    input.close();

    cout << "Huffman decompression completed.\n";
    cout << "Reconstructed PCM size: "
         << decodedCount << " bytes\n";

    return true;
}

bool runFFmpeg(const string& pcmFile,
               const string& mp3File) {
    // The PCM format must match the compression pipeline.
    string command =
        "ffmpeg -nostdin -hide_banner -loglevel error -y "
        "-f s16le -ar 44100 -ac 2 -i \"" + pcmFile + "\" "
        "-c:a libmp3lame -b:a 192k \"" + mp3File + "\"";

    cout << "Converting reconstructed PCM to MP3...\n";
    return system(command.c_str()) == 0;
}

int main(int argc, char* argv[]) {
    string compressedFile =
        argc > 1 ? argv[1] : "compressed_audio.huf";
    string outputMp3 =
        argc > 2 ? argv[2] : "final_audio.mp3";

    string pcmFile = "audio_reconstructed_temp.pcm";

    if (!decompressHuffman(compressedFile, pcmFile))
        return 1;

    if (!runFFmpeg(pcmFile, outputMp3)) {
        cerr << "MP3 encoding failed. Check FFmpeg and its "
             << "MP3 encoder support.\n";
        return 1;
    }

    cout << "\nDecompression pipeline completed.\n";
    cout << "Final MP3: " << outputMp3 << '\n';

    return 0;
}
