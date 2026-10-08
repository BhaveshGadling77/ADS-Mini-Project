#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>

using namespace cv;
using namespace std;

// ==========================================================
// 1. Bit Unpacking Helper
// ==========================================================
string unpackBits(const vector<uint8_t>& packed, int bit_count) {
    string bits = "";
    for (int i = 0; i < bit_count; ++i) {
        if (packed[i / 8] & (1 << (7 - (i % 8)))) bits += '1';
        else bits += '0';
    }
    return bits;
}

// ==========================================================
// 2. Quadtree Decoder
// ==========================================================
class QuadTreeDecoder {
private:
    int bit_index;
    int color_index;

    void rebuild(Mat& curr, const Mat& prev, const string& bitstream, const vector<uint8_t>& colors) {
        if (bit_index >= bitstream.length()) return;

        char bit = bitstream[bit_index++];

        if (bit == '1') {
            int half_w = curr.cols / 2;
            int half_h = curr.rows / 2;
            if (half_w == 0 || half_h == 0) return;

            int rem_w = curr.cols - half_w;
            int rem_h = curr.rows - half_h;

            Rect q1(0, 0, half_w, half_h);
            Rect q2(half_w, 0, rem_w, half_h);
            Rect q3(0, half_h, half_w, rem_h);
            Rect q4(half_w, half_h, rem_w, rem_h);

            rebuild(curr(q1), prev.empty() ? Mat() : prev(q1), bitstream, colors);
            rebuild(curr(q2), prev.empty() ? Mat() : prev(q2), bitstream, colors);
            rebuild(curr(q3), prev.empty() ? Mat() : prev(q3), bitstream, colors);
            rebuild(curr(q4), prev.empty() ? Mat() : prev(q4), bitstream, colors);
        } else {
            if (bit_index >= bitstream.length()) return;
            char leaf_type = bitstream[bit_index++];

            if (leaf_type == '0') {
                if (!prev.empty()) {
                    prev.copyTo(curr);
                }
            } else if (leaf_type == '1') {
                uint8_t b = colors[color_index++];
                uint8_t g = colors[color_index++];
                uint8_t r = colors[color_index++];
                curr.setTo(Scalar(b, g, r));
            }
        }
    }

public:
    void decodeFrame(const string& bitstream, const vector<uint8_t>& colors, Mat& curr_frame, const Mat& prev_frame) {
        bit_index = 0;
        color_index = 0;
        rebuild(curr_frame, prev_frame, bitstream, colors);
    }
};

// ==========================================================
// 3. Decompression Pipeline (.cmp to .yuv)
// ==========================================================
class VideoDecompressor {
private:
    QuadTreeDecoder qtDecoder;

public:
    void decompress(const string& input_cmp, const string& output_yuv) {
        ifstream infile(input_cmp, ios::binary);
        if (!infile) {
            cerr << "Error: Could not open compressed file." << endl;
            return;
        }

        ofstream outfile(output_yuv, ios::binary);
        if (!outfile) {
            cerr << "Error: Could not create output YUV file." << endl;
            return;
        }

        char signature[5] = {0};
        infile.read(signature, 4);
        if (string(signature) != "CMP ") {
            cerr << "Error: Invalid file format. Not a recognized .cmp file." << endl;
            return;
        }
        
        uint8_t is_degraded;
        int target_w, target_h;

        infile.read(reinterpret_cast<char*>(&is_degraded), sizeof(is_degraded));
        infile.read(reinterpret_cast<char*>(&target_w), sizeof(target_w));
        infile.read(reinterpret_cast<char*>(&target_h), sizeof(target_h));

        cout << "Detected .cmp resolution: " << target_w << "x" << target_h << endl;
        cout << "Reconstructing raw YUV data..." << endl;

        int yuv_frame_size = target_w * target_h * 3 / 2;
        Mat prev_frame;
        int frame_count = 0;

        while (infile.peek() != EOF) {
            uint32_t bit_count, packed_size, colors_size;
            infile.read(reinterpret_cast<char*>(&bit_count), sizeof(bit_count));
            if (infile.eof()) break;
            infile.read(reinterpret_cast<char*>(&packed_size), sizeof(packed_size));
            infile.read(reinterpret_cast<char*>(&colors_size), sizeof(colors_size));

            vector<uint8_t> packed_bits(packed_size);
            infile.read(reinterpret_cast<char*>(packed_bits.data()), packed_size);

            vector<uint8_t> colors(colors_size);
            infile.read(reinterpret_cast<char*>(colors.data()), colors_size);

            string bitstream = unpackBits(packed_bits, bit_count);

            Mat current_frame = Mat::zeros(target_h, target_w, CV_8UC3);
            qtDecoder.decodeFrame(bitstream, colors, current_frame, prev_frame);

            // Convert to YUV420p
            Mat yuv_frame;
            cvtColor(current_frame, yuv_frame, COLOR_BGR2YUV_I420);

            // Safe stride write: prevents horizontal tearing & padding bugs
            if (yuv_frame.isContinuous()) {
                outfile.write(reinterpret_cast<char*>(yuv_frame.data), yuv_frame_size);
            } else {
                for (int r = 0; r < yuv_frame.rows; ++r) {
                    outfile.write(reinterpret_cast<char*>(yuv_frame.ptr(r)), yuv_frame.cols);
                }
            }

            prev_frame = current_frame.clone();
            frame_count++;
            
            if (frame_count % 10 == 0) cout << "Restored " << frame_count << " YUV frames..." << endl;
        }
        cout << "Restored " << frame_count << " frames." << endl;
        cout << "Output saved to: " << output_yuv << endl;
    }
};

// ==========================================================
// 4. Execution Module
// ==========================================================
int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input_file.cmp>" << endl;
        return -1;
    }
    
    string input_cmp = argv[1];
    string output_yuv = input_cmp.substr(0, input_cmp.find_last_of(".")) + "_restored.yuv";

    VideoDecompressor decompressor;
    decompressor.decompress(input_cmp, output_yuv);
    
    return 0;
}