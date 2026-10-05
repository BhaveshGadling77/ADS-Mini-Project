# ADS Mini Project — Compression Application

A cross-platform compression and decompression application built as an **ADS Mini Project**.

The project focuses on implementing compression algorithms and their underlying **data structures and algorithms** in C++, exposed through a backend server and used by a React Native mobile application.

## Task Assigned
Video compression => Sangram
Image compression => Jibran
Text File compression => Bhavesh
Audio File compression => Parth 
---

## 1. Architecture

```text
React Native Mobile App
          │
          │ HTTP / REST
          ▼
Node.js + Typescrip Server
          │
          │ Process communication
          ▼
C++ Compression Engine
```

### Responsibilities

| Module              | Responsibility                                                       |
| ------------------- | -------------------------------------------------------------------- |
| `mobile/`           | UI, file selection, user input, results and visualization            |
| `server/`           | REST API, validation, file handling, jobs and communication with C++ |
| `compression-core/` | Actual compression/decompression algorithms and data structures      |
| `shared/`           | Shared API schemas and types                                         |
| `docs/`             | Architecture, algorithms and API documentation                       |
| `scripts/`          | Build, development and testing scripts                               |
| `docker/`           | Containerization configuration                                       |

**Important:** The compression logic must remain inside `compression-core`. The server should not implement compression algorithms.

---

## 2. Project Structure

```text
ADS-Mini-Project/
│
├── mobile/                  # React Native application
│
├── server/                  # Node.js + TypeScript
│   ├── src/
│   │   ├── routes/
│   │   ├── controllers/
│   │   ├── services/
│   │   ├── middleware/
│   │   ├── types/
│   │   ├── utils/
│   │   └── config/
│   └── tests/
│
├── compression-core/        # C++17/20 compression engine
│   ├── include/
│   ├── src/
│   ├── tests/
│   └── cli/
│
├── shared/
│   ├── schemas/
│   └── types/
│
├── docs/
│   ├── architecture/
│   ├── algorithms/
│   ├── api/
│   └── development/
│
├── scripts/
│   ├── build/
│   ├── dev/
│   └── test/
│
└── docker/
    ├── server/
    └── compression-core/
```

`mobile/` is intentionally empty at the beginning. The React Native application will be initialized separately by the teammate responsible for the mobile module.

---

## 3. Compression Algorithms

The application will support four types of data.

### Text

* **LZW** — Trie
* **Static Huffman** — Min-Heap + Binary Tree
* **BWT** — Suffix Array + Move-to-Front

### Image

* **Spatial Quadtree**
* **Color Quantization** — KD-Tree
* **2D Haar Wavelets**

### Audio

* **1D Wavelet Tree**
* **Delta Encoding + RLE**
* **Linear Predictive Coding + Huffman**

### Video

* **Inter-frame Quadtree**
* **3D Octree**

These algorithms and their associated data structures follow the original project specification.

---

## 4. Compression Core

`compression-core/` is a **standalone C++17/20 module** containing all compression and decompression logic.

### Folder responsibilities

```text
compression-core/
│
├── include/
│   ├── core/
│   ├── algorithms/
│   ├── datastructures/
│   └── io/
│
├── src/
│   ├── core/
│   ├── algorithms/
│   ├── datastructures/
│   └── io/
│
├── tests/
└── cli/
```

### `include/`

Contains **public C++ headers/interfaces** used by the compression engine.

* `core/` — Common interfaces, configuration, result/metric structures and core abstractions.
* `algorithms/` — Header files for all compression/decompression algorithms.
* `datastructures/` — Header files for the data structures used by the algorithms.
* `io/` — Binary and bit-level reading/writing interfaces.

### `src/`

Contains the **implementations** corresponding to the headers in `include/`.

* `core/` — Core engine implementation.
* `algorithms/` — Actual compression/decompression implementations.
* `datastructures/` — Implementations of supporting data structures.
* `io/` — Binary file and bit-level I/O implementations.

### `tests/`

Contains tests for the compression core.

Tests should cover:

* Compression
* Decompression
* Round-trip correctness
* Edge cases
* Data-structure correctness
* Performance/metrics where relevant

### `cli/`

Contains the command-line entry point for the C++ engine.

The CLI allows the compression core to be tested independently from the server.

For example:

```text
compression-engine
        │
        ├── compress
        └── decompress
```

The Node.js server will initially communicate with this compiled executable.

### Important separation

```text
include/       → What the module exposes
src/           → How it is implemented
tests/         → Whether it works correctly
cli/           → How the engine is executed externally
```

The compression core must **not depend on Node.js, React Native, HTTP, or Fastify**.

---

## 5. Server

The backend will use:

* **Node.js**
* **TypeScript**
* **Fastify**

The server handles:

```text
HTTP Request
     ↓
Validation
     ↓
File handling
     ↓
Algorithm/parameter selection
     ↓
Invoke C++ engine
     ↓
Collect output + metrics
     ↓
HTTP Response
```

Initially, Node.js will communicate with the C++ engine by launching the compiled C++ executable.

The server should **orchestrate** compression, not perform the compression itself.

---

## 6. API

The initial API will provide endpoints such as:

```text
POST /api/compress
POST /api/decompress
```

A compression request should specify the required information, such as:

```text
modality
algorithm
input file
algorithm parameters
```

The response should provide the generated output and relevant metrics, such as:

```text
original size
compressed size
compression ratio
space saved
execution time
algorithm
```

The exact API contract will be documented in:

```text
docs/api/
```

---

## 7. Development Rules

### Keep modules independent

Do not put C++ compression logic inside the server or mobile application.

### Use the appropriate language

* Mobile → TypeScript / React Native
* Server → TypeScript / Node.js
* Compression → C++17/20

### Test independently

Every compression algorithm should have tests for:

* Compression
* Decompression
* Correctness / round-trip behaviour
* Edge cases
* Relevant performance metrics

### Keep documentation updated

Algorithm explanations belong in:

```text
docs/algorithms/
```

Architecture decisions belong in:

```text
docs/architecture/
```

API specifications belong in:

```text
docs/api/
```

---

## 8. Development Workflow

Each contributor should work inside the module they are assigned to.

```text
1. Create / switch to your branch
2. Implement your assigned module
3. Test locally
4. Update relevant documentation
5. Commit changes
6. Push branch
7. Create Pull Request
```

Before merging, ensure that your changes do not break the other modules.

---

## 9. Build Order

The project should be developed in this order:

```text
1. Compression Core
        ↓
2. C++ CLI / standalone testing
        ↓
3. Node.js Server
        ↓
4. Server ↔ C++ integration
        ↓
5. React Native application
        ↓
6. Mobile ↔ Server integration
        ↓
7. End-to-end testing
```

This keeps the core algorithms testable before introducing networking and mobile complexity.

---

## 10. Current Status

**Stage:** Project structure / initial setup

Next steps:

* Initialize React Native inside `mobile/`
* Initialize Node.js + TypeScript + Fastify inside `server/`
* Initialize CMake-based C++ project inside `compression-core/`
* Define the C++ ↔ server interface
* Implement algorithms incrementally
* Build the REST API
* Connect the mobile application
