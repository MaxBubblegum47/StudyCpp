// =============================================================================
//  C++20 — The Four Pillars
//  Compile: g++ -std=c++20 -Wall -o c20 c20.cpp && ./c20
// =============================================================================
//
//  C++20 is comparable to C++11 in scope. The four headline features —
//  Concepts, Ranges, Coroutines, Modules — each represent a major new way
//  of expressing programs. There are many smaller but very useful additions too.
//
//  Covered here:
//    1.  Concepts — named type constraints for templates
//    2.  Ranges & Views — composable lazy pipelines
//    3.  Coroutines (co_await, co_yield, co_return) — brief intro
//    4.  std::format — Python-style string formatting
//    5.  Three-way comparison <=> (spaceship operator)
//    6.  std::span<T> — non-owning view over contiguous data
//    7.  consteval & constinit
//    8.  Designated initialisers
//    9.  std::jthread & std::stop_token
//   10.  std::bit_cast
//   11.  std::numbers — mathematical constants
//   12.  Abbreviated function templates (auto parameters)
//   13.  std::erase / std::erase_if
// =============================================================================

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <array>
#include <span>
#include <ranges>
#include <algorithm>
#include <concepts>
#include <format>
#include <numeric>
#include <numbers>
#include <bit>
#include <thread>
#include <stop_token>
#include <type_traits>
#include <compare>    // for <=>

// =============================================================================
//  1. CONCEPTS
//
//  Concepts are named, boolean predicates on template parameters that are
//  evaluated at compile time. They give you:
//    a) Readable error messages — "constraint not satisfied: T must be Numeric"
//       instead of a 50-line template instantiation traceback
//    b) Expressive function signatures — you can read what a function accepts
//    c) Overload resolution — the compiler picks the most constrained match
//
//  You can define your own concepts with `concept`, or use the standard
//  library concepts in <concepts> and <iterator>/<ranges>.
// =============================================================================

// A simple concept: T must support + - * / and comparisons
template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

// A concept built from multiple requirements
template <typename T>
concept ChessPieceLike = requires(T t) {
    { t.name()  } -> std::convertible_to<std::string>;
    { t.value() } -> std::convertible_to<int>;
};

// A concept checking that T is a container with value_type
template <typename T>
concept Container = requires(T c) {
    typename T::value_type;
    { c.begin() } -> std::input_or_output_iterator;
    { c.end()   } -> std::input_or_output_iterator;
    { c.size()  } -> std::convertible_to<std::size_t>;
};

// --- Functions constrained by concepts ---

// Only compiles for Numeric types
template <Numeric T>
T square(T x) { return x * x; }

// Shorthand syntax (C++20 abbreviated template):
// auto square(Numeric auto x) — same thing, more concise

// Only compiles for types satisfying Container
template <Container C>
auto containerSum(const C& c) {
    typename C::value_type total{};
    for (const auto& v : c) total += v;
    return total;
}

// Requires clause syntax — alternative to concept name before parameter
template <typename T>
    requires Numeric<T> && (sizeof(T) >= 4)
T safeDivide(T a, T b) {
    if (b == 0) throw std::domain_error("division by zero");
    return a / b;
}

// Concept-based overloading — compiler picks most constrained match
template <typename T>
std::string describe(T) { return "unknown type"; }

template <std::integral T>
std::string describe(T v) { return "integral: " + std::to_string(v); }

template <std::floating_point T>
std::string describe(T v) { return "float: " + std::to_string(v); }

struct Queen {
    std::string name()  const { return "Queen"; }
    int         value() const { return 9; }
};

void demo_concepts() {
    std::cout << "\n--- Concepts ---\n";

    std::cout << "square(5)    = " << square(5)    << "\n";
    std::cout << "square(3.14) = " << square(3.14) << "\n";
    // square(std::string("x"));   // compile error: string doesn't satisfy Numeric

    std::vector<int> v = {1, 3, 3, 5, 9};
    std::cout << "containerSum({1,3,3,5,9}) = " << containerSum(v) << "\n";

    std::cout << "safeDivide(10, 3) = " << safeDivide(10, 3) << "\n";

    std::cout << "describe(42)     = " << describe(42)     << "\n";
    std::cout << "describe(3.14)   = " << describe(3.14)   << "\n";
    std::cout << "describe('A')    = " << describe('A')    << "\n";

    // Checking concept satisfaction
    std::cout << "Queen satisfies ChessPieceLike: "
              << std::boolalpha << ChessPieceLike<Queen> << "\n";
    std::cout << "int satisfies ChessPieceLike:   "
              << ChessPieceLike<int> << "\n";
}


// =============================================================================
//  2. RANGES & VIEWS
//
//  The Ranges library brings composable, lazy, pipeline-style operations to C++.
//  A "view" is a range adaptor: it doesn't copy or modify data — it produces
//  elements on demand when you iterate.
//
//  Key idea: views are lazy. std::views::filter doesn't allocate a new vector;
//  it wraps the original range and skips elements on the fly during iteration.
//
//  The | operator is the pipe — it chains views together just like Unix pipes.
// =============================================================================

void demo_ranges() {
    std::cout << "\n--- Ranges & Views ---\n";

    // Basic range algorithms — work directly on containers (no begin/end)
    std::vector<int> values = {5, 9, 1, 3, 7, 2, 8, 4, 6};
    std::ranges::sort(values);
    std::cout << "sorted: ";
    for (int v : values) std::cout << v << " ";
    std::cout << "\n";

    // --- Views pipeline ---
    // Build a sequence lazily: even numbers from 0 to 19, squared, take first 5
    auto pipeline = std::views::iota(0, 20)
                  | std::views::filter([](int x){ return x % 2 == 0; })
                  | std::views::transform([](int x){ return x * x; })
                  | std::views::take(5);

    std::cout << "even squares (first 5): ";
    for (int v : pipeline) std::cout << v << " ";
    std::cout << "\n";
    // Output: 0 4 16 36 64
    // No intermediate vectors created — fully lazy

    // Reverse a vector view
    std::vector<std::string> moves = {"e4","e5","Nf3","Nc6","Bb5"};
    std::cout << "moves in reverse: ";
    for (const auto& m : std::views::reverse(moves)) {
        std::cout << m << " ";
    }
    std::cout << "\n";

    // Filter + transform: get doubled values of pieces > 3
    std::vector<int> pieces = {1, 3, 3, 5, 9, 0, 1};
    auto highValueDoubled = pieces
                          | std::views::filter([](int v){ return v > 3; })
                          | std::views::transform([](int v){ return v * 2; });
    std::cout << "high-value pieces doubled: ";
    for (int v : highValueDoubled) std::cout << v << " ";
    std::cout << "\n";

    // std::ranges::find_if — returns an iterator directly
    auto it = std::ranges::find_if(pieces, [](int v){ return v == 9; });
    if (it != pieces.end()) {
        std::cout << "Found queen (value 9) in pieces list\n";
    }

    // zip two ranges together (C++23 added zip; C++20 enumerate is in libstdc++14+)
    std::vector<std::string> files = {"a","b","c","d","e","f","g","h"};
    std::cout << "Files with indices (manual): ";
    for (int idx = 0; idx < (int)files.size(); ++idx) {
        std::cout << idx << ":" << files[idx] << " ";
    }
    std::cout << "\n";
}


// =============================================================================
//  3. std::format — Python-style String Formatting
//
//  Before C++20: printf (unsafe, no types), std::ostringstream (verbose),
//  or third-party fmt library. std::format provides type-safe, extensible,
//  Unicode-aware formatting.
//
//  Note: std::print/println arrived in C++23. For now (C++20) we use
//  std::format() + std::cout, or put formatted strings into variables.
// =============================================================================

void demo_format() {
    std::cout << "\n--- std::format ---\n";

    // Basic positional formatting
    std::string s1 = std::format("Piece: {} at square {}", "Queen", 27);
    std::cout << s1 << "\n";

    // Numbered arguments
    std::string s2 = std::format("{0} takes {1}, {1} score: {2}", "White", "pawn", 1);
    std::cout << s2 << "\n";

    // Format specifiers — same syntax as Python's format strings
    double eval = 0.35678;
    std::string s3 = std::format("Evaluation: {:+.3f}", eval);  // +0.357
    std::cout << s3 << "\n";

    // Integer formatting: width, fill, alignment
    std::cout << std::format("{:>10} {:>5}\n", "Piece", "Value");
    std::cout << std::format("{:>10} {:>5}\n", "Queen", 9);
    std::cout << std::format("{:>10} {:>5}\n", "Rook",  5);
    std::cout << std::format("{:>10} {:>5}\n", "Bishop",3);

    // Hexadecimal and binary
    unsigned int bitmask = 0b10110101;
    std::cout << std::format("bitmask: dec={0:d} hex={0:#x} bin={0:#b}\n", bitmask);

    // Build a FEN-like position string
    int file = 4, rank = 3;   // e4
    char piece = 'Q';
    std::string pos = std::format("{}{}{}", piece, (char)('a'+file), rank+1);
    std::cout << "Formatted position: " << pos << "\n";
}


// =============================================================================
//  4. THREE-WAY COMPARISON <=> (Spaceship Operator)
//
//  Before C++20: to support sorting and comparison you had to implement
//  operator<, operator>, operator<=, operator>=, operator==, operator!=
//  — that's SIX functions per class.
//
//  C++20: one operator<=> is enough. The compiler auto-generates all the
//  others from it.
//
//  Return types:
//    std::strong_ordering  — for types with total order (ints, strings)
//    std::weak_ordering    — for types where equal doesn't mean identical
//    std::partial_ordering — for types where some pairs are incomparable (floats: NaN)
// =============================================================================

struct Square {
    int file;   // 0-7
    int rank;   // 0-7

    // = default: the compiler generates <=> member-by-member
    // (file compared first, then rank — in declaration order)
    auto operator<=>(const Square&) const = default;
    // This ALSO auto-generates operator== from <=>
};

struct EloRating {
    double rating;
    std::string name;

    // Custom <=> — order by rating descending, then name ascending
    std::strong_ordering operator<=>(const EloRating& other) const {
        if (rating != other.rating) {
            // Descending by rating: if our rating is higher, we come first (<)
            if (other.rating < rating) return std::strong_ordering::less;
            return std::strong_ordering::greater;
        }
        return name <=> other.name;  // ascending by name
    }
    bool operator==(const EloRating& other) const = default;
};

void demo_spaceship() {
    std::cout << "\n--- Three-way Comparison <=> ---\n";

    Square a{4, 3};   // e4
    Square b{4, 5};   // e6
    Square c{4, 3};   // e4 (same as a)

    std::cout << "a < b: " << std::boolalpha << (a < b) << "\n";
    std::cout << "a > b: " << (a > b) << "\n";
    std::cout << "a == c: " << (a == c) << "\n";

    // Sort squares using auto-generated operators
    std::vector<Square> squares = {{4,3},{1,0},{4,0},{1,3}};
    std::ranges::sort(squares);
    std::cout << "Sorted squares: ";
    for (auto& [f, r] : squares) std::cout << "(" << f << "," << r << ") ";
    std::cout << "\n";

    // EloRating with custom ordering
    std::vector<EloRating> players = {
        {1650, "Bob"}, {1800, "Alice"}, {1800, "Carol"}, {1550, "Dave"}
    };
    std::ranges::sort(players);
    std::cout << "Players by Elo:\n";
    for (const auto& [elo, name] : players) {
        std::cout << "  " << std::format("{} {:.0f}\n", name, elo);
    }
}


// =============================================================================
//  5. std::span<T>
//
//  A non-owning view into a contiguous sequence of elements.
//  Essentially a (pointer, length) pair with a nice interface.
//
//  Why it matters: in C++98/11, functions that needed to accept "an array of
//  things" had to choose between T* + size (unsafe), const vector<T>& (forces
//  heap allocation on the caller), or a template (code bloat). span accepts
//  any contiguous sequence uniformly.
// =============================================================================

// Works with C arrays, std::array, std::vector — any contiguous buffer
int materialCount(std::span<const int> pieces) {
    int total = 0;
    for (int v : pieces) total += v;
    return total;
}

// Zero-copy subspan: view a portion of a larger buffer
void printBoard(std::span<const int, 64> board) {
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            std::cout << board[rank * 8 + file] << " ";
        }
        std::cout << "\n";
    }
}

void demo_span() {
    std::cout << "\n--- std::span ---\n";

    // Works with C arrays
    int arr[] = {9, 5, 3, 3, 1, 1};
    std::cout << "C-array material: " << materialCount(arr) << "\n";

    // Works with std::vector
    std::vector<int> vec = {9, 5, 5, 3, 3, 1, 1, 1, 1};
    std::cout << "vector material:  " << materialCount(vec) << "\n";

    // Works with std::array
    std::array<int, 4> sarr = {3, 3, 5, 9};
    std::cout << "std::array material: " << materialCount(sarr) << "\n";

    // Subspan — a view into part of a span
    std::vector<int> allPieces = {9, 5, 5, 3, 3, 1, 1, 1, 1,
                                   9, 5, 5, 3, 3, 1, 1, 1, 1};
    std::span<const int> whitePieces = std::span(allPieces).subspan(0, 9);
    std::span<const int> blackPieces = std::span(allPieces).subspan(9, 9);
    std::cout << "white: " << materialCount(whitePieces) << "\n";
    std::cout << "black: " << materialCount(blackPieces) << "\n";
}


// =============================================================================
//  6. consteval & constinit
//
//  consteval: "always evaluate at compile time" — stronger than constexpr.
//             A consteval function MUST be called with compile-time arguments.
//             It cannot be called at runtime. Use for functions where runtime
//             evaluation would be a bug (e.g. security-sensitive hashing,
//             validated string constants).
//
//  constinit: "initialise at compile time, but the variable isn't const".
//             Prevents the "static initialisation order fiasco" — guarantees
//             the variable is zero/constant-initialised before main() starts.
//             Unlike constexpr, the variable can be modified later.
// =============================================================================

// consteval: MUST be called at compile time
consteval int squareIndex(int file, int rank) {
    if (file < 0 || file > 7 || rank < 0 || rank > 7)
        throw "square out of bounds";   // compile error if invalid
    return rank * 8 + file;
}

// constinit: guaranteed compile-time init, but NOT const (can be modified)
constinit int globalCounter = 0;   // will not have static init order issues
constinit const int QUEEN_VALUE = 9;  // constinit + const = truly immutable

void demo_consteval_constinit() {
    std::cout << "\n--- consteval & constinit ---\n";

    // squareIndex MUST be called with compile-time args
    constexpr int e4 = squareIndex(4, 3);  // 4*8+3 = 28 — at compile time
    constexpr int d1 = squareIndex(3, 0);  // 3
    std::cout << "e4 = square " << e4 << "\n";
    std::cout << "d1 = square " << d1 << "\n";

    // int file = 4;
    // squareIndex(file, 3);  // ERROR: consteval requires compile-time args

    // constinit
    globalCounter = 42;   // modifiable at runtime
    std::cout << "globalCounter = " << globalCounter << "\n";
    std::cout << "QUEEN_VALUE   = " << QUEEN_VALUE   << "\n";
}


// =============================================================================
//  7. DESIGNATED INITIALISERS
//
//  Named initialisation of struct/class members. You can initialise only the
//  members you care about (rest are zero-initialised), and the names document
//  what each value means.
//
//  The member names must match the struct definition and be in order.
//  (C11 allows out-of-order, C++20 does not — must be in declaration order.)
// =============================================================================

struct GameConfig {
    int    searchDepth    = 5;
    bool   useAlphaBeta   = true;
    bool   useTransTable  = true;
    int    transTableMB   = 64;
    double timeLimit      = 5.0;
    bool   ponderEnabled  = false;
};

void demo_designated_init() {
    std::cout << "\n--- Designated Initialisers ---\n";

    // Only specify what differs from defaults — clear intent
    GameConfig fast = {
        .searchDepth  = 3,
        .useTransTable = false,
        .timeLimit     = 1.0
    };
    std::cout << "fast config: depth=" << fast.searchDepth
              << " transTable=" << std::boolalpha << fast.useTransTable
              << " time=" << fast.timeLimit << "s\n";

    GameConfig deep = {
        .searchDepth  = 12,
        .transTableMB = 512,
        .timeLimit    = 30.0,
        .ponderEnabled = true
    };
    std::cout << "deep config: depth=" << deep.searchDepth
              << " ttMB=" << deep.transTableMB
              << " time=" << deep.timeLimit << "s\n";
}


// =============================================================================
//  8. std::jthread (joining thread) & std::stop_token
//
//  std::thread in C++11 had a problem: you MUST call join() or detach()
//  before the thread object is destroyed, or the program calls terminate().
//
//  std::jthread fixes this:
//    - Automatically joins on destruction (RAII — no forgotten join())
//    - Supports cooperative cancellation via std::stop_token
//
//  This is the correct way to write threads in C++20+.
// =============================================================================

void searchWorker(std::stop_token token, int depth, const std::string& name) {
    std::cout << name << " searching at depth " << depth << "\n";
    for (int i = 0; i < depth; ++i) {
        if (token.stop_requested()) {
            std::cout << name << " received stop signal at iteration " << i << "\n";
            return;
        }
        // Simulate search work
        volatile int work = 0;
        for (int j = 0; j < 100000; ++j) work += j;
    }
    std::cout << name << " finished depth " << depth << "\n";
}

void demo_jthread() {
    std::cout << "\n--- std::jthread & stop_token ---\n";

    // Automatically joins when jthread goes out of scope — no manual join()
    {
        std::jthread worker(searchWorker, 5, "Alpha");
    }  // worker.join() called automatically here
    std::cout << "Alpha worker done (auto-joined)\n";

    // Cooperative cancellation
    std::jthread longWorker(searchWorker, 1000, "Beta");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    longWorker.request_stop();   // signal the worker to stop
    longWorker.join();           // wait for it to acknowledge and exit
    std::cout << "Beta worker cancelled and joined\n";
}


// =============================================================================
//  9. std::bit_cast
//
//  A safe, well-defined way to reinterpret the bits of one type as another.
//  In C++98/11, people used memcpy or pointer casts — both were technically
//  undefined behaviour. bit_cast is constexpr, explicit, and correct.
//
//  Requirements: same sizeof, both types trivially copyable.
// =============================================================================

void demo_bit_cast() {
    std::cout << "\n--- std::bit_cast ---\n";

    // Common use: inspect the bits of a float (IEEE 754 representation)
    float f = 1.0f;
    uint32_t bits = std::bit_cast<uint32_t>(f);
    std::cout << std::format("float 1.0 bits: {:#010x}\n", bits);
    // IEEE 754: sign=0, exponent=127 (0x7F), mantissa=0 -> 0x3F800000

    // Round-trip: bits back to float
    float restored = std::bit_cast<float>(bits);
    std::cout << "restored: " << restored << "\n";

    // Chess: pack two int16 board squares into one int32 for a move
    struct TwoSquares { int16_t from; int16_t to; };
    TwoSquares move{12, 28};
    uint32_t packed = std::bit_cast<uint32_t>(move);
    std::cout << std::format("packed move: {:#010x}\n", packed);
    auto unpacked = std::bit_cast<TwoSquares>(packed);
    std::cout << "unpacked: from=" << unpacked.from << " to=" << unpacked.to << "\n";
}


// =============================================================================
//  10. std::numbers — Mathematical Constants
//
//  Standard constants at the correct precision for any floating-point type.
//  No more defining M_PI yourself or relying on non-standard math.h macros.
// =============================================================================

void demo_numbers() {
    std::cout << "\n--- std::numbers ---\n";

    double  pid = std::numbers::pi;
    float   pif = std::numbers::pi_v<float>;
    double  ed  = std::numbers::e;
    double  sq2 = std::numbers::sqrt2;
    double  phi = std::numbers::phi;   // golden ratio

    std::cout << std::format("pi (double) = {:.15f}\n", pid);
    std::cout << std::format("pi (float)  = {:.7f}\n",  pif);
    std::cout << std::format("e           = {:.15f}\n", ed);
    std::cout << std::format("sqrt(2)     = {:.15f}\n", sq2);
    std::cout << std::format("phi (golden)= {:.15f}\n", phi);
}


// =============================================================================
//  11. std::erase / std::erase_if
//
//  The "erase-remove idiom" from C++98 was notoriously verbose:
//    v.erase(std::remove_if(v.begin(), v.end(), pred), v.end());
//  C++20 adds free functions that do this in one clean call.
// =============================================================================

void demo_erase() {
    std::cout << "\n--- std::erase / erase_if ---\n";

    std::vector<int> pieces = {9, 5, 3, 0, 1, 3, 5, 0, 1};
    std::cout << "Before: ";
    for (int v : pieces) std::cout << v << " ";
    std::cout << "\n";

    // Erase all zeroes (captured/removed pieces)
    std::erase(pieces, 0);
    std::cout << "After erase(0): ";
    for (int v : pieces) std::cout << v << " ";
    std::cout << "\n";

    // Erase all pieces with value < 3
    std::erase_if(pieces, [](int v){ return v < 3; });
    std::cout << "After erase_if(<3): ";
    for (int v : pieces) std::cout << v << " ";
    std::cout << "\n";

    // Works on map too
    std::map<std::string, int> scores = {
        {"Alice",1800}, {"Bob",1200}, {"Carol",1750}, {"Dave",900}
    };
    std::erase_if(scores, [](const auto& kv){ return kv.second < 1500; });
    std::cout << "Players above 1500: ";
    for (const auto& [name, elo] : scores) std::cout << name << " ";
    std::cout << "\n";
}


// =============================================================================
//  MAIN
// =============================================================================

int main() {
    std::cout << "======================================\n";
    std::cout << "  C++20 Feature Showcase\n";
    std::cout << "======================================\n";

    demo_concepts();
    demo_ranges();
    demo_format();
    demo_spaceship();
    demo_span();
    demo_consteval_constinit();
    demo_designated_init();
    demo_jthread();
    demo_bit_cast();
    demo_numbers();
    demo_erase();

    return 0;
}
