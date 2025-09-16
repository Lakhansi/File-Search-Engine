# File-Search-Engine
File Search Engine - A high-performance C++17 search application that rapidly indexes and searches filesystems. Implements hash maps for O(1) filename searches, trie structures for prefix matching, and inverted indexes for content search. Features multithreaded indexing, intelligent ranking, and persistent caching for instant results. 

## 🚀 Features

- **Fast File Indexing**: Recursive directory scanning using `std::filesystem`
- **Multiple Search Methods**:
  - Exact filename search (Hash Map - O(1) complexity)
  - Prefix search (Trie data structure)
  - File extension search
  - Full-text content search (Inverted Index)
- **Advanced Ranking**: Results sorted by size, date, or relevance
- **Multithreaded Indexing**: Optimized performance with smart thread management
- **Persistent Cache**: Index saved to disk for instant startup on subsequent runs

## 🏗️ Architecture

### Core Data Structures
- **Hash Maps**: For O(1) exact filename and extension searches
- **Trie**: For efficient prefix-based autocomplete functionality
- **Inverted Index**: For fast full-text content search
- **Metadata Storage**: File paths, sizes, modification dates, and content

### Key Components
- `main.cpp`: Application entry point and demonstration of all features
- `Index.cpp/h`: Central data management and search functionality
- `Indexer.cpp/h`: Filesystem crawling and metadata extraction
- `Trie.cpp/h`: Prefix search implementation
- `utils.h`: File content reading utilities

## 📦 Installation & Build

### Prerequisites
- C++17 compatible compiler (GCC, Clang, or MSVC)
- CMake 3.16+
- Mingw-w64 (Windows) or build-essentials (Linux)

### Build Instructions

```bash
# Clone the repository
git clone <your-repo-url>
cd FileSearchApp

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake -G "MinGW Makefiles" ..

# Build the project
make

# Run the application
./FileSearchApp
```
🎯 Usage
Basic Indexing and Search
The application will automatically index a test_folder directory and demonstrate various search capabilities:

```bash
// Example search operations implemented:
index.searchByFilename("example.txt");
index.searchByPrefix("img_2020");
index.searchByExtension("pdf");
index.searchByContent("search query");
```
Cache Management
The index is automatically saved to index_cache.bin and loaded on subsequent runs for instant startup.

🔧 Project Phases Completed
Core Indexing Engine (std::filesystem integration)

Data Structures Implementation (Hash maps, Trie, Inverted index)

Search Algorithms (Multiple search methods)

Result Ranking System (Size, date, and relevance sorting)

Multithreading (Optimized indexing performance)

Persistence Layer (Disk caching for instant loading)

🏆 Performance Features
O(1) complexity for exact filename searches

Prefix search with Trie data structure

Content search with inverted indexing

Smart multithreading adaptive to workload size

Binary serialization for fast cache loading

🤝 Contributing
This project demonstrates advanced C++ features including:

Modern C++17 standards and std::filesystem

Template metaprogramming and STL containers

Memory management and efficient data structures

Algorithm optimization and complexity analysis

Multithreading patterns and synchronization

📄 License
MIT License - feel free to use this code for learning and development purposes.

