#!/usr/bin/env bash

set -u
set -o pipefail

# Run from the project root
if [[ ! -x ./compressor ]]; then
    echo "Error: ./compressor not found. Run 'make' first."
    exit 1
fi

INPUT_DIR="input/images"
COMPRESSED_DIR="output/compressed"
DECOMPRESSED_DIR="output/decompressed"
RESULTS_DIR="results"
LOG_DIR="$RESULTS_DIR/logs"
CSV_FILE="$RESULTS_DIR/compression_report.csv"

mkdir -p "$COMPRESSED_DIR" "$DECOMPRESSED_DIR" "$LOG_DIR" "$RESULTS_DIR"

# Clear outputs from previous runs
shopt -s nullglob
rm -f "$COMPRESSED_DIR"/* "$DECOMPRESSED_DIR"/* "$LOG_DIR"/*
shopt -u nullglob

# CSV escaping for filenames
csv_quote() {
    local value=${1//\"/\"\"}
    printf '"%s"' "$value"
}

# Discover BMP files, including filenames with spaces
found=0
passed=0
failed=0

printf '"Image","Original Bytes","Compressed Bytes","Saved (%%)","Ratio","Status"\n' > "$CSV_FILE"

printf "\nIMAGE COMPRESSION BENCHMARK\n"
printf '%s\n' '-------------------------------------------------------------------------------'
printf '%-32s %12s %12s %10s %10s %10s\n' \
"IMAGE" "ORIGINAL" "COMPRESSED" "SAVED" "RATIO" "STATUS"
printf '%s\n' '-------------------------------------------------------------------------------'

while IFS= read -r -d '' input; do
    found=$((found + 1))
    
    filename=$(basename "$input")
    name="${filename%.*}"
    
    compressed="$COMPRESSED_DIR/$name.huf"
    decompressed="$DECOMPRESSED_DIR/${name}_decompressed.bmp"
    
    compress_log="$LOG_DIR/${name}_compress.log"
    decompress_log="$LOG_DIR/${name}_decompress.log"
    
    # Remove stale outputs so old files cannot be mistaken for success
    rm -f "$compressed" "$decompressed"
    
    status="PASS"
    original_size=$(wc -c < "$input" | tr -d ' ')
    
    # Compress using the existing stdin-driven menu
    if ! printf '1\n%s\n%s\n0\n' "$input" "$compressed" |
    ./compressor > "$compress_log" 2>&1 ||
    ! grep -q "Operation completed successfully." "$compress_log" ||
    [[ ! -f "$compressed" ]]; then
        status="COMPRESS_FAIL"
    fi
    
    compressed_size=0
    
    if [[ "$status" == "PASS" ]]; then
        compressed_size=$(wc -c < "$compressed" | tr -d ' ')
        
        # Decompress using the existing stdin-driven menu
        if ! printf '2\n%s\n%s\n0\n' "$compressed" "$decompressed" |
        ./compressor > "$decompress_log" 2>&1 ||
        ! grep -q "Operation completed successfully." "$decompress_log" ||
        [[ ! -f "$decompressed" ]]; then
            status="DECOMPRESS_FAIL"
        fi
    fi
    
    if [[ "$status" == "PASS" ]]; then
        reconstructed_size=$(wc -c < "$decompressed" | tr -d ' ')
        
        # Verify exact pixel equality using our own BMP reader
        if ./compare_images "$input" "$decompressed" \
        > "$LOG_DIR/${name}_verify.log" 2>&1; then
            status="PASS"
        else
            status="PIXEL_CHECK_FAIL"
        fi
    fi
    
    if [[ "$status" == "PASS" || "$status" == "DECOMPRESS_OK" ]]; then
        passed=$((passed + 1))
    else
        failed=$((failed + 1))
    fi
    
    if (( compressed_size > 0 )); then
        read -r saved ratio < <(
            awk -v original="$original_size" -v compressed="$compressed_size" \
            'BEGIN {
                    printf "%.2f %.3f\n",
                    (1 - compressed / original) * 100,
                    original / compressed
            }'
        )
    else
        saved="N/A"
        ratio="N/A"
    fi
    
    printf '%-32.32s %12s %12s %9s%% %10s %10s\n' \
    "$filename" "$original_size" "$compressed_size" "$saved" "$ratio" "$status"
    
    {
        csv_quote "$filename"; printf ','
        printf '%s,%s,' "$original_size" "$compressed_size"
        csv_quote "$saved"; printf ','
        csv_quote "$ratio"; printf ','
        csv_quote "$status"; printf '\n'
    } >> "$CSV_FILE"
    
done < <(find "$INPUT_DIR" -maxdepth 1 -type f -iname '*.bmp' -print0 | sort -z)

printf '%s\n' '-------------------------------------------------------------------------------'
printf 'Images found: %d | Passed: %d | Failed: %d\n' "$found" "$passed" "$failed"
printf 'CSV report: %s\n' "$CSV_FILE"
printf 'Detailed logs: %s\n\n' "$LOG_DIR"

if (( found == 0 )); then
    echo "No BMP images found in $INPUT_DIR"
    exit 1
fi

if (( failed > 0 )); then
    exit 1
fi
