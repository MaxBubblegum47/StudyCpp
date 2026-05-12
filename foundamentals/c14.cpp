// =============================================================================
//  C++14 — The Polish Release
//  Compile: g++ -std=c++14 -Wall -o c14 c14.cpp && ./c14
// =============================================================================
//
//  C++14 is a relatively small release — it fixes gaps and rough edges in
//  C++11 rather than introducing paradigm shifts. The changes are real and
//  daily-use useful, but none of them change how you think about the language.
//
//  Covered here:
//    1. Generic lambdas (auto parameters)
//    2. Lambda capture initialisers (init-captures)
//    3. Return type deduction for regular functions
//    4. Relaxed constexpr (multi-statement functions)
//    5. Binary literals & digit separators
//    6. std::make_unique (missing from C++11)
//    7. Variable templates
//    8. [[deprecated]] attribute
//    9. std::integer_sequence (compile-time index tricks)
// =============================================================================

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <utility>
#include <tuple>
#include <type_traits>

// =============================================================================
//  1. GENERIC LAMBDAS
//
//  In C++11 lambda parameters had to have explicit types.
//  C++14 allows "auto" parameters — the lambda becomes a template function.
//  The compiler generates a separate instantiation for each distinct type.
// =============================================================================

void demo_generic_lambdas() {
    std::cout << "\n--- Generic Lambdas ---\n";

    // C++11 lambda — type must be specified
    auto printInt = [](int x) { std::cout << x << "\n"; };
    printInt(42);

    // C++14 generic lambda — works for any printable type
    auto print = [](auto x) { std::cout << x << "\n"; };
    print(42);
    print(3.14);
    print(std::string("any type!"));
    print('A');

    // Generic lambda with multiple auto params
    auto add = [](auto a, auto b) { return a + b; };
    std::cout << "add(3, 4)         = " << add(3, 4)         << "\n";
    std::cout << "add(1.5, 2.5)     = " << add(1.5, 2.5)     << "\n";
    std::cout << "add(string, str)  = " << add(std::string("hello "), std::string("world")) << "\n";

    // Practical use: a generic sorter for any container of comparable elements
    std::vector<int>         ints    = {5, 2, 8, 1, 9, 3};
    std::vector<std::string> strings = {"queen", "rook", "pawn", "bishop"};

    auto sortDesc = [](auto& container) {
        std::sort(container.begin(), container.end(),
                  [](const auto& a, const auto& b){ return a > b; });
    };

    sortDesc(ints);
    sortDesc(strings);

    std::cout << "ints sorted desc: ";
    for (auto v : ints) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "strings sorted desc: ";
    for (const auto& s : strings) std::cout << s << " ";
    std::cout << "\n";
}


// =============================================================================
//  2. LAMBDA CAPTURE INITIALISERS (init-captures)
//
//  C++11 lambdas could only capture existing local variables by value or
//  reference. C++14 lets you introduce NEW variables in the capture list,
//  including moves.
//
//  This was critical for capturing move-only types (like unique_ptr) into
//  lambdas — you couldn't do it in C++11 at all without workarounds.
// =============================================================================

void demo_init_captures() {
    std::cout << "\n--- Lambda Init-Captures ---\n";

    // Move a unique_ptr into a lambda (impossible in C++11)
    auto bigData = std::make_unique<std::vector<int>>(1000, 42);
    std::cout << "Before capture: bigData is "
              << (bigData ? "valid" : "null") << "\n";

    // [moved = std::move(bigData)] creates a new capture variable 'moved'
    // initialised from std::move(bigData). The lambda now owns the data.
    auto worker = [moved = std::move(bigData)]() {
        std::cout << "Lambda owns data, first elem: " << (*moved)[0] << "\n";
    };

    std::cout << "After capture: bigData is "
              << (bigData ? "valid" : "null") << " (moved out)\n";
    worker();

    // Compute a value at capture time (not at lambda call time)
    int count = 5;
    // doubled is computed ONCE when the lambda is created
    auto lambda = [doubled = count * 2]() {
        std::cout << "doubled (computed at capture): " << doubled << "\n";
    };
    count = 999;  // too late — lambda already captured count*2 = 10
    lambda();     // prints 10, not 1998
}


// =============================================================================
//  3. RETURN TYPE DEDUCTION FOR FUNCTIONS
//
//  C++11 required trailing return types for complex cases.
//  C++14 lets you write plain "auto" as the return type and the compiler
//  deduces it from the return statement — just like it does for lambdas.
//
//  Rules:
//   - All return statements must return the same type
//   - Recursive functions need at least one non-recursive return path
//     that the compiler can see first
// =============================================================================

// C++11 style: must spell out the trailing type
// auto describeC11(int n) -> std::string { return n > 0 ? "positive" : "non-positive"; }

// C++14 style: just say auto, compiler deduces std::string
auto describe(int n) {
    if (n > 0) return std::string("positive");
    if (n < 0) return std::string("negative");
    return std::string("zero");
}

// Works for templates too — very useful for generic code
template <typename Container>
auto firstElement(const Container& c) {
    return c.front();   // deduces to Container::value_type
}

void demo_return_deduction() {
    std::cout << "\n--- Return Type Deduction ---\n";
    std::cout << "describe(5)  = " << describe(5)  << "\n";
    std::cout << "describe(-3) = " << describe(-3) << "\n";
    std::cout << "describe(0)  = " << describe(0)  << "\n";

    std::vector<std::string> pieces = {"rook", "bishop", "queen"};
    std::cout << "firstElement(pieces) = " << firstElement(pieces) << "\n";

    std::vector<int> values = {9, 5, 1};
    std::cout << "firstElement(values) = " << firstElement(values) << "\n";
}


// =============================================================================
//  4. RELAXED constexpr
//
//  C++11 constexpr functions had a severe restriction: they could only contain
//  a SINGLE return statement (they were basically limited to ternary chains).
//
//  C++14 relaxes this completely:
//    - Local variable declarations allowed
//    - if/else allowed
//    - loops allowed (for, while, do-while)
//    - Multiple return statements allowed
//
//  The only remaining rule: no static/thread_local variables, no
//  side effects observable outside the function.
// =============================================================================

// C++11 was forced to use a ternary chain for this:
// constexpr int factorial11(int n) { return n <= 1 ? 1 : n * factorial11(n-1); }

// C++14: natural loop, much more readable
constexpr int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i)
        result *= i;
    return result;
}

// Count how many set bits in an integer (popcount)
constexpr int countBits(unsigned int n) {
    int count = 0;
    while (n) {
        count += n & 1;
        n >>= 1;
    }
    return count;
}

// Very useful for chess: bishop covers a diagonal, how many squares?
constexpr int diagonalLength(int file, int rank) {
    int maxDist = 0;
    // Count squares reachable diagonally (simplified)
    for (int d = 1; d < 8; ++d) {
        if (file + d < 8 && rank + d < 8) ++maxDist;
    }
    return maxDist;
}

void demo_relaxed_constexpr() {
    std::cout << "\n--- Relaxed constexpr (C++14) ---\n";

    constexpr int f6 = factorial(6);
    constexpr int f0 = factorial(0);
    std::cout << "factorial(6) at compile time = " << f6 << "\n";
    std::cout << "factorial(0) at compile time = " << f0 << "\n";

    // Popcount: how many 1-bits in the number?
    constexpr int bits = countBits(0b10110101);
    std::cout << "bits in 10110101 = " << bits << "\n";
    static_assert(bits == 5, "should have 5 set bits");

    // Chess: diagonal reachability from e4 (file=4, rank=3)
    constexpr int diagLen = diagonalLength(4, 3);
    std::cout << "Bishop at e4 can reach " << diagLen << " squares NE\n";
}


// =============================================================================
//  5. BINARY LITERALS & DIGIT SEPARATORS
//
//  Small ergonomic additions.
//
//  Binary literals: 0b or 0B prefix. Before C++14 you had to write
//  hex (0xFF) or just trust comments for bit patterns.
//
//  Digit separators: ' (single quote) can appear anywhere inside a numeric
//  literal as a visual separator — it's ignored by the compiler but makes
//  long numbers far more readable.
// =============================================================================

void demo_literals() {
    std::cout << "\n--- Binary Literals & Digit Separators ---\n";

    // Binary literals — great for bitmasks and chess board representations
    unsigned char whitePawns = 0b1111'1111;   // all 8 pawns on rank 2
    unsigned char empty      = 0b0000'0000;

    std::cout << "whitePawns mask = " << (int)whitePawns << "\n";
    std::cout << "empty mask      = " << (int)empty      << "\n";

    // Chess piece existence flags (per-type bitfield)
    int pieces = 0b0000'0001   // pawn
               | 0b0000'0100   // bishop
               | 0b0001'0000;  // queen
    std::cout << "pieces bitmask  = " << pieces << "\n";

    // Digit separators for large numbers — purely visual
    long long millisInYear = 31'536'000'000LL;
    double    avogadro     = 6.022'140'76e23;
    int       hexColor     = 0xFF'AA'BB;

    std::cout << "ms in year:  " << millisInYear << "\n";
    std::cout << "avogadro:    " << avogadro     << "\n";
    std::cout << "hex color:   " << hexColor     << "\n";

    // Elo calculation — easier to read with separators
    int maxElo      = 3'000;
    int centerElo   = 1'500;
    double kFactor  = 32.0;
    std::cout << "Elo range: " << centerElo << " to " << maxElo
              << "  K-factor: " << kFactor << "\n";
}


// =============================================================================
//  6. std::make_unique
//
//  Embarrassingly, C++11 added unique_ptr but forgot make_unique.
//  You had to write: std::unique_ptr<T>(new T(args...))
//  which had an exception-safety issue in certain expressions.
//
//  C++14 adds make_unique for consistency with make_shared.
//  Always prefer make_unique over raw new — it's exception-safe and
//  makes the intent clear.
// =============================================================================

struct ChessPiece {
    std::string name;
    int         value;
    ChessPiece(std::string n, int v) : name(std::move(n)), value(v) {}
    ~ChessPiece() {
        std::cout << "  ~ChessPiece(" << name << ")\n";
    }
};

void demo_make_unique() {
    std::cout << "\n--- std::make_unique ---\n";

    // C++11 workaround (don't do this in C++14+):
    // std::unique_ptr<ChessPiece> p1(new ChessPiece("Queen", 9));

    // C++14 — exception safe, intent clear
    auto queen  = std::make_unique<ChessPiece>("Queen",  9);
    auto rook   = std::make_unique<ChessPiece>("Rook",   5);
    auto bishop = std::make_unique<ChessPiece>("Bishop", 3);

    std::cout << queen->name  << " = " << queen->value  << "\n";
    std::cout << rook->name   << " = " << rook->value   << "\n";
    std::cout << bishop->name << " = " << bishop->value << "\n";

    // make_unique for arrays
    auto board = std::make_unique<int[]>(64);  // 64 ints, zero-initialised
    board[0] = 1; board[63] = 1;
    std::cout << "board[0]=" << board[0] << "  board[63]=" << board[63] << "\n";

    std::cout << "Destructors firing at scope end:\n";
}


// =============================================================================
//  7. VARIABLE TEMPLATES
//
//  Before C++14, only functions and classes could be templated.
//  Variable templates allow parameterising a constant value by type.
//  The canonical use case is mathematical constants at the right precision.
// =============================================================================

template <typename T>
constexpr T pi = T(3.14159265358979323846);

template <typename T>
constexpr T e  = T(2.71828182845904523536);

// Useful for type traits: a variable template version of ::value
template <typename T>
constexpr bool isIntegral = std::is_integral<T>::value;  // C++14 style

void demo_variable_templates() {
    std::cout << "\n--- Variable Templates ---\n";

    // Same constant, different precision
    float  pif = pi<float>;
    double pid = pi<double>;
    std::cout << "pi<float>  = " << pif << "\n";
    std::cout << "pi<double> = " << pid << "\n";

    // isIntegral<T> is cleaner than std::is_integral<T>::value
    std::cout << "isIntegral<int>    = " << std::boolalpha << isIntegral<int>    << "\n";
    std::cout << "isIntegral<double> = " << isIntegral<double> << "\n";
    std::cout << "isIntegral<char>   = " << isIntegral<char>   << "\n";
}


// =============================================================================
//  8. [[deprecated]] ATTRIBUTE
//
//  A standard way to mark APIs as deprecated. The compiler will emit a
//  warning (not an error) when deprecated code is used. Previously you had
//  to use compiler-specific extensions (__attribute__((deprecated)) on GCC,
//  __declspec(deprecated) on MSVC).
// =============================================================================

[[deprecated("Use computeMaterialScore() instead")]]
int countMaterial(const std::vector<int>& /*pieces*/) {
    return 0;
}

int computeMaterialScore(const std::vector<int>& pieces) {
    int total = 0;
    for (int v : pieces) total += v;
    return total;
}

void demo_deprecated() {
    std::cout << "\n--- [[deprecated]] attribute ---\n";

    std::vector<int> myPieces = {9, 5, 5, 3, 3, 1, 1, 1, 1};

    // Using the deprecated function will generate a compiler warning like:
    // warning: 'countMaterial' is deprecated: Use computeMaterialScore() instead
    // countMaterial(myPieces);  // <- uncomment to see the warning

    int score = computeMaterialScore(myPieces);
    std::cout << "Material score: " << score << "\n";
    std::cout << "(deprecated function commented out to suppress warning)\n";
}


// =============================================================================
//  MAIN
// =============================================================================

int main() {
    std::cout << "======================================\n";
    std::cout << "  C++14 Feature Showcase\n";
    std::cout << "======================================\n";

    demo_generic_lambdas();
    demo_init_captures();
    demo_return_deduction();
    demo_relaxed_constexpr();
    demo_literals();
    demo_make_unique();
    demo_variable_templates();
    demo_deprecated();

    return 0;
}
