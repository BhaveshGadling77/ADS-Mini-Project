# CODEC REST API Specification

Base URL: `http://localhost:3000`

---

## 1. System Endpoints

### `GET /health`
Verifies server health and connectivity.

**Response `200 OK`:**
```json
{
  "status": "ok",
  "timestamp": "2026-10-12T07:15:00.000Z"
}
```

---

## 2. Compression Endpoints

### `POST /api/compress`
Uploads and compresses an input file.

**Content-Type:** `multipart/form-data`

**Form Fields:**
- `file`: (Binary) The input file to compress.
- `algorithm`: (Text, optional) `"lzw"`, `"huffman"`, or `"bwt"`. Defaults to `"lzw"`.
- `modality`: (Text, optional) Defaults to `"text"`.

**Response `200 OK`:**
```json
{
  "success": true,
  "jobId": "e67b2d56-788c-486d-9653-5249cfd6f830",
  "createdAt": "2026-10-12T07:15:00.000Z",
  "operation": "compress",
  "algorithm": "lzw",
  "modality": "text",
  "originalFileName": "sample.txt",
  "outputFileName": "sample.txt.adsc",
  "metrics": {
    "algorithm": "lzw",
    "originalSizeBytes": 8389,
    "compressedSizeBytes": 4067,
    "compressionRatio": 2.0627,
    "spaceSavedPercent": 51.52,
    "durationMs": 1.35,
    "success": true
  },
  "downloadUrl": "/api/jobs/e67b2d56-788c-486d-9653-5249cfd6f830/download"
}
```

---

### `POST /api/decompress`
Uploads and restores an `.adsc` compressed file.

**Content-Type:** `multipart/form-data`

**Form Fields:**
- `file`: (Binary) The `.adsc` file to decompress.
- `algorithm`: (Text, optional) Auto-detected from file header if omitted.

**Response `200 OK`:**
```json
{
  "success": true,
  "jobId": "8294a64c-f09b-4ec2-beaa-eb8b60b73c4d",
  "createdAt": "2026-10-12T07:16:00.000Z",
  "operation": "decompress",
  "algorithm": "lzw",
  "modality": "text",
  "originalFileName": "sample.txt.adsc",
  "outputFileName": "sample.txt",
  "metrics": {
    "algorithm": "lzw",
    "originalSizeBytes": 8389,
    "compressedSizeBytes": 4067,
    "compressionRatio": 2.0627,
    "spaceSavedPercent": 0,
    "durationMs": 0.75,
    "success": true
  },
  "downloadUrl": "/api/jobs/8294a64c-f09b-4ec2-beaa-eb8b60b73c4d/download"
}
```

---

### `POST /api/compare`
Uploads a single text file and benchmarks all three compression algorithms simultaneously.

**Content-Type:** `multipart/form-data`

**Form Fields:**
- `file`: (Binary) The input text file.

**Response `200 OK`:**
```json
{
  "jobId": "f78103c8-66d4-4bb8-8686-35b91b8a5fcb",
  "originalFileName": "data.txt",
  "originalSizeBytes": 8389,
  "results": [
    {
      "algorithm": "lzw",
      "algorithmName": "LZW",
      "dataStructure": "Trie (Prefix Tree)",
      "compressedSizeBytes": 4067,
      "compressionRatio": 2.0627,
      "spaceSavedPercent": 51.52,
      "durationMs": 1.35,
      "success": true,
      "downloadUrl": "/api/jobs/f78103c8-66d4-4bb8-8686-35b91b8a5fcb/download?file=data.txt.lzw.adsc"
    },
    {
      "algorithm": "huffman",
      "algorithmName": "Static Huffman",
      "dataStructure": "Min-Heap + Binary Tree",
      "compressedSizeBytes": 5617,
      "compressionRatio": 1.4935,
      "spaceSavedPercent": 33.04,
      "durationMs": 0.35,
      "success": true,
      "downloadUrl": "/api/jobs/f78103c8-66d4-4bb8-8686-35b91b8a5fcb/download?file=data.txt.huffman.adsc"
    },
    {
      "algorithm": "bwt",
      "algorithmName": "BWT + MTF",
      "dataStructure": "Suffix Array + Move-to-Front List",
      "compressedSizeBytes": 5587,
      "compressionRatio": 1.5015,
      "spaceSavedPercent": 33.40,
      "durationMs": 4.19,
      "success": true,
      "downloadUrl": "/api/jobs/f78103c8-66d4-4bb8-8686-35b91b8a5fcb/download?file=data.txt.bwt.adsc"
    }
  ]
}
```

---

## 3. Job & History Endpoints

### `GET /api/jobs/:jobId`
Retrieves metadata and metrics for a specific operation.

### `GET /api/jobs/:jobId/download[?file=filename]`
Downloads the generated file with `Content-Disposition: attachment`.

### `GET /api/history[?limit=50]`
Returns list of past compression and decompression records.
