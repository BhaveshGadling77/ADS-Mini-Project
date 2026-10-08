#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>

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
    // Spatial = 3 preserves edges/gradients; Temporal = 1 prevents frame flicker
    QuadTreeCompressor(int s_thresh = 3, int t_thresh = 1) 
        : spatial_threshold(s_thresh), temporal_threshold(t_thresh) {}

    void processFrame(const Mat& curr, const Mat& prev, string& bitstream, vector<uint8_t>& colors) {
        bitstream = "";
        colors.clear();
        subdivide(curr, prev, bitstream, colors);
    }
};

// ==========================================================
// 3. Workflow 1: Degrade MP4 Video
// ==========================================================
void degradeMP4Workflow(const string& input_path, const string& base_name) {
    VideoCapture cap(input_path);
    if (!cap.isOpened()) {
        cerr << "Error: Could not open video file." << endl;
        return;
    }

    int orig_w = static_cast<int>(cap.get(CAP_PROP_FRAME_WIDTH));
    int orig_h = static_cast<int>(cap.get(CAP_PROP_FRAME_HEIGHT));
    double fps = cap.get(CAP_PROP_FPS);
    if (fps <= 0.0) fps = 30.0;

    cout << "Standard Video Detected. Original Resolution: " << orig_w << "x" << orig_h << endl;
    cout << "\nChoose degradation target resolution:" << endl;
    cout << "1. 1080p (1920x1080)" << endl;
    cout << "2. 720p  (1280x720)" << endl;
    cout << "3. 480p  (854x480)" << endl;
    cout << "4. 360p  (640x360)" << endl;
    cout << "5. 240p  (426x240)" << endl;
    cout << "6. 144p  (256x144)" << endl;
    cout << "Enter choice (1-6): ";
    
    int res_choice;
    cin >> res_choice;

    int target_w, target_h;
    string res_label = "";

    switch (res_choice) {
        case 1: target_w = 1920; target_h = 1080; res_label = "1080p"; break;
        case 2: target_w = 1280; target_h = 720;  res_label = "720p"; break;
        case 3: target_w = 854;  target_h = 480;  res_label = "480p"; break;
        case 4: target_w = 640;  target_h = 360;  res_label = "360p"; break;
        case 5: target_w = 426;  target_h = 240;  res_label = "240p"; break;
        case 6: target_w = 256;  target_h = 144;  res_label = "144p"; break;
        default:
            cerr << "Error: Invalid choice. Exiting." << endl;
            return;
    }

    string output_mp4 = base_name + "_" + res_label + ".mp4";
    cout << "\nDegrading video to " << target_w << "x" << target_h << "..." << endl;

    VideoWriter writer(output_mp4, VideoWriter::fourcc('m', 'p', '4', 'v'), fps, Size(target_w, target_h), true);
    Mat frame, resized_frame;
    int frame_count = 0;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        resize(frame, resized_frame, Size(target_w, target_h), 0, 0, INTER_LINEAR);
        writer.write(resized_frame);

        frame_count++;
        if (frame_count % 30 == 0) cout << "Processed " << frame_count << " frames..." << endl;
    }
    cout << "Degradation complete. Saved to: " << output_mp4 << endl;
}

// ==========================================================
// 4. Workflow 2: Compress Raw YUV
// ==========================================================
void compressYUVWorkflow(const string& input_path, const string& base_name) {
    cout << "Raw YUV Video Detected." << endl;
    int orig_w, orig_h;
    cout << "Enter original width: ";
    cin >> orig_w;
    cout << "Enter original height: ";
    cin >> orig_h;

    ifstream infile(input_path, ios::binary);
    if (!infile) {
        cerr << "Error: Could not open YUV file." << endl;
        return;
    }

    string output_cmp = base_name + ".cmp";
    ofstream outfile(output_cmp, ios::binary);
    if (!outfile) {
        cerr << "Error: Could not create output file." << endl;
        return;
    }

    // CMP Header
    outfile.write("CMP ", 4);
    uint8_t is_degraded = 0;
    outfile.write(reinterpret_cast<char*>(&is_degraded), sizeof(is_degraded));
    outfile.write(reinterpret_cast<char*>(&orig_w), sizeof(orig_w));
    outfile.write(reinterpret_cast<char*>(&orig_h), sizeof(orig_h));

    int frame_size = orig_w * orig_h * 3 / 2;
    vector<uint8_t> yuv_buffer(frame_size);

    QuadTreeCompressor qt;
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
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input_video_file>" << endl;
        return -1;
    }
    
    string input_video = argv[1];
    string base_name = input_video.substr(0, input_video.find_last_of("."));
    string ext = input_video.substr(input_video.find_last_of(".") + 1);
    transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == "yuv") {
        compressYUVWorkflow(input_video, base_name);
    } else {
        degradeMP4Workflow(input_video, base_name);
    }
    
    return 0;
}