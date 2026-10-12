# Bitshrink — Teammate Implementation Guide (Image, Audio, Video)

Welcome! **Bitshrink** is structured so that each team member can implement their assigned compression algorithms independently in C++ without having to write backend or frontend code.

The Fastify backend and React frontend are already connected to the C++ engine. Once you implement your C++ functions, your algorithms will immediately work in the web UI and REST API!

---

## 1. Team Assignment Summary

| Contributor | Modality | Algorithms | Core Data Structures | Status |
|---|---|---|---|---|
| **Bhavesh** | **Text** | LZW, Static Huffman, BWT + MTF | Trie, Min-Heap, Binary Tree, Suffix Array | **Completed** |
| **Jibran** | **Image** | Spatial Quadtree, Color Quantization, 2D Haar Wavelets | Quadtree, KD-Tree | *Awaiting implementation* |
| **Parth** | **Audio** | 1D Wavelet Tree, Delta + RLE, LPC + Huffman | Wavelet Tree, Min-Heap | *Awaiting implementation* |
| **Sangram** | **Video** | Inter-frame Quadtree, 3D Octree | Quadtree, 3D Octree | *Awaiting implementation* |

---

## 2. Common Interface Contract

All compression modules inherit from `adsc::ICompressor` (defined in `compression-core/include/core/ICompressor.hpp`):

```cpp
class ICompressor {
public:
    virtual ~ICompressor() = default;
    virtual Algorithm getAlgorithm() const = 0;
    virtual std::string getName() const = 0;

    // 1. COMPRESS: Reads raw uncompressed bytes from `in`, writes header + compressed payload to `out`
    virtual CompressionMetrics compress(std::istream& in, std::ostream& out) = 0;

    // 2. DECOMPRESS: Reads compressed payload from `in`, restores exact uncompressed bytes to `out`
    virtual void decompress(std::istream& in, std::ostream& out, uint64_t originalSize) = 0;
};
```

---

## 3. Guide for Jibran (Image Compression)

### Files to Edit
1. **Header:** [`compression-core/include/algorithms/image/ImageCompressor.hpp`](file:///home/bhavesh/ADS-Mini-Project/compression-core/include/algorithms/image/ImageCompressor.hpp)
2. **Implementation:** [`compression-core/src/algorithms/ImageCompressor.cpp`](file:///home/bhavesh/ADS-Mini-Project/compression-core/src/algorithms/ImageCompressor.cpp)
3. **Data Structures (if needed):**
   - Quadtree header: `compression-core/include/datastructures/quadtree/Quadtree.hpp`
   - KD-Tree header: `compression-core/include/datastructures/kdtree/KDTree.hpp`

### Function Signatures to Implement
In `compression-core/src/algorithms/ImageCompressor.cpp`:

```cpp
CompressionMetrics ImageCompressor::compress(std::istream& in, std::ostream& out) {
    auto startTime = std::chrono::high_resolution_clock::now();

    // 1. Read input data
    std::vector<uint8_t> inputData(
        (std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>()
    );
    uint64_t originalSize = inputData.size();

    // 2. Write standard 16-byte file header
    Header header;
    header.algorithm = static_cast<uint8_t>(algorithm);
    header.modality = static_cast<uint8_t>(Modality::IMAGE);
    header.originalSize = originalSize;
    header.write(out);

    // 3. Algorithm logic depending on this->algorithm:
    // - If Algorithm::IMAGE_QUADTREE:
    //     Build Spatial Quadtree decomposing image regions with variance threshold
    // - If Algorithm::IMAGE_QUANTIZATION:
    //     Build KD-Tree to cluster RGB color palette into 16/256 centroids
    // - If Algorithm::IMAGE_WAVELET:
    //     Compute 2D Haar Wavelet transform matrix (LL, LH, HL, HH subbands)
    
    // Write encoded payload to `out`...

    auto endTime = std::chrono::high_resolution_clock::now();
    double durationMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    // 4. Return metrics
    CompressionMetrics metrics;
    metrics.algorithm = algorithmToString(algorithm);
    metrics.modality = "image";
    metrics.originalSizeBytes = originalSize;
    metrics.compressedSizeBytes = out.tellp();
    metrics.compressionRatio = (metrics.compressedSizeBytes > 0)
        ? (double)originalSize / metrics.compressedSizeBytes : 1.0;
    metrics.spaceSavedPercent = (1.0 - (double)metrics.compressedSizeBytes / originalSize) * 100.0;
    metrics.executionTimeMs = durationMs;
    metrics.success = true;
    return metrics;
}

void ImageCompressor::decompress(std::istream& in, std::ostream& out, uint64_t originalSize) {
    // 1. Read encoded payload from `in`
    // 2. Invert Quadtree / KD-Tree / Haar Wavelet transform
    // 3. Write restored image bytes to `out`
}
```

### CLI Testing for Jibran
```bash
cmake --build compression-core/build
./compression-core/build/bin/compression-engine compress -m image -a quadtree -i input.bmp -o output.adsc --json
./compression-core/build/bin/compression-engine decompress -i output.adsc -o restored.bmp --json
```

---

## 4. Guide for Parth (Audio Compression)

### Files to Edit
1. **Header:** [`compression-core/include/algorithms/audio/AudioCompressor.hpp`](file:///home/bhavesh/ADS-Mini-Project/compression-core/include/algorithms/audio/AudioCompressor.hpp)
2. **Implementation:** [`compression-core/src/algorithms/AudioCompressor.cpp`](file:///home/bhavesh/ADS-Mini-Project/compression-core/src/algorithms/AudioCompressor.cpp)
3. **Data Structures:**
   - Wavelet Tree: `compression-core/include/datastructures/tree/WaveletTree.hpp`

### Function Signatures to Implement
In `compression-core/src/algorithms/AudioCompressor.cpp`:

```cpp
CompressionMetrics AudioCompressor::compress(std::istream& in, std::ostream& out) {
    // 1. Read audio samples (e.g. 16-bit PCM samples from WAV header)
    // 2. Write 16-byte Header (header.modality = Modality::AUDIO)
    // 3. Perform algorithm:
    //    - If Algorithm::AUDIO_DELTA_RLE:
    //        Compute adjacent sample differences: delta[i] = sample[i] - sample[i-1]
    //        Apply Run-Length Encoding on zero/low deltas
    //    - If Algorithm::AUDIO_WAVELET:
    //        Build 1D Haar Wavelet Tree decomposition
    //    - If Algorithm::AUDIO_LPC:
    //        Compute Linear Predictive Coding reflection coefficients + Huffman residual
    // 4. Return CompressionMetrics
}

void AudioCompressor::decompress(std::istream& in, std::ostream& out, uint64_t originalSize) {
    // Invert audio transform and restore original PCM samples
}
```

### CLI Testing for Parth
```bash
cmake --build compression-core/build
./compression-core/build/bin/compression-engine compress -m audio -a delta_rle -i sample.wav -o sample.adsc --json
./compression-core/build/bin/compression-engine decompress -i sample.adsc -o restored.wav --json
```

---

## 5. Guide for Sangram (Video Compression)

### Files to Edit
1. **Header:** [`compression-core/include/algorithms/video/VideoCompressor.hpp`](file:///home/bhavesh/ADS-Mini-Project/compression-core/include/algorithms/video/VideoCompressor.hpp)
2. **Implementation:** [`compression-core/src/algorithms/VideoCompressor.cpp`](file:///home/bhavesh/ADS-Mini-Project/compression-core/src/algorithms/VideoCompressor.cpp)
3. **Data Structures:**
   - 3D Octree: `compression-core/include/datastructures/octree/Octree.hpp`
   - Inter-frame Quadtree: `compression-core/include/datastructures/quadtree/Quadtree.hpp`

### Function Signatures to Implement
In `compression-core/src/algorithms/VideoCompressor.cpp`:

```cpp
CompressionMetrics VideoCompressor::compress(std::istream& in, std::ostream& out) {
    // 1. Read sequential video frames
    // 2. Write 16-byte Header (header.modality = Modality::VIDEO)
    // 3. Perform algorithm:
    //    - If Algorithm::VIDEO_INTERFRAME:
    //        Compute difference between Frame[t] and Frame[t-1]
    //        Construct Quadtree over changed macroblocks; encode only active blocks
    //    - If Algorithm::VIDEO_OCTREE:
    //        Group (X, Y, Time) spatio-temporal volume into a 3D Octree
    // 4. Return CompressionMetrics
}

void VideoCompressor::decompress(std::istream& in, std::ostream& out, uint64_t originalSize) {
    // Reconstruct frame sequence from keyframes + interframe delta quadtrees
}
```

### CLI Testing for Sangram
```bash
cmake --build compression-core/build
./compression-core/build/bin/compression-engine compress -m video -a interframe -i video.raw -o video.adsc --json
./compression-core/build/bin/compression-engine decompress -i video.adsc -o restored.raw --json
```

---

## 6. How it Integrates with Server and Web Frontend

- **Server:**
  The server routes `/api/compress` and `/api/decompress` automatically pass the `modality` parameter (`text`, `image`, `audio`, `video`) to the C++ CLI executable (`-m <modality>`).
- **Web Frontend:**
  The frontend UI provides a modality switcher (Text, Image, Audio, Video) and lists each person's algorithms with appropriate file dropzone format filters.
- **When You Finish:**
  Once you write your `compress()` and `decompress()` logic in your respective `.cpp` file, run:
  ```bash
  ./scripts/build.sh
  ./scripts/test.sh
  ./scripts/start.sh
  ```
  And test your algorithm directly in the web browser at `http://localhost:3000`!
