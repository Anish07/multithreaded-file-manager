# Multithreaded File Manager (C++17)

A parallelized file system scanner written in modern C++17.  
It recursively traverses directories, counts files and directories, and sums file sizes using a thread pool of worker threads.  

This project demonstrates **multithreaded programming, synchronization with mutexes and condition variables, and filesystem traversal** — core skills for systems and performance engineering.

---

## Features
- Recursively scans a directory tree
- Counts number of directories, files, and total bytes
- Uses multiple worker threads for concurrency
- Thread-safe task queue with condition variables
- Error handling for inaccessible paths

---

## Build Instructions

### Prerequisites
- CMake (>= 3.16)
- A C++17-compatible compiler (GCC, Clang, or MSVC)

### Steps
```bash
# Clone this repo
git clone https://github.com/Anish07/multithreaded-file-manager.git
cd multithreaded-file-manager

# Configure and build
cmake -S . -B build
cmake --build build -j

# Run
./build/multithreaded_file_manager <root_path> [num_threads]
