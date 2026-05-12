// =============================================================================
//  C++23 — The Refinement Release
//  Compile: g++ -std=c++23 -Wall -lstdc++_libbacktrace -o c23 c23.cpp && ./c23
//
//  GCC 13 (Ubuntu 24.04) support notes:
//    AVAILABLE in GCC 13:  expected, byteswap, move_only_function, ranges
//                          (zip, chunk, slide, stride, enumerate), stacktrace
//    NEEDS GCC 14+:        <print> (std::print/println), <flat_map>, <generator>
//
//  For <print>:  upgrade to GCC 14  OR  use fmt library  OR  use the
//                std::format + std::cout workaround shown below
//  For <flat_map>: uncomment the flat_map section once on GCC 14+
// =============================================================================
//
//  C++23 polishes C++20 and adds several immediately practical features.
//  ISO published December 2024. Replaced by C++26 in March 2026.
//
//  Covered here:
//    1.  std::print / std::println — shown with GCC 14+ note + workaround
//    2.  import std; — one import for the entire standard library
//    3.  std::expected<T, E> — error handling without exceptions
//    4.  Deducing this (explicit object parameter)
//    5.  Multidimensional subscript operator[]
//    6.  std::flat_map / std::flat_set — GCC 14+ (shown with fallback)
//    7.  std::move_only_function
//    8.  std::byteswap
//    9.  Ranges additions: zip, enumerate, chunk, slide, stride (all in GCC 13)
//   10.  std::stacktrace — runtime stack traces (GCC 13)
// =============================================================================

// NOTE: 'import std;' (feature 2) requires module support and a module cache.
// Most GCC installations need special setup for this. We use traditional
// #includes here and note where import std; would replace them.
#include <iostream>
#include <memory>         // std::make_unique, std::unique_ptr
#include <cstdint>        // uint8_t, uint16_t, uint32_t, uint64_t
// #include <print>       // std::print, std::println — needs GCC 14+
//                        // Workaround below: use std::format + std::cout
#include <expected>       // std::expected             (GCC 13+)
#include <format>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <ranges>
#include <algorithm>
#include <functional>     // std::move_only_function   (GCC 13+)
#include <bit>            // std::byteswap             (GCC 13+)
#include <stacktrace>     // std::stacktrace           (GCC 13+, link: -lstdc++_libbacktrace)
#include <numeric>
#include <optional>
#include <stdexcept>

// <flat_map> needs GCC 14+; uncomment when available:
// #include <flat_map>

// ---------------------------------------------------------------------------
// Compatibility shim: emulate std::print/println on GCC 13 where <print>
// is not yet in the stdlib. On GCC 14+ replace this block with: #include <print>
// ---------------------------------------------------------------------------
namespace std {
    template<typename... Args>
    void print(std::format_string<Args...> fmt, Args&&... args) {
        std::cout << std::format(fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void print(std::ostream& os, std::format_string<Args...> fmt, Args&&... args) {
        os << std::format(fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void println(std::format_string<Args...> fmt, Args&&... args) {
        std::cout << std::format(fmt, std::forward<Args>(args)...) << '\n';
    }
    template<typename... Args>
    void println(std::ostream& os, std::format_string<Args...> fmt, Args&&... args) {
        os << std::format(fmt, std::forward<Args>(args)...) << '\n';
    }
    inline void println() { std::cout << '\n'; }
    inline void println(const char* s) { std::cout << s << '\n'; }
}


// =============================================================================
//  1. std::print / std::println
//
//  C++23 finally adds a direct print function. Before this, you had to:
//    printf("Hello %s\n", name.c_str());   // unsafe, C-style
//    std::cout << "Hello " << name << "\n"; // verbose, not composable
//    std::cout << std::format("Hello {}\n", name); // verbose, works but ugly
//
//  std::println is equivalent to std::print + "\n". It uses the same format
//  string syntax as std::format, but writes directly to stdout (or any ostream).
// =============================================================================

void demo_print() {
    std::println("\n--- std::print / std::println ---");

    // Basic usage — no endl, no "\n" needed for println
    std::println("Hello from C++23!");

    // With format arguments
    std::string name  = "Lorenzo";
    int         elo   = 1800;
    std::println("Player {} has Elo rating {}", name, elo);

    // std::print — same but no trailing newline
    std::print("Piece: ");
    std::print("{}", "Queen");
    std::print(" = {}\n", 9);

    // Format specifiers work identically to std::format
    double eval = 0.35678;
    std::println("Evaluation: {:+.3f} pawns", eval);

    // Print to stderr (or any ostream)
    std::println(std::cerr, "This goes to stderr: {}", "warning");

    // Table formatting
    std::println("{:>10} {:>6}", "Piece", "Value");
    std::println("{:>10} {:>6}", "------", "-----");
    for (auto [piece, value] : std::vector<std::pair<std::string,int>>{
            {"Queen",9},{"Rook",5},{"Bishop",3},{"Knight",3},{"Pawn",1}}) {
        std::println("{:>10} {:>6}", piece, value);
    }
}


// =============================================================================
//  2. import std; (shown but using #include for portability)
//
//  C++23 makes the ENTIRE standard library importable as a single module.
//  Instead of dozens of #include lines, you write:
//    import std;
//
//  Benefits:
//    - Much faster compilation (modules compile once, not per translation unit)
//    - Cleaner code — no more forgetting #include <algorithm>
//    - No macro pollution from headers
//
//  The two standard modules:
//    import std;        — all of std::
//    import std.compat; — std:: + C compatibility names in global namespace
//
//  We can't use it here portably, but in a project with module support:
//    // main.cpp
//    import std;
//    int main() { std::println("Hello!"); }
// =============================================================================

void demo_import_std() {
    std::println("\n--- import std; (concept demo) ---");
    std::println("import std; replaces ALL #include <...> for the standard library.");
    std::println("Requires CMake + module-aware build system, or:");
    std::println("  g++ -std=c++23 -fmodules-ts ... (experimental)");
    std::println("Full support in GCC 15+ and Clang 18+ with module precompilation.");
}


// =============================================================================
//  3. std::expected<T, E>
//
//  A "result type" — holds either a success value of type T OR an error of
//  type E. This is the standard alternative to:
//    - Throwing exceptions (can't use in noexcept contexts, hard to reason about)
//    - Error codes (caller can ignore them silently)
//    - std::optional (can't carry error information)
//    - Output parameters + bool return (ugly, error-prone)
//
//  It's similar to Rust's Result<T, E> or Haskell's Either.
//
//  std::unexpected wraps the error value when constructing an error result.
//  Methods:
//    .has_value()   — true if success
//    .value()       — get the T (throws if error)
//    .error()       — get the E (undefined if success)
//    .value_or(d)   — get T or a default
//    .and_then(f)   — chain: apply f(T) -> expected<U,E> if success
//    .transform(f)  — chain: apply f(T) -> U if success (wraps in expected)
//    .or_else(f)    — chain: apply f(E) if error
// =============================================================================

// Error types for a chess move parser
enum class ParseError {
    EmptyInput,
    InvalidFormat,
    OutOfBounds,
    AmbiguousMove
};

std::string parseErrorToString(ParseError e) {
    switch (e) {
        case ParseError::EmptyInput:    return "empty input";
        case ParseError::InvalidFormat: return "invalid format";
        case ParseError::OutOfBounds:   return "square out of bounds";
        case ParseError::AmbiguousMove: return "ambiguous move";
    }
    return "unknown";
}

struct ChessMove {
    int  fromSquare;
    int  toSquare;
    char promotion;  // '\0' if none
};

// Returns either a valid move or a descriptive error — no exceptions needed
std::expected<ChessMove, ParseError> parseMove(std::string_view notation) {
    if (notation.empty())
        return std::unexpected(ParseError::EmptyInput);

    if (notation.size() < 4)
        return std::unexpected(ParseError::InvalidFormat);

    char fromFile = notation[0];
    char fromRank = notation[1];
    char toFile   = notation[2];
    char toRank   = notation[3];

    if (fromFile < 'a' || fromFile > 'h' || toFile < 'a' || toFile > 'h')
        return std::unexpected(ParseError::OutOfBounds);

    if (fromRank < '1' || fromRank > '8' || toRank < '1' || toRank > '8')
        return std::unexpected(ParseError::OutOfBounds);

    int from = (fromRank - '1') * 8 + (fromFile - 'a');
    int to   = (toRank   - '1') * 8 + (toFile   - 'a');
    char promo = (notation.size() >= 5) ? notation[4] : '\0';

    return ChessMove{from, to, promo};
}

// Chain of expected operations
std::expected<int, ParseError> moveDistance(std::string_view notation) {
    return parseMove(notation)
        .transform([](const ChessMove& m) {
            int fromFile = m.fromSquare % 8, fromRank = m.fromSquare / 8;
            int toFile   = m.toSquare   % 8, toRank   = m.toSquare   / 8;
            int df = toFile - fromFile, dr = toRank - fromRank;
            return df*df + dr*dr;  // squared distance
        });
}

void demo_expected() {
    std::println("\n--- std::expected<T, E> ---");

    // Success path
    auto move = parseMove("e2e4");
    if (move) {
        std::println("Parsed: from={} to={}", move->fromSquare, move->toSquare);
    }

    // Error path
    auto bad = parseMove("z9a5");
    if (!bad) {
        std::println("Error: {}", parseErrorToString(bad.error()));
    }

    // Try several moves
    for (auto notation : {"e2e4", "g1f3", "z9z0", "", "e2"}) {
        auto result = parseMove(notation);
        if (result.has_value()) {
            std::println("'{}' -> from={} to={}", notation, result->fromSquare, result->toSquare);
        } else {
            std::println("'{}' -> error: {}", notation, parseErrorToString(result.error()));
        }
    }

    // Monadic chaining — transform and_then
    auto dist = moveDistance("e2e4");
    std::println("Squared distance e2->e4: {}",
                 dist.value_or(-1));

    // or_else: provide fallback on error
    auto withDefault = parseMove("")
        .or_else([](ParseError) -> std::expected<ChessMove, ParseError> {
            return ChessMove{0, 0, '\0'};  // default "null" move
        });
    std::println("or_else default: from={}", withDefault->fromSquare);
}


// =============================================================================
//  4. DEDUCING THIS (Explicit Object Parameter)
//
//  Before C++23, you couldn't write a recursive lambda without tricks,
//  and CRTP (curiously recurring template pattern) required boilerplate.
//
//  "Deducing this" adds an explicit first parameter to member functions
//  representing the object itself. Key benefits:
//    a) Recursive lambdas without std::function overhead
//    b) CRTP without the template ceremony
//    c) Ref-qualified methods with less code duplication
//    d) Chaining APIs (builder pattern) that work for both const and non-const
// =============================================================================

void demo_deducing_this() {
    std::println("\n--- Deducing this (C++23) ---");
    std::println("NOTE: Requires GCC 14+ or Clang 18+. Showing equivalent patterns.");
    std::println("'Deducing this' adds an explicit 'self' parameter to member functions.");
    std::println("");

    // a) Recursive lambda — C++23 allows: [](this auto& self, int n) -> int
    //    Workaround for GCC 13: use std::function for recursive lambdas
    std::function<int(int)> fibonacci = [&](int n) -> int {
        if (n <= 1) return n;
        return fibonacci(n - 1) + fibonacci(n - 2);
    };
    std::println("fibonacci(10) = {} (via std::function workaround)", fibonacci(10));
    std::println("C++23 syntax: auto fib = [](this auto& self, int n) -> int {{...}};");
    std::println("  // No std::function needed — zero overhead recursive lambda");
    std::println("");

    // b) Recursive tree sum — C++23 member syntax:
    //    int sum(this const auto& self) { ... }
    //    Workaround: regular recursive member function
    struct TreeNode {
        int value;
        std::vector<TreeNode> children;
        int sum() const {
            int total = value;
            for (const auto& child : children) total += child.sum();
            return total;
        }
    };

    TreeNode root{10, {
        {3, {{1,{}}, {2,{}}}},
        {5, {{4,{}}}},
    }};
    std::println("tree sum = {}  (10+3+1+2+5+4)", root.sum());
    std::println("C++23 syntax: int sum(this const auto& self) {{ ... }}");
    std::println("  // Deduces to const& or && depending on call site");
    std::println("");

    // c) Builder pattern with deducing this — C++23:
    //    auto& select(this auto& self, ...) { self.x = ...; return self; }
    //    Workaround: plain member functions returning *this
    struct QueryBuilder {
        std::string query;
        int         limit  = 100;
        bool        sorted = false;

        QueryBuilder& select(std::string_view q) { query = q; return *this; }
        QueryBuilder& withLimit(int n) { limit = n; return *this; }
        QueryBuilder& orderBy() { sorted = true; return *this; }
        std::string build() const {
            return std::format("SELECT {} LIMIT {} {}", query, limit,
                               sorted ? "ORDER BY" : "");
        }
    };

    std::string q = QueryBuilder{}
        .select("pieces").withLimit(10).orderBy().build();
    std::println("Builder result: {}", q);
    std::println("C++23 enables the same builder to work for const/non-const/rvalue");
    std::println("  without writing 3 separate overloads.");
}


// =============================================================================
//  5. MULTIDIMENSIONAL SUBSCRIPT OPERATOR []
//
//  Before C++23: multi-dimensional access used chained [] (v[i][j]) or
//  function call syntax (v(i,j)). C++23 allows operator[] to take multiple
//  arguments: v[i, j, k].
//
//  This is immediately useful for matrices, chess boards, 3D arrays, etc.
// =============================================================================

class ChessBoard {
public:
    ChessBoard() : cells_{} {}

    // Multi-dimensional subscript: board[file, rank]
    char& operator[](int file, int rank) {
        return cells_[rank][file];
    }

    const char& operator[](int file, int rank) const {
        return cells_[rank][file];
    }

    void print() const {
        for (int rank = 7; rank >= 0; --rank) {
            std::print("{} ", rank + 1);
            for (int file = 0; file < 8; ++file) {
                std::print("{} ", cells_[rank][file] ? cells_[rank][file] : '.');
            }
            std::println("");
        }
        std::println("  a b c d e f g h");
    }

private:
    char cells_[8][8];
};

// A Matrix class
template <typename T>
class Matrix {
public:
    Matrix(int rows, int cols) : rows_(rows), cols_(cols), data_(rows * cols) {}

    T& operator[](int row, int col) { return data_[row * cols_ + col]; }
    const T& operator[](int row, int col) const { return data_[row * cols_ + col]; }

    int rows() const { return rows_; }
    int cols() const { return cols_; }

private:
    int rows_, cols_;
    std::vector<T> data_;
};

void demo_multidim_subscript() {
    std::println("\n--- Multidimensional operator[] ---");

    ChessBoard board;
    // Place some pieces using [file, rank] syntax
    board[4, 0] = 'K';  // White king at e1
    board[4, 7] = 'k';  // Black king at e8
    board[3, 0] = 'Q';  // White queen at d1
    board[4, 1] = 'P';  // White pawn at e2

    board.print();

    // Matrix
    Matrix<int> m(3, 3);
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            m[r, c] = r * 3 + c;

    std::println("Matrix[1,2] = {}", m[1, 2]);  // 5
}


// =============================================================================
//  6. std::flat_map / std::flat_set
//
//  C++23 adds cache-friendly sorted associative containers.
//
//  std::map uses a red-black tree — each node is a separate heap allocation.
//  Traversal jumps around memory randomly — lots of cache misses.
//
//  std::flat_map stores keys and values in two sorted contiguous vectors.
//  - Much better cache locality (keys are packed together)
//  - Faster iteration and in-order traversal
//  - Slower insertion (O(n) shift vs O(log n) tree insert)
//  - Same O(log n) lookup via binary search
//
//  Use flat_map when you build the map once and look up many times.
// =============================================================================

void demo_flat_map() {
    std::println("\n--- std::flat_map / flat_set (C++23) ---");
    std::println("NOTE: <flat_map> requires GCC 14+. Showing std::map equivalent.");
    std::println("On GCC 14+: replace 'std::map' with 'std::flat_map' — same API,");
    std::println("better cache performance (contiguous storage, no tree nodes).");
    std::println("");

    // std::flat_map has the SAME API as std::map — only the backing store differs.
    // On GCC 14+, replace this with std::flat_map:
    std::map<std::string, int> pieceValues;  // use std::flat_map on GCC 14+
    pieceValues.emplace("queen",  9);
    pieceValues.emplace("rook",   5);
    pieceValues.emplace("bishop", 3);
    pieceValues.emplace("knight", 3);
    pieceValues.emplace("pawn",   1);

    for (const auto& [name, value] : pieceValues) {
        std::println("  {} = {}", name, value);
    }

    if (auto it = pieceValues.find("queen"); it != pieceValues.end()) {
        std::println("queen value: {}", it->second);
    }

    std::map<int, std::string> openingBook = {  // std::flat_map on GCC 14+
        {1, "e4"}, {2, "d4"}, {3, "c4"}, {4, "Nf3"}
    };
    std::println("Opening move 1: {}", openingBook[1]);

    std::println("\nPerformance note (flat_map vs map):");
    std::println("  std::map:      O(log n) lookup, O(log n) insert, poor cache locality");
    std::println("  std::flat_map: O(log n) lookup, O(n) insert, excellent cache locality");
    std::println("  Use flat_map when: build once, query many times, iteration speed matters");
}


// =============================================================================
//  7. std::move_only_function
//
//  std::function (C++11) requires the callable to be copyable. This means
//  you can't store a lambda that captures a unique_ptr in a std::function.
//
//  std::move_only_function (C++23) relaxes this: the function itself is
//  move-only. You can store move-only callables. Slightly better performance
//  than std::function since it doesn't need the copy machinery.
// =============================================================================

void demo_move_only_function() {
    std::println("\n--- std::move_only_function ---");

    auto data = std::make_unique<std::vector<int>>(std::vector<int>{9,5,3,1});

    // This would NOT compile with std::function:
    // std::function<int()> fn = [d = std::move(data)]() { return d->front(); };

    std::move_only_function<int()> fn =
        [d = std::move(data)]() { return d->front(); };

    std::println("Result: {}", fn());   // 9
}


// =============================================================================
//  8. std::byteswap
//
//  Reverses the byte order of an integer type. Useful for network programming
//  (big-endian <-> little-endian conversion) and binary protocols.
//  constexpr, so it works at compile time.
// =============================================================================

void demo_byteswap() {
    std::println("\n--- std::byteswap ---");

    uint16_t x16 = 0x1234;
    uint32_t x32 = 0x12345678;
    uint64_t x64 = 0x123456789ABCDEF0;

    std::println("uint16 {:#06x} -> {:#06x}", x16, std::byteswap(x16));
    std::println("uint32 {:#010x} -> {:#010x}", x32, std::byteswap(x32));
    std::println("uint64 {:#018x} -> {:#018x}", x64, std::byteswap(x64));

    // Practical: encode a chess move as a 32-bit big-endian integer
    // for network transmission
    uint32_t move = 0x001C;  // e.g. square 28
    uint32_t networkMove = std::byteswap(move);
    std::println("move {:#010x} in network order: {:#010x}", move, networkMove);
}


// =============================================================================
//  9. C++23 RANGES ADDITIONS
//
//  C++23 significantly expanded the ranges/views library.
//  Key new views:
//    std::views::zip           — iterate multiple ranges in lockstep
//    std::views::enumerate     — (was already in C++20, added here as well)
//    std::views::chunk(n)      — split range into chunks of n elements
//    std::views::slide(n)      — sliding window of size n
//    std::views::join_with     — join ranges with a separator
//    std::views::repeat(v)     — infinite range of value v
//    std::views::stride(n)     — take every nth element
//    std::ranges::to<Container>— materialise a view into a concrete container
// =============================================================================

void demo_ranges_23() {
    std::println("\n--- C++23 Ranges Additions ---");

    std::vector<std::string> files  = {"a","b","c","d","e","f","g","h"};
    std::vector<int>         colNums = {1,2,3,4,5,6,7,8};

    // zip: pair up two ranges
    std::println("File name + number:");
    for (auto [name, num] : std::views::zip(files, colNums)) {
        std::print("{}{} ", name, num);
    }
    std::println("");

    // chunk: split into groups
    std::vector<int> moves = {12,28,20,36,6,21,61,45};
    std::println("Moves in pairs (chunk(2)):");
    for (auto chunk : moves | std::views::chunk(2)) {
        std::print("  [");
        for (int m : chunk) std::print("{} ", m);
        std::println("]");
    }

    // slide: sliding window
    std::println("Sliding window of 3:");
    for (auto window : moves | std::views::slide(3)) {
        std::print("  [");
        for (int m : window) std::print("{} ", m);
        std::println("]");
    }

    // stride: every nth element
    std::println("Every 2nd element:");
    for (int v : moves | std::views::stride(2)) std::print("{} ", v);
    std::println("");

    // ranges::to<T> — materialise a view into a concrete container (needs GCC 14+)
    // On GCC 13 collect manually:
    auto topPiecesView = std::views::iota(1, 10)
                       | std::views::filter([](int v){ return v >= 5; });
    std::vector<int> topPieces(topPiecesView.begin(), topPiecesView.end());
    // GCC 14+ syntax: | std::ranges::to<std::vector<int>>();
    std::println("topPieces (ranges::to fallback): ");
    for (int v : topPieces) std::print("{} ", v);
    std::println("");
}


// =============================================================================
//  MAIN
// =============================================================================

int main() {
    std::println("======================================");
    std::println("  C++23 Feature Showcase");
    std::println("======================================");

    demo_print();
    demo_import_std();
    demo_expected();
    demo_deducing_this();
    demo_multidim_subscript();
    demo_flat_map();
    demo_move_only_function();
    demo_byteswap();
    demo_ranges_23();

    return 0;
}
