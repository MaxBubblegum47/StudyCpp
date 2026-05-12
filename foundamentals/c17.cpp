// =============================================================================
//  C++17 — The Big Vocabulary Release
//  Compile: g++ -std=c++17 -Wall -o c17 c17.cpp && ./c17
// =============================================================================
//
//  C++17 is a significant release — not a revolution like C++11, but it added
//  several features that immediately changed how good C++ looks and reads.
//  Structured bindings and optional/variant became the idiomatic way to
//  express "return multiple values" and "nullable types" in modern APIs.
//
//  Covered here:
//    1.  Structured bindings
//    2.  if / switch with initialiser
//    3.  std::optional<T>
//    4.  std::variant<Ts...>
//    5.  std::any
//    6.  std::string_view
//    7.  if constexpr
//    8.  Fold expressions
//    9.  Class Template Argument Deduction (CTAD)
//   10.  Parallel algorithms (execution policies)
//   11.  std::filesystem
//   12.  Inline variables
//   13.  Guaranteed copy elision (RVO)
//   14.  std::invoke / std::apply
// =============================================================================

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <optional>
#include <variant>
#include <any>
#include <tuple>
#include <algorithm>
#include <execution>
#include <filesystem>
#include <functional>
#include <type_traits>
#include <numeric>

#include <fstream>
namespace fs = std::filesystem;

// =============================================================================
//  1. STRUCTURED BINDINGS
//
//  Structured bindings let you "unpack" a pair, tuple, struct, or array into
//  named variables in a single declaration. No more .first/.second everywhere.
//  They work on:
//    - std::pair / std::tuple
//    - Any struct with public members (aggregate types)
//    - Arrays (C arrays and std::array)
//    - Any type with a custom get<N>() specialisation
// =============================================================================

struct MoveRecord {
    std::string notation;
    int         fromSquare;
    int         toSquare;
    bool        isCapture;
};

std::pair<int, int> squareToCoords(int square) {
    return {square % 8, square / 8};  // {file, rank}
}

std::tuple<double, double, int> getStats(const std::vector<int>& values) {
    double sum = 0;
    for (int v : values) sum += v;
    double mean = sum / values.size();
    int    count = static_cast<int>(values.size());
    return {mean, sum, count};
}

void demo_structured_bindings() {
    std::cout << "\n--- Structured Bindings ---\n";

    // Unpack a std::pair
    auto [file, rank] = squareToCoords(27);   // d4 = square 27
    std::cout << "Square 27 = file " << file << ", rank " << rank << "\n";

    // Unpack a std::tuple
    std::vector<int> scores = {1, 3, 3, 5, 9};
    auto [mean, total, count] = getStats(scores);
    std::cout << "mean=" << mean << "  total=" << total << "  count=" << count << "\n";

    // Unpack a struct (aggregate — only works for public members, no constructors)
    // This works because MoveRecord is an aggregate type
    MoveRecord mv{"e4", 12, 28, false};
    auto& [notation, from, to, capture] = mv;   // reference binding!
    std::cout << "Move: " << notation << " (" << from << " -> " << to << ")\n";
    notation = "d4";   // modifies mv.notation through the reference
    std::cout << "Updated notation in original: " << mv.notation << "\n";

    // Unpack in a range-for over a map — the killer use case
    std::map<std::string, int> pieceValues = {
        {"pawn",1}, {"knight",3}, {"bishop",3}, {"rook",5}, {"queen",9}
    };
    std::cout << "Piece values:\n";
    for (const auto& [name, value] : pieceValues) {
        std::cout << "  " << name << " = " << value << "\n";
    }
}


// =============================================================================
//  2. if / switch WITH INITIALISER
//
//  You can now declare and initialise a variable inside an if or switch
//  condition. The variable is scoped to the entire if/else chain — it doesn't
//  leak into the surrounding scope.
//
//  This pattern is particularly useful when:
//    - You lock a mutex and want it held for the if body but not after
//    - You do a map lookup and want the iterator only in the if block
//    - You parse a value and want the result only if parsing succeeded
// =============================================================================

std::optional<int> findPieceValue(const std::string& name) {
    static const std::map<std::string, int> table = {
        {"pawn",1}, {"knight",3}, {"bishop",3}, {"rook",5}, {"queen",9}
    };
    auto it = table.find(name);
    if (it != table.end()) return it->second;
    return std::nullopt;
}

void demo_if_init() {
    std::cout << "\n--- if / switch with initialiser ---\n";

    std::map<std::string, int> scores = {{"Alice", 1800}, {"Bob", 1650}};

    // if-init: 'it' is scoped to the if/else — not visible after
    if (auto it = scores.find("Alice"); it != scores.end()) {
        std::cout << "Alice's rating: " << it->second << "\n";
    } else {
        std::cout << "Alice not found\n";
    }
    // 'it' is not accessible here — clean scope

    // switch-init: extremely useful for error codes
    enum class ParseResult { Ok, BadFormat, OutOfRange };
    auto parseSquare = [](const std::string& s) -> ParseResult {
        if (s.size() != 2)        return ParseResult::BadFormat;
        if (s[0] < 'a' || s[0] > 'h') return ParseResult::OutOfRange;
        if (s[1] < '1' || s[1] > '8') return ParseResult::OutOfRange;
        return ParseResult::Ok;
    };

    switch (auto result = parseSquare("e4"); result) {
        case ParseResult::Ok:        std::cout << "Square 'e4' is valid\n";       break;
        case ParseResult::BadFormat: std::cout << "Bad format\n";                 break;
        case ParseResult::OutOfRange:std::cout << "Square out of range\n";        break;
    }

    switch (auto result = parseSquare("z9"); result) {
        case ParseResult::Ok:         std::cout << "Valid\n";        break;
        case ParseResult::OutOfRange: std::cout << "'z9' is out of range\n"; break;
        default:                      std::cout << "Bad format\n";   break;
    }
}


// =============================================================================
//  3. std::optional<T>
//
//  std::optional<T> holds either a value of type T or nothing (std::nullopt).
//  It's the clean, explicit alternative to:
//    - Returning -1 or 0 as a sentinel (magic numbers)
//    - Returning a pointer (implies heap allocation and lifetime questions)
//    - Using an output parameter + bool return
//
//  The value is stored INLINE inside the optional — no heap allocation.
//  optional<int> is typically 8 bytes: 4 for the int + 1 byte flag + padding.
// =============================================================================

// Clear intent: "this might not find anything"
std::optional<std::string> openingName(const std::string& firstMove) {
    if (firstMove == "e4") return "King's Pawn";
    if (firstMove == "d4") return "Queen's Pawn";
    if (firstMove == "c4") return "English Opening";
    return std::nullopt;  // not in our database
}

// Chain of optional operations — if any step fails, propagate nullopt
std::optional<int> safeDiv(int a, int b) {
    if (b == 0) return std::nullopt;
    return a / b;
}

void demo_optional() {
    std::cout << "\n--- std::optional ---\n";

    // Basic usage
    auto name = openingName("e4");
    if (name) {   // bool conversion: true if has value
        std::cout << "Opening: " << *name << "\n";  // dereference like a pointer
    }

    auto unknown = openingName("b4");
    std::cout << "b4 known? " << std::boolalpha << unknown.has_value() << "\n";

    // value_or — provide a default if empty
    std::cout << "Name or default: " << unknown.value_or("Unknown opening") << "\n";

    // optional in a range-for via transform
    std::vector<std::string> moves = {"e4", "b4", "d4", "h4"};
    std::cout << "Named openings:\n";
    for (const auto& m : moves) {
        auto n = openingName(m);
        std::cout << "  " << m << " -> " << n.value_or("?") << "\n";
    }

    // Safe arithmetic chains
    auto result = safeDiv(10, 0);
    std::cout << "10/0 = " << (result ? std::to_string(*result) : "undefined") << "\n";
    std::cout << "10/2 = " << *safeDiv(10, 2) << "\n";
}


// =============================================================================
//  4. std::variant<Ts...>
//
//  A type-safe union. It holds exactly one value of one of the listed types.
//  Unlike a C union, it knows at runtime which type it currently holds, and
//  accessing the wrong type throws std::bad_variant_access.
//
//  The killer feature: std::visit — dispatch a visitor (usually an overloaded
//  lambda set) to the currently-held type. This is essentially pattern matching.
//
//  Common uses:
//    - Representing algebraic data types (like Rust's enum)
//    - Error-or-value (before std::expected was added in C++23)
//    - AST nodes in parsers/compilers
//    - Chess: different event types (Move, Capture, Castle, Promotion)
// =============================================================================

// Chess event types
struct Move     { int from; int to; };
struct Capture  { int from; int to; std::string capturedPiece; };
struct Castle   { bool kingside; };
struct Promotion{ int from; int to; char newPiece; };

// ChessEvent can hold exactly ONE of these types
using ChessEvent = std::variant<Move, Capture, Castle, Promotion>;

// An "overloaded" visitor — C++17 trick using variadic inheritance
// This lets you pass multiple lambdas to std::visit
template <typename... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};
// Deduction guide — tells the compiler how to deduce Overloaded's template args
template <typename... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

void demo_variant() {
    std::cout << "\n--- std::variant ---\n";

    std::vector<ChessEvent> events = {
        Move{12, 28},
        Capture{28, 36, "pawn"},
        Castle{true},
        Promotion{52, 60, 'Q'}
    };

    std::cout << "Game events:\n";
    for (const auto& event : events) {
        // std::visit dispatches to the right lambda based on active type
        std::visit(Overloaded{
            [](const Move& m) {
                std::cout << "  Move: " << m.from << " -> " << m.to << "\n";
            },
            [](const Capture& c) {
                std::cout << "  Capture " << c.capturedPiece
                          << " at " << c.to << "\n";
            },
            [](const Castle& c) {
                std::cout << "  Castle " << (c.kingside ? "kingside" : "queenside") << "\n";
            },
            [](const Promotion& p) {
                std::cout << "  Promotion at " << p.to
                          << " to " << p.newPiece << "\n";
            }
        }, event);
    }

    // Alternative: std::get<T> when you know the type
    ChessEvent known = Castle{false};
    if (std::holds_alternative<Castle>(known)) {
        auto& c = std::get<Castle>(known);
        std::cout << "It's a " << (c.kingside ? "kingside" : "queenside") << " castle\n";
    }

    // std::get_if — non-throwing, returns pointer or nullptr
    if (auto* m = std::get_if<Move>(&known)) {
        std::cout << "It's a move: " << m->from << "\n";
    } else {
        std::cout << "Not a Move\n";
    }
}


// =============================================================================
//  5. std::any
//
//  A type-safe container for a single value of any type.
//  Unlike void*, it remembers what type it holds and throws if you try
//  to retrieve the wrong type.
//  Use when you genuinely don't know the type at compile time — but prefer
//  variant when you know the set of possible types.
// =============================================================================

void demo_any() {
    std::cout << "\n--- std::any ---\n";

    std::any val;
    std::cout << "Empty any: has_value=" << std::boolalpha << val.has_value() << "\n";

    val = 42;
    std::cout << "Holds int: " << std::any_cast<int>(val) << "\n";

    val = std::string("knight");
    std::cout << "Holds string: " << std::any_cast<std::string>(val) << "\n";

    val = 3.14;
    std::cout << "Holds double: " << std::any_cast<double>(val) << "\n";

    try {
        std::any_cast<int>(val);   // val holds double, not int
    } catch (const std::bad_any_cast& e) {
        std::cout << "Wrong cast caught: " << e.what() << "\n";
    }

    // A heterogeneous property bag — useful for dynamic config/metadata
    std::map<std::string, std::any> pieceProps;
    pieceProps["name"]    = std::string("Queen");
    pieceProps["value"]   = 9;
    pieceProps["sliding"] = true;
    std::cout << "name: "    << std::any_cast<std::string>(pieceProps["name"])  << "\n";
    std::cout << "value: "   << std::any_cast<int>(pieceProps["value"])         << "\n";
    std::cout << "sliding: " << std::any_cast<bool>(pieceProps["sliding"])      << "\n";
}


// =============================================================================
//  6. std::string_view
//
//  A non-owning, read-only view into a string. It's a (pointer, length) pair.
//  Passing string_view to a function that only reads the string:
//    - Avoids copying a std::string (like const std::string& does)
//    - BUT also works for C strings, string literals, substrings, and
//      any other contiguous char sequence without conversion or allocation.
//
//  Rules:
//    - The underlying data must outlive the view (it's non-owning!)
//    - Do NOT store a string_view in a struct/class unless you're sure
//      the lifetime is managed correctly
//    - DO use as function parameters that only read strings
// =============================================================================

// Takes any kind of string without copying or allocating
bool isValidMove(std::string_view notation) {
    if (notation.size() < 2 || notation.size() > 5) return false;
    // A basic sanity check — file a-h, rank 1-8
    return notation[0] >= 'a' && notation[0] <= 'h' &&
           notation[1] >= '1' && notation[1] <= '8';
}

std::string_view pieceSymbol(std::string_view name) {
    if (name == "king")   return "♔";
    if (name == "queen")  return "♕";
    if (name == "rook")   return "♖";
    if (name == "bishop") return "♗";
    if (name == "knight") return "♘";
    return "♙";
}

void demo_string_view() {
    std::cout << "\n--- std::string_view ---\n";

    // Works with string literals — no std::string constructed
    std::cout << "e4 valid? " << std::boolalpha << isValidMove("e4")   << "\n";
    std::cout << "z9 valid? " << isValidMove("z9") << "\n";

    // Works with std::string — no copy
    std::string move = "Nf3";
    std::cout << "Nf3 valid? " << isValidMove(move) << "\n";

    // Substring view — no allocation!
    std::string pgn = "1. e4 e5 2. Nf3 Nc6 3. Bb5";
    std::string_view view = pgn;
    std::string_view firstMove = view.substr(3, 2);   // "e4"
    std::cout << "First move from PGN: " << firstMove << "\n";

    // Symbols
    for (const auto& piece : {"king","queen","rook","bishop","knight","pawn"}) {
        std::cout << pieceSymbol(piece) << " ";
    }
    std::cout << "\n";
}


// =============================================================================
//  7. if constexpr
//
//  "Compile-time if" — the condition is evaluated at compile time, and ONLY
//  the chosen branch is compiled. The other branch is discarded entirely.
//
//  This replaces most uses of template specialisation and enable_if when
//  you want different behaviour based on type properties. The discarded
//  branch doesn't even need to be syntactically valid for the types involved.
// =============================================================================

// A single function that handles any numeric type — different behaviour at
// compile time based on whether it's integral or floating-point
template <typename T>
void printTyped(T value) {
    if constexpr (std::is_integral_v<T>) {
        // This branch only compiled for integral T
        std::cout << "integral: " << value
                  << " (binary: " << std::hex << value << std::dec << ")\n";
    } else if constexpr (std::is_floating_point_v<T>) {
        // This branch only compiled for floating-point T
        std::cout << "float: " << value
                  << " (rounded: " << static_cast<long long>(value) << ")\n";
    } else {
        std::cout << "other: " << value << "\n";
    }
}

void demo_if_constexpr() {
    std::cout << "\n--- if constexpr ---\n";

    printTyped(255);        // integral path
    printTyped(3.14159);    // float path
    printTyped('A');        // integral path (char is integral)
    printTyped(std::string("queen"));  // other path
}


// =============================================================================
//  8. FOLD EXPRESSIONS
//
//  A concise way to apply a binary operator to all elements of a parameter
//  pack. Eliminates the recursive template boilerplate needed in C++11/14.
//
//  Four forms:
//    (pack op ...)       — right fold:  a0 op (a1 op (a2 op ...))
//    (... op pack)       — left fold:   (... (a0 op a1) op a2) op ...
//    (pack op ... op I)  — right fold with initial value I
//    (I op ... op pack)  — left fold  with initial value I
// =============================================================================

// Sum: left fold over +
template <typename... Args>
auto sum(Args... args) {
    return (args + ...);       // left fold: ((a0 + a1) + a2) + ...
}

// All-true: fold over &&
template <typename... Args>
bool allPositive(Args... args) {
    return ((args > 0) && ...);
}

// Print all args separated by spaces: fold over comma operator  
template <typename... Args>
void printAll(Args... args) {
    ((std::cout << args << " "), ...);
    std::cout << "\n";
}

// Count args whose value is above a threshold
template <typename T, typename... Args>
int countAbove(T threshold, Args... args) {
    return ((args > threshold ? 1 : 0) + ...);
}

void demo_fold_expressions() {
    std::cout << "\n--- Fold Expressions ---\n";

    std::cout << "sum(1,2,3,4,5)      = " << sum(1,2,3,4,5)           << "\n";
    std::cout << "sum(1.0,2.5,3.5)    = " << sum(1.0, 2.5, 3.5)       << "\n";
    std::cout << "allPositive(1,2,3)  = " << std::boolalpha << allPositive(1,2,3) << "\n";
    std::cout << "allPositive(1,-1,3) = " << allPositive(1,-1,3)       << "\n";

    std::cout << "printAll: ";
    printAll("e4", "e5", "Nf3", "Nc6");

    std::cout << "countAbove(3, 1,2,3,4,5,9) = "
              << countAbove(3, 1,2,3,4,5,9) << "\n";
}


// =============================================================================
//  9. CLASS TEMPLATE ARGUMENT DEDUCTION (CTAD)
//
//  Before C++17, you needed to specify template arguments explicitly when
//  constructing a class template, or use a make_ helper function.
//  CTAD lets the compiler deduce them from the constructor arguments,
//  just like function templates have always done.
// =============================================================================

void demo_ctad() {
    std::cout << "\n--- Class Template Argument Deduction (CTAD) ---\n";

    // C++14 and earlier required:
    // std::pair<std::string, int> p1("queen", 9);
    // std::vector<int> v1 = {1,2,3};
    // or: auto p1 = std::make_pair("queen", 9);

    // C++17 CTAD — types deduced from constructor arguments:
    std::pair  p("queen", 9);           // std::pair<const char*, int>
    std::tuple t("rook", 5, true);      // std::tuple<const char*, int, bool>
    std::vector v{1, 2, 3, 4, 5};       // std::vector<int>

    std::cout << "pair:   " << p.first << " = " << p.second     << "\n";
    std::cout << "tuple:  " << std::get<0>(t) << " = " << std::get<1>(t) << "\n";
    std::cout << "vector: ";
    for (int x : v) std::cout << x << " ";
    std::cout << "\n";

    // Very useful with lock_guard
    // std::mutex m;
    // std::lock_guard lock(m);  // no need to write lock_guard<std::mutex>
}


// =============================================================================
//  10. std::filesystem
//
//  A portable, type-safe API for file and directory operations.
//  Previously you needed OS-specific APIs or Boost.Filesystem.
//  Now standard: enumerate directories, query file sizes, copy/move files, etc.
// =============================================================================

void demo_filesystem() {
    std::cout << "\n--- std::filesystem ---\n";

    fs::path current = fs::current_path();
    std::cout << "Current directory: " << current << "\n";

    // Create a temp directory for demo
    fs::path tmpDir = fs::temp_directory_path() / "cpp17_demo";
    fs::create_directories(tmpDir);
    std::cout << "Temp dir: " << tmpDir << "\n";

    // Write a file
    std::ofstream f(tmpDir / "test.txt");
    f << "hello from C++17 filesystem\n";
    f.close();

    // Query properties
    fs::path file = tmpDir / "test.txt";
    std::cout << "exists:    " << std::boolalpha << fs::exists(file)     << "\n";
    std::cout << "is_file:   " << fs::is_regular_file(file)              << "\n";
    std::cout << "file_size: " << fs::file_size(file) << " bytes\n";

    // Iterate directory
    std::cout << "Files in temp dir:\n";
    for (const auto& entry : fs::directory_iterator(tmpDir)) {
        std::cout << "  " << entry.path().filename() << "\n";
    }

    // Cleanup
    fs::remove_all(tmpDir);
    std::cout << "Temp dir removed. exists=" << fs::exists(tmpDir) << "\n";
}


// =============================================================================
//  MAIN
// =============================================================================

int main() {
    std::cout << "======================================\n";
    std::cout << "  C++17 Feature Showcase\n";
    std::cout << "======================================\n";

    demo_structured_bindings();
    demo_if_init();
    demo_optional();
    demo_variant();
    demo_any();
    demo_string_view();
    demo_if_constexpr();
    demo_fold_expressions();
    demo_ctad();
    demo_filesystem();

    return 0;
}
