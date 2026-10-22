// tests/test_spatial_predictor.cpp
// Dedicated unit test suite for left-neighbor spatial predictor and inverse predictor.

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <random>
#include <sstream>

#include "../compression/spatial_predictor.h"

using namespace std;

static int g_passed = 0;
static int g_failed = 0;
static int g_total = 0;

#define TEST(name)                                          \
    do                                                      \
    {                                                       \
        g_total++;                                          \
        cout << "TEST " << g_total << ": " << name << " "; \
        cout.flush();                                       \
    } while (0)

#define PASS()                     \
    do                             \
    {                              \
        g_passed++;                \
        cout << "[PASS]" << endl;  \
    } while (0)

#define FAIL(msg)                                 \
    do                                            \
    {                                             \
        g_failed++;                               \
        cout << "[FAIL] " << msg << endl;         \
    } while (0)

// Helper: Assert two byte vectors are identical
bool assertEqual(const vector<unsigned char> &actual,
                 const vector<unsigned char> &expected,
                 string &errMsg)
{
    if (actual.size() != expected.size())
    {
        errMsg = "Size mismatch: expected " + to_string(expected.size()) +
                 ", got " + to_string(actual.size());
        return false;
    }
    for (size_t i = 0; i < actual.size(); ++i)
    {
        if (actual[i] != expected[i])
        {
            ostringstream oss;
            oss << "Mismatch at byte index " << i << " (pixel " << (i / 3)
                << ", channel " << (i % 3) << "): expected "
                << static_cast<int>(expected[i]) << ", got "
                << static_cast<int>(actual[i]);
            errMsg = oss.str();
            return false;
        }
    }
    return true;
}

// ----------------------------------------------------------------------------
// Test 1: Row boundary reset
// Prediction must reset at the start of each row.
// Pixel (0, y) must not subtract from pixel (width-1, y-1).
// ----------------------------------------------------------------------------
void test_row_boundary_reset()
{
    TEST("Row boundary reset: pixel (0, y) does not subtract from previous row");

    // 2x2 image (6 bytes total: 2 pixels per row, 2 rows)
    // Row 0: Pixel(10, 20, 30), Pixel(15, 25, 35)
    // Row 1: Pixel(40, 50, 60), Pixel(45, 55, 65)
    vector<unsigned char> original = {
        10, 20, 30,   15, 25, 35, // row 0
        40, 50, 60,   45, 55, 65  // row 1
    };

    // Expected forward residuals:
    // Row 0:
    //   Pixel 0: 10, 20, 30 (raw)
    //   Pixel 1: (15-10)=5, (25-20)=5, (35-30)=5
    // Row 1:
    //   Pixel 0: 40, 50, 60 (raw reset! NOT 40-15=25)
    //   Pixel 1: (45-40)=5, (55-50)=5, (65-60)=5
    vector<unsigned char> expectedResiduals = {
        10, 20, 30,   5, 5, 5,
        40, 50, 60,   5, 5, 5
    };

    vector<unsigned char> data = original;
    applySpatialPrediction(data, 2, 2);

    string err;
    if (!assertEqual(data, expectedResiduals, err))
    {
        FAIL("Forward prediction: " + err);
        return;
    }

    applyInverseSpatialPrediction(data, 2, 2);
    if (!assertEqual(data, original, err))
    {
        FAIL("Inverse prediction: " + err);
        return;
    }

    PASS();
}

// ----------------------------------------------------------------------------
// Test 2: Identical pixels (Uniform regions)
// In a uniform color image, only the very first pixel retains raw values;
// all subsequent pixels in each row must become (0, 0, 0).
// ----------------------------------------------------------------------------
void test_identical_pixels()
{
    TEST("Identical pixels: uniform region yields zeroes for subsequent pixels");

    int width = 4, height = 3;
    vector<unsigned char> original(width * height * 3);
    for (size_t i = 0; i < original.size(); i += 3)
    {
        original[i]     = 128; // R
        original[i + 1] = 64;  // G
        original[i + 2] = 200; // B
    }

    vector<unsigned char> expectedResiduals(width * height * 3, 0);
    for (int y = 0; y < height; ++y)
    {
        size_t rowStart = static_cast<size_t>(y) * width * 3;
        // First pixel of each row has raw values
        expectedResiduals[rowStart]     = 128;
        expectedResiduals[rowStart + 1] = 64;
        expectedResiduals[rowStart + 2] = 200;
        // Remaining pixels in this row stay 0
    }

    vector<unsigned char> data = original;
    applySpatialPrediction(data, width, height);

    string err;
    if (!assertEqual(data, expectedResiduals, err))
    {
        FAIL("Forward prediction: " + err);
        return;
    }

    applyInverseSpatialPrediction(data, width, height);
    if (!assertEqual(data, original, err))
    {
        FAIL("Inverse prediction: " + err);
        return;
    }

    PASS();
}

// ----------------------------------------------------------------------------
// Test 3: Channel differences (Independent R, G, B behavior)
// Channels should vary at completely different rates without cross-channel bleeding.
// ----------------------------------------------------------------------------
void test_channel_differences()
{
    TEST("Channel independence: R, G, B predicted separately without crosstalk");

    // 3x1 image (single row, 3 pixels)
    // Pixel 0: (100, 200, 50)
    // Pixel 1: (120, 180, 50) -> diffs: R +20, G -20 (wraps to 236), B 0
    // Pixel 2: (150, 150, 50) -> diffs: R +30, G -30 (wraps to 226), B 0
    vector<unsigned char> original = {
        100, 200, 50,
        120, 180, 50,
        150, 150, 50
    };

    vector<unsigned char> expectedResiduals = {
        100, 200, 50,
        20, static_cast<unsigned char>(-20), 0,
        30, static_cast<unsigned char>(-30), 0
    };

    vector<unsigned char> data = original;
    applySpatialPrediction(data, 3, 1);

    string err;
    if (!assertEqual(data, expectedResiduals, err))
    {
        FAIL("Forward prediction: " + err);
        return;
    }

    applyInverseSpatialPrediction(data, 3, 1);
    if (!assertEqual(data, original, err))
    {
        FAIL("Inverse prediction: " + err);
        return;
    }

    PASS();
}

// ----------------------------------------------------------------------------
// Test 4: Byte wraparound (modulo 256)
// Test edge underflow (5 - 20 = 241) and overflow (250 - 240 = 10, 0 - 255 = 1).
// ----------------------------------------------------------------------------
void test_byte_wraparound()
{
    TEST("Byte wraparound: underflow and overflow wrap modulo 256 correctly");

    // 4x1 image:
    // Pixel 0: (20,  255, 0)
    // Pixel 1: (5,   0,   255) -> diff: 5-20=-15(241), 0-255=-255(1), 255-0=255
    // Pixel 2: (250, 10,  10)  -> diff: 250-5=245, 10-0=10, 10-255=-245(11)
    // Pixel 3: (0,   0,   0)   -> diff: 0-250=-250(6), 0-10=-10(246), 0-10=-10(246)
    vector<unsigned char> original = {
        20,  255, 0,
        5,   0,   255,
        250, 10,  10,
        0,   0,   0
    };

    vector<unsigned char> expectedResiduals = {
        20,  255, 0,
        static_cast<unsigned char>(5 - 20),
        static_cast<unsigned char>(0 - 255),
        static_cast<unsigned char>(255 - 0),

        static_cast<unsigned char>(250 - 5),
        static_cast<unsigned char>(10 - 0),
        static_cast<unsigned char>(10 - 255),

        static_cast<unsigned char>(0 - 250),
        static_cast<unsigned char>(0 - 10),
        static_cast<unsigned char>(0 - 10)
    };

    vector<unsigned char> data = original;
    applySpatialPrediction(data, 4, 1);

    string err;
    if (!assertEqual(data, expectedResiduals, err))
    {
        FAIL("Forward prediction: " + err);
        return;
    }

    applyInverseSpatialPrediction(data, 4, 1);
    if (!assertEqual(data, original, err))
    {
        FAIL("Inverse prediction: " + err);
        return;
    }

    PASS();
}

// ----------------------------------------------------------------------------
// Test 5: Dimension edge cases (1x1, 1xN, Nx1, 0x0)
// ----------------------------------------------------------------------------
void test_dimension_edge_cases()
{
    TEST("Dimension edge cases: 1x1, 1xN, empty, and invalid sizes");

    string err;

    // 1x1 single pixel
    {
        vector<unsigned char> orig = {42, 84, 126};
        vector<unsigned char> data = orig;
        applySpatialPrediction(data, 1, 1);
        if (!assertEqual(data, orig, err)) { FAIL("1x1 forward altered data"); return; }
        applyInverseSpatialPrediction(data, 1, 1);
        if (!assertEqual(data, orig, err)) { FAIL("1x1 inverse altered data"); return; }
    }

    // 1x3 vertical column (every pixel is the first in its row -> all residuals raw)
    {
        vector<unsigned char> orig = {
            10, 20, 30,
            40, 50, 60,
            70, 80, 90
        };
        vector<unsigned char> data = orig;
        applySpatialPrediction(data, 1, 3);
        if (!assertEqual(data, orig, err)) { FAIL("1x3 forward altered data"); return; }
        applyInverseSpatialPrediction(data, 1, 3);
        if (!assertEqual(data, orig, err)) { FAIL("1x3 inverse altered data"); return; }
    }

    // 0x0 empty data
    {
        vector<unsigned char> emptyData;
        try {
            applySpatialPrediction(emptyData, 0, 0);
            applyInverseSpatialPrediction(emptyData, 0, 0);
        } catch (...) {
            FAIL("0x0 should not throw for empty data");
            return;
        }
    }

    // Invalid dimensions (< 0)
    {
        vector<unsigned char> dummy = {1, 2, 3};
        bool threw = false;
        try { applySpatialPrediction(dummy, -1, 1); }
        catch (const invalid_argument &) { threw = true; }
        if (!threw) { FAIL("Did not throw for negative width"); return; }
    }

    // Size mismatch
    {
        vector<unsigned char> badSize = {1, 2, 3, 4}; // not a multiple of 3, or doesn't match 2x2*3=12
        bool threw = false;
        try { applySpatialPrediction(badSize, 2, 2); }
        catch (const invalid_argument &) { threw = true; }
        if (!threw) { FAIL("Did not throw for size mismatch"); return; }
    }

    PASS();
}

// ----------------------------------------------------------------------------
// Test 6: Round-trip exact reconstruction property
// Test arbitrary/random data on various non-power-of-two dimensions.
// ----------------------------------------------------------------------------
void test_roundtrip_arbitrary_dimensions()
{
    TEST("Round-trip exact reconstruction: arbitrary dimensions and random data");

    mt19937 rng(1337);
    uniform_int_distribution<int> dist(0, 255);

    vector<pair<int, int>> dimensions = {
        {7, 5},
        {13, 11},
        {17, 1},
        {1, 19},
        {64, 48},
        {37, 23}
    };

    for (const auto &dim : dimensions)
    {
        int w = dim.first;
        int h = dim.second;
        size_t totalBytes = static_cast<size_t>(w) * h * 3;
        vector<unsigned char> orig(totalBytes);
        for (size_t i = 0; i < totalBytes; ++i)
        {
            orig[i] = static_cast<unsigned char>(dist(rng));
        }

        vector<unsigned char> data = orig;
        applySpatialPrediction(data, w, h);
        applyInverseSpatialPrediction(data, w, h);

        string err;
        if (!assertEqual(data, orig, err))
        {
            FAIL("Dimension " + to_string(w) + "x" + to_string(h) + ": " + err);
            return;
        }
    }

    PASS();
}

// ----------------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------------
int main()
{
    cout << "\n===============================================" << endl;
    cout << "  SPATIAL PREDICTOR UNIT TEST SUITE" << endl;
    cout << "===============================================" << endl;

    test_row_boundary_reset();
    test_identical_pixels();
    test_channel_differences();
    test_byte_wraparound();
    test_dimension_edge_cases();
    test_roundtrip_arbitrary_dimensions();

    cout << "\n===============================================" << endl;
    cout << "  RESULTS: " << g_passed << " passed, " << g_failed
         << " failed, " << g_total << " total" << endl;
    cout << "===============================================" << endl;

    return g_failed > 0 ? 1 : 0;
}
