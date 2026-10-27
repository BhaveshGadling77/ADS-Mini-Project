#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>

using namespace cv;
using namespace std;

// ==========================================================
// 1. Bit Packing Helper
// ==========================================================
vector<uint8_t> packBits(const string& bits) {
    vector<uint8_t> packed((bits.size() + 7) / 8, 0);
    for (size_t i = 0; i < bits.size(); ++i) {
        if (bits[i] == '1') {
            packed[i / 8] |= (1 << (7 - (i % 8)));
        }
    }
    return packed;
}

// ==========================================================
// 2. High-Fidelity Quadtree Engine
// ==========================================================
class QuadTreeCompressor {
private:
    int spatial_threshold;
    int temporal_threshold;

    void subdivide(const Mat& curr, const Mat& prev, string& bitstream, vector<uint8_t>& colors) {
        // Temporal Check: Only copy if block change is imperceptible
        if (!prev.empty()) {
            Mat diff;
            absdiff(curr, prev, diff);
            Scalar m_diff = mean(diff);
            
            if (m_diff[0] < temporal_threshold && 
                m_diff[1] < temporal_threshold && 
                m_diff[2] < temporal_threshold) {
                bitstream += "00"; // Code '00': Unchanged from previous frame
                return;
            }
        }

        // Spatial Check: Strict variance check for clean edges and detail
        Scalar m_curr, stddev_curr;
        meanStdDev(curr, m_curr, stddev_curr);

        bool needs_split = (curr.rows > 1 && curr.cols > 1) && 
                           (stddev_curr[0] > spatial_threshold || 
                            stddev_curr[1] > spatial_threshold || 
                            stddev_curr[2] > spatial_threshold);

        if (needs_split) {
            bitstream += "1"; // Code '1': Split block into 4 quadrants
            
            int half_w = curr.cols / 2;
            int half_h = curr.rows / 2;
            int rem_w = curr.cols - half_w;
            int rem_h = curr.rows - half_h;

            Rect q1(0, 0, half_w, half_h);
            Rect q2(half_w, 0, rem_w, half_h);
            Rect q3(0, half_h, half_w, rem_h);
            Rect q4(half_w, half_h, rem_w, rem_h);

            subdivide(curr(q1), prev.empty() ? Mat() : prev(q1), bitstream, colors);
            subdivide(curr(q2), prev.empty() ? Mat() : prev(q2), bitstream, colors);
            subdivide(curr(q3), prev.empty() ? Mat() : prev(q3), bitstream, colors);
            subdivide(curr(q4), prev.empty() ? Mat() : prev(q4), bitstream, colors);
        } else {
            bitstream += "01"; // Code '01': Leaf node, save color
            colors.push_back(static_cast<uint8_t>(m_curr[0])); // Blue
            colors.push_back(static_cast<uint8_t>(m_curr[1])); // Green
            colors.push_back(static_cast<uint8_t>(m_curr[2])); // Red
        }
    }

public:
    // Higher spatial thresholds reduce detail and usually produce smaller archives.
    QuadTreeCompressor(int s_thresh = 12, int t_thresh = 1) 
        : spatial_threshold(s_thresh), temporal_threshold(t_thresh) {}

    void processFrame(const Mat& curr, const Mat& prev, string& bitstream, vector<uint8_t>& colors) {
        bitstream = "";
        colors.clear();
        subdivide(curr, prev, bitstream, colors);
    }
};

// ==========================================================
// Compress raw YUV420 video
// ==========================================================
void compressYUVWorkflow(const string& input_path, const string& output_cmp, int orig_w, int orig_h, int compression_strength) {
    cout << "Raw YUV Video Detected." << endl;
    if (orig_w <= 0 || orig_h <= 0 || orig_w % 2 != 0 || orig_h % 2 != 0) {
        throw runtime_error("YUV420 dimensions must be positive even numbers.");
    }
    if (compression_strength < 3 || compression_strength > 40) {
        throw runtime_error("Compression strength must be between 3 and 40.");
    }

    ifstream infile(input_path, ios::binary);
    if (!infile) {
        throw runtime_error("Could not open YUV file: " + input_path);
    }

    ofstream outfile(output_cmp, ios::binary);
    if (!outfile) {
        throw runtime_error("Could not create output file: " + output_cmp);
    }

    // CMP Header
    outfile.write("CMP ", 4);
    uint8_t is_degraded = 0;
    outfile.write(reinterpret_cast<char*>(&is_degraded), sizeof(is_degraded));
    outfile.write(reinterpret_cast<char*>(&orig_w), sizeof(orig_w));
    outfile.write(reinterpret_cast<char*>(&orig_h), sizeof(orig_h));

    int frame_size = orig_w * orig_h * 3 / 2;
    vector<uint8_t> yuv_buffer(frame_size);

    QuadTreeCompressor qt(compression_strength);
    Mat prev_frame;
    int frame_count = 0;

    cout << "\nStarting Quadtree compression..." << endl;

    while (infile.read(reinterpret_cast<char*>(yuv_buffer.data()), frame_size)) {
        Mat raw_yuv(orig_h + orig_h / 2, orig_w, CV_8UC1, yuv_buffer.data());
        Mat current_frame;
        cvtColor(raw_yuv, current_frame, COLOR_YUV2BGR_I420);

        string bitstream;
        vector<uint8_t> colors;
        
        qt.processFrame(current_frame, prev_frame, bitstream, colors);

        vector<uint8_t> packed_bits = packBits(bitstream);
        uint32_t bit_count = static_cast<uint32_t>(bitstream.size());
        uint32_t packed_size = static_cast<uint32_t>(packed_bits.size());
        uint32_t colors_size = static_cast<uint32_t>(colors.size());

        outfile.write(reinterpret_cast<char*>(&bit_count), sizeof(bit_count));
        outfile.write(reinterpret_cast<char*>(&packed_size), sizeof(packed_size));
        outfile.write(reinterpret_cast<char*>(&colors_size), sizeof(colors_size));

        outfile.write(reinterpret_cast<char*>(packed_bits.data()), packed_size);
        outfile.write(reinterpret_cast<char*>(colors.data()), colors_size);

        prev_frame = current_frame.clone();
        frame_count++;
        
        if (frame_count % 10 == 0) cout << "Compressed " << frame_count << " frames..." << endl;
    }
    cout << "Compression complete. Saved to: " << output_cmp << endl;
}

// ==========================================================
// 5. Main Execution Router
// ==========================================================
int main(int argc, char** argv) {
    if (argc != 6) {
        cerr << "Usage: " << argv[0] << " <input.yuv> <output.cmp> <width> <height> <compression-strength>" << endl;
        return 2;
    }

    try {
        compressYUVWorkflow(argv[1], argv[2], stoi(argv[3]), stoi(argv[4]), stoi(argv[5]));
    } catch (const exception& e) {
        cerr << "Video compression failed: " << e.what() << endl;
        return 1;
    }
    return 0;
}
