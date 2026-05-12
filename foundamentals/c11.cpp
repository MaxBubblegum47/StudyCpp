// =============================================================================
//  C++11 — The Revolution
//  Compile: g++ -std=c++11 -Wall -o c11 c11.cpp && ./c11
// =============================================================================
//
//  C++11 is the dividing line between "old C++" and "modern C++".
//  It took 13 years after C++98 but delivered an enormous set of features.
//  Herb Sutter called it "a completely different language than C++98."
//
//  Covered here:
//    1.  auto & decltype
//    2.  Range-based for
//    3.  Lambda expressions
//    4.  nullptr
//    5.  Move semantics & rvalue references
//    6.  Smart pointers (unique_ptr, shared_ptr, weak_ptr)
//    7.  constexpr
//    8.  enum class
//    9.  override & final
//   10.  Initializer lists & uniform initialisation
//   11.  Variadic templates
//   12.  std::thread & std::mutex
//   13.  std::tuple & std::pair improvements
//   14.  New STL containers: unordered_map, array
//   15.  static_assert
//   16.  noexcept
//   17.  std::chrono
// =============================================================================

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <array>
#include <tuple>
#include <memory>       // unique_ptr, shared_ptr, weak_ptr
#include <functional>   // std::function
#include <algorithm>
#include <thread>
#include <mutex>
#include <chrono>
#include <cassert>
#include <type_traits>  // std::is_integral, etc.
#include <utility>      // std::move, std::forward

// =============================================================================
//  1. auto & decltype
//
//  auto tells the compiler to deduce the type from the initialiser.
//  It doesn't mean "dynamic typing" — the type is still fixed at compile time.
//  It just saves you from writing it out, especially for long iterator types.
//
//  decltype(expr) gives you the type of an expression, without evaluating it.
//  Mainly useful in templates and trailing return types.
// =============================================================================

void demo_auto() {
    std::cout << "\n--- auto & decltype ---\n";

    auto x        = 42;           // int
    auto pi_val   = 3.14159;      // double (named pi_val to avoid clash)
    auto greeting = std::string("hello");  // std::string

    // Without auto you'd write: std::map<std::string,int>::iterator
    std::map<std::string, int> scores = {{"Alice", 1800}, {"Bob", 1650}};
    auto it = scores.begin();     // std::map<std::string,int>::iterator
    std::cout << "First player: " << it->first << " (" << it->second << ")\n";

    // decltype — get the type of an expression
    decltype(x) y = 100;          // y is int, same type as x
    std::cout << "x=" << x << "  y(decltype(x))=" << y << "\n";

    // Trailing return type with auto + decltype (useful in templates)
    // (more commonly seen in C++14 which made this simpler)
    auto add = [](int a, int b) -> decltype(a + b) { return a + b; };
    std::cout << "add(3,4) = " << add(3, 4) << "\n";
}


// =============================================================================
//  2. RANGE-BASED FOR
//
//  A clean syntax for iterating over anything that has begin()/end().
//  Works on arrays, STL containers, and any custom type you give
//  those two functions.
//
//  Always prefer:
//    for (const auto& elem : container)  when you don't modify elements
//    for (auto& elem : container)        when you do modify them
//    for (auto elem : container)         only for cheap-to-copy types (int, char)
// =============================================================================

void demo_range_for() {
    std::cout << "\n--- Range-based for ---\n";

    std::vector<std::string> openings = {"Ruy Lopez", "Sicilian", "King's Indian"};

    // Read-only iteration — const& avoids copying each string
    for (const auto& opening : openings) {
        std::cout << "  " << opening << "\n";
    }

    // Modify in place
    for (auto& opening : openings) {
        opening += " Defense";
    }
    std::cout << "After modification:\n";
    for (const auto& opening : openings) {
        std::cout << "  " << opening << "\n";
    }

    // Works on C arrays too
    int ranks[] = {1, 2, 3, 4, 5, 6, 7, 8};
    int sum = 0;
    for (int r : ranks) sum += r;
    std::cout << "Sum of ranks 1-8: " << sum << "\n";
}


// =============================================================================
//  3. LAMBDA EXPRESSIONS
//
//  A lambda is an anonymous function defined inline. It's essentially syntactic
//  sugar for a class with operator(). Captures let you "close over" local
//  variables from the enclosing scope.
//
//  Capture modes:
//    []        — captures nothing
//    [=]       — captures all locals by value (copies)
//    [&]       — captures all locals by reference
//    [x]       — captures only x by value
//    [&x]      — captures only x by reference
//    [=, &x]   — all by value, but x by reference
// =============================================================================

void demo_lambdas() {
    std::cout << "\n--- Lambda Expressions ---\n";

    // Simple lambda stored in auto
    auto square = [](int n) { return n * n; };
    std::cout << "square(7) = " << square(7) << "\n";

    // Lambda capturing by value
    int threshold = 5;
    auto aboveThreshold = [threshold](int x) { return x > threshold; };
    std::cout << "4 > threshold(5)? " << std::boolalpha << aboveThreshold(4) << "\n";
    std::cout << "6 > threshold(5)? " << aboveThreshold(6) << "\n";

    // Lambda capturing by reference — can modify the outer variable
    int moveCount = 0;
    auto recordMove = [&moveCount](const std::string& /*m*/) {
        ++moveCount;
    };
    recordMove("e4"); recordMove("e5"); recordMove("Nf3");
    std::cout << "Moves recorded: " << moveCount << "\n";

    // Lambdas with STL algorithms — replaces functor boilerplate from C++98
    std::vector<int> values = {9, 1, 5, 3, 7, 2, 8, 4, 6};
    std::sort(values.begin(), values.end(), [](int a, int b){ return a < b; });
    std::cout << "Sorted: ";
    for (int v : values) std::cout << v << " ";
    std::cout << "\n";

    // Count pieces above a certain value using count_if + lambda
    std::vector<int> pieceValues = {1, 3, 3, 5, 9, 0};
    int highValue = std::count_if(pieceValues.begin(), pieceValues.end(),
                                  [](int v){ return v >= 5; });
    std::cout << "Pieces with value >= 5: " << highValue << "\n";

    // std::function — type-erased wrapper, can store any callable
    // (lambdas, function pointers, functors)
    std::function<bool(int,int)> compare = [](int a, int b){ return a < b; };
    std::cout << "compare(3, 5): " << compare(3, 5) << "\n";
}


// =============================================================================
//  4. nullptr
//
//  In C++98, NULL was typically defined as 0 (an int), which caused
//  overload resolution ambiguities:
//    void foo(int);
//    void foo(int*);
//    foo(NULL);   // which overload? depends on compiler!
//
//  nullptr is a keyword of type std::nullptr_t. It converts to any pointer
//  type but NOT to int, so overload resolution is always unambiguous.
// =============================================================================

void foo(int x)  { std::cout << "foo(int) called with " << x << "\n"; }
void foo(int* p) { std::cout << "foo(int*) called, ptr is "
                              << (p ? "non-null" : "null") << "\n"; }

void demo_nullptr() {
    std::cout << "\n--- nullptr ---\n";

    int* p = nullptr;           // clearly a null pointer, not 0
    std::cout << "p is " << (p == nullptr ? "null" : "non-null") << "\n";

    foo(0);       // calls foo(int)   — fine
    foo(nullptr); // calls foo(int*) — unambiguous with nullptr
    // foo(NULL); would be ambiguous or call foo(int) depending on compiler
}


// =============================================================================
//  5. MOVE SEMANTICS & RVALUE REFERENCES
//
//  This is the most important performance feature of C++11.
//
//  In C++98, passing or returning objects by value always copied them.
//  For a vector of 1 million ints that's expensive — even when the source
//  is a temporary that's about to be destroyed anyway.
//
//  C++11 introduces rvalue references (T&&), which bind to temporaries
//  (things without a name, or things you explicitly cast with std::move).
//  A move constructor/assignment "steals" the resource from the source
//  instead of copying it, leaving the source in a valid-but-empty state.
//
//  T&  — lvalue reference: binds to named objects (lvalues)
//  T&& — rvalue reference: binds to temporaries / std::move'd objects
// =============================================================================

class Buffer {
public:
    explicit Buffer(int size)
        : size_(size), data_(new int[size]) {
        std::cout << "  Buffer(" << size << "): allocated\n";
    }

    // Destructor
    ~Buffer() {
        delete[] data_;
        std::cout << "  ~Buffer: freed (size was " << size_ << ")\n";
    }

    // Copy constructor — expensive: copies every element
    Buffer(const Buffer& other)
        : size_(other.size_), data_(new int[other.size_]) {
        for (int i = 0; i < size_; ++i) data_[i] = other.data_[i];
        std::cout << "  Buffer(copy): copied " << size_ << " elements\n";
    }

    // Move constructor — cheap: just steal the pointer
    Buffer(Buffer&& other) noexcept
        : size_(other.size_), data_(other.data_) {
        other.data_ = nullptr;   // leave source in valid empty state
        other.size_ = 0;
        std::cout << "  Buffer(move): pointer stolen — no copy!\n";
    }

    // Move assignment
    Buffer& operator=(Buffer&& other) noexcept {
        if (this == &other) return *this;
        delete[] data_;
        data_       = other.data_;
        size_       = other.size_;
        other.data_ = nullptr;
        other.size_ = 0;
        std::cout << "  Buffer(move=): pointer stolen\n";
        return *this;
    }

    int size() const { return size_; }

private:
    int  size_;
    int* data_;
};

Buffer makeBuffer(int n) {
    Buffer b(n);           // constructs locally
    return b;              // NRVO may elide the copy/move entirely,
                           // but if not, the move constructor is used
}

void demo_move() {
    std::cout << "\n--- Move Semantics ---\n";

    std::cout << "Creating b1:\n";
    Buffer b1(10);

    std::cout << "Copying b1 -> b2 (expensive):\n";
    Buffer b2(b1);

    std::cout << "Moving b1 -> b3 (cheap, using std::move):\n";
    Buffer b3(std::move(b1));
    // After move: b1.data_ == nullptr, b1.size_ == 0
    // b1 is still destructable (no crash) but "empty"
    std::cout << "  b3.size()=" << b3.size()
              << "  b1.size()=" << b1.size() << " (drained)\n";

    std::cout << "Function returning Buffer (move/NRVO):\n";
    Buffer b4 = makeBuffer(5);
    std::cout << "  b4.size()=" << b4.size() << "\n";

    std::cout << "Destructors firing:\n";
}


// =============================================================================
//  6. SMART POINTERS
//
//  Raw owning pointers (new/delete) are error-prone: you can forget to
//  delete, delete twice, or throw before reaching delete.
//  Smart pointers wrap raw pointers and tie lifetime to scope (RAII).
//
//  unique_ptr — sole owner. Not copyable, but moveable. Zero overhead.
//  shared_ptr — shared ownership via reference counting. Slightly more
//               overhead. Cyclic references leak (use weak_ptr to break cycles).
//  weak_ptr   — non-owning observer of a shared_ptr. Doesn't affect ref count.
// =============================================================================

struct Node {
    int value;
    std::shared_ptr<Node> next;
    Node(int v) : value(v) {}
    ~Node() { std::cout << "  ~Node(" << value << ")\n"; }
};

void demo_smart_pointers() {
    std::cout << "\n--- Smart Pointers ---\n";

    // unique_ptr: single owner, destroyed when it goes out of scope
    {
        std::unique_ptr<std::string> p(new std::string("unique owner"));
        std::cout << "unique_ptr holds: " << *p << "\n";
        // auto p2 = p;  // ERROR: unique_ptr is not copyable
        auto p2 = std::move(p);  // transfer ownership
        std::cout << "After move: p is " << (p ? "valid" : "null")
                  << ", p2 holds: " << *p2 << "\n";
    }  // p2 goes out of scope here -> string is deleted automatically
    std::cout << "unique_ptr destroyed at end of block\n";

    // shared_ptr: multiple owners, deleted when last owner dies
    {
        std::cout << "\nshared_ptr nodes:\n";
        auto n1 = std::make_shared<Node>(1);
        auto n2 = std::make_shared<Node>(2);
        n1->next = n2;   // n2 now has ref count 2 (n2 + n1->next)

        std::cout << "  n2 use_count=" << n2.use_count() << "\n";  // 2
        {
            auto n3 = n2;   // copy increases ref count
            std::cout << "  n2 use_count inside inner block=" << n2.use_count() << "\n"; // 3
        }  // n3 destroyed, ref count drops to 2
        std::cout << "  n2 use_count after inner block=" << n2.use_count() << "\n"; // 2
    }  // n1 and n2 both destroyed — Nodes freed in reverse order
    std::cout << "shared_ptr chain destroyed\n";

    // weak_ptr: observe without owning — won't prevent destruction
    std::cout << "\nweak_ptr:\n";
    std::weak_ptr<Node> weak;
    {
        auto sp = std::make_shared<Node>(99);
        weak = sp;
        std::cout << "  weak expired? " << std::boolalpha << weak.expired() << "\n";
        if (auto locked = weak.lock()) {   // lock() returns shared_ptr or null
            std::cout << "  locked value: " << locked->value << "\n";
        }
    }  // sp destroyed
    std::cout << "  weak expired after sp destroyed? " << weak.expired() << "\n";
}


// =============================================================================
//  7. constexpr
//
//  constexpr marks a function or variable as "computable at compile time".
//  If the arguments are compile-time constants, the result is computed by
//  the compiler and embedded in the binary — zero runtime cost.
//  If the arguments are runtime values, it falls back to a normal function call.
//
//  In C++11, constexpr functions had strict restrictions (single return stmt).
//  C++14 relaxed these significantly.
// =============================================================================

constexpr int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}

constexpr int fibonacciIndex(int n) {
    return n <= 1 ? n : fibonacciIndex(n-1) + fibonacciIndex(n-2);
}

// constexpr variable — computed at compile time
constexpr int BOARD_SIZE    = 8;
constexpr int SQUARES       = BOARD_SIZE * BOARD_SIZE;  // 64
constexpr int TOTAL_PIECES  = 32;

void demo_constexpr() {
    std::cout << "\n--- constexpr ---\n";

    // Computed at compile time — factorial<5> is literally 120 in the binary
    constexpr int f5 = factorial(5);
    std::cout << "factorial(5) at compile time = " << f5 << "\n";

    // static_assert checks at compile time — zero runtime overhead
    static_assert(factorial(5) == 120,  "factorial(5) must be 120");
    static_assert(SQUARES      == 64,   "board must have 64 squares");

    std::cout << "BOARD_SIZE=" << BOARD_SIZE
              << "  SQUARES=" << SQUARES
              << "  TOTAL_PIECES=" << TOTAL_PIECES << "\n";

    // Can also be called with runtime values (acts as a normal function)
    int n;
    std::cout << "Enter n for factorial (or 0 to use 6): ";
    // In non-interactive contexts just use 6
    n = 6;
    std::cout << n << "\n";
    std::cout << "factorial(" << n << ") at runtime = " << factorial(n) << "\n";
}


// =============================================================================
//  8. enum class (scoped enums)
//
//  C++98 enums had two problems:
//    a) Their values leaked into the enclosing scope, causing name collisions.
//    b) They implicitly converted to int, hiding type errors.
//
//  enum class fixes both: values are scoped to the enum name, and no implicit
//  int conversion.
// =============================================================================

// C++98 style — PAWN, ROOK etc. are in the global scope
// enum PieceType { PAWN, ROOK };  // clashes with chess namespace above

// C++11 style — scoped, no leakage
enum class PieceType { Pawn, Knight, Bishop, Rook, Queen, King };
enum class Color     { White, Black };

// Can specify the underlying integer type explicitly
enum class CastlingRights : unsigned char {
    None         = 0,
    WhiteKingside  = 1,
    WhiteQueenside = 2,
    BlackKingside  = 4,
    BlackQueenside = 8,
    All            = 15
};

void demo_enum_class() {
    std::cout << "\n--- enum class ---\n";

    PieceType pt = PieceType::Queen;
    // int bad = pt;   // ERROR: no implicit conversion
    int val = static_cast<int>(pt);   // explicit cast OK
    std::cout << "Queen enum value: " << val << "\n";

    Color c = Color::White;
    std::cout << "Color is " << (c == Color::White ? "white" : "black") << "\n";

    // Underlying type control
    CastlingRights cr = CastlingRights::WhiteKingside;
    std::cout << "Castling rights value: "
              << static_cast<int>(cr) << "\n";
}


// =============================================================================
//  9. override & final
//
//  override explicitly says "this method is meant to override a virtual
//  function in the base class." If it doesn't actually override anything
//  (e.g. you misspelled the name or got the signature wrong), the compiler
//  emits an error. This catches a very common silent bug in C++98.
//
//  final prevents further overriding or inheritance.
// =============================================================================

class Piece {
public:
    virtual std::string name() const { return "Piece"; }
    virtual int value()       const { return 0; }
    virtual ~Piece() {}
};

class Queen : public Piece {
public:
    // 'override' tells the compiler: verify this actually overrides something
    std::string name()  const override { return "Queen"; }
    int         value() const override { return 9; }
};

class Pawn final : public Piece {  // 'final': no class may inherit from Pawn
public:
    std::string name()  const override { return "Pawn"; }
    int         value() const override { return 1; }
};

// class SuperPawn : public Pawn {}; // ERROR: Pawn is final

void demo_override_final() {
    std::cout << "\n--- override & final ---\n";

    std::vector<std::unique_ptr<Piece>> pieces;
    pieces.push_back(std::unique_ptr<Piece>(new Queen()));
    pieces.push_back(std::unique_ptr<Piece>(new Pawn()));

    for (const auto& p : pieces) {
        std::cout << p->name() << " = " << p->value() << " points\n";
    }
}


// =============================================================================
//  10. UNIFORM INITIALISATION & INITIALIZER LISTS
//
//  C++98 had multiple inconsistent initialisation syntaxes:
//    int x = 5;           // copy-init
//    int x(5);            // direct-init
//    int arr[] = {1,2,3}; // aggregate init for arrays only
//    MyClass c(1,2);      // constructor call
//
//  C++11 introduces {} for everything — called "uniform initialisation".
//  It also adds std::initializer_list<T> so your own types can accept
//  brace-init lists.
// =============================================================================

class Position {
public:
    int file, rank;

    Position(int f, int r) : file(f), rank(r) {}

    // Constructor taking an initializer_list — enables: Position p = {4, 6};
    // (here we just add a two-element version manually)
};

// A simple collection that accepts brace-init
class OpeningBook {
public:
    OpeningBook(std::initializer_list<std::string> moves)
        : moves_(moves) {}

    void print() const {
        for (const auto& m : moves_) std::cout << "  " << m << "\n";
    }

private:
    std::vector<std::string> moves_;
};

void demo_uniform_init() {
    std::cout << "\n--- Uniform Initialisation ---\n";

    // All equivalent now:
    int a{5};                   // direct brace-init
    int b = {5};                // copy brace-init
    std::vector<int> v{1,2,3,4,5};
    std::map<std::string,int> m{{"e4",1}, {"d4",2}, {"Nf3",3}};

    std::cout << "a=" << a << "  b=" << b << "\n";
    std::cout << "vector: ";
    for (int x : v) std::cout << x << " ";
    std::cout << "\n";

    // Narrowing conversion detection — brace-init prevents silent truncation
    // int bad{3.7};  // ERROR: narrowing from double to int
    // int ok = 3.7;  // C++98: silently truncates to 3 (!)

    // initializer_list in our own class
    OpeningBook ruy = {"e4", "e5", "Nf3", "Nc6", "Bb5"};
    std::cout << "Ruy Lopez opening:\n";
    ruy.print();
}


// =============================================================================
//  11. VARIADIC TEMPLATES
//
//  Templates that accept any number of type parameters. The "..." is called
//  a parameter pack. This is how std::tuple, std::make_unique, std::forward,
//  and printf-style type-safe variadic functions are implemented.
// =============================================================================

// Base case: no arguments left
void printAll() {
    std::cout << "\n";
}

// Recursive case: peel off first argument, recurse on the rest
template <typename First, typename... Rest>
void printAll(const First& first, const Rest&... rest) {
    std::cout << first;
    if (sizeof...(rest) > 0) std::cout << ", ";
    printAll(rest...);
}

// Type-safe variadic sum
template <typename T>
T varSum(T t) { return t; }

template <typename T, typename... Args>
T varSum(T first, Args... args) { return first + varSum(args...); }

void demo_variadic() {
    std::cout << "\n--- Variadic Templates ---\n";
    printAll("position", 4, 6, 3.14, true);
    std::cout << "sum(1,2,3,4,5) = " << varSum(1,2,3,4,5) << "\n";
}


// =============================================================================
//  12. std::thread & std::mutex
//
//  Before C++11, threading required platform-specific APIs (pthreads on Linux,
//  Win32 threads on Windows). C++11 standardised threading with std::thread,
//  std::mutex, std::lock_guard, std::condition_variable, and std::atomic.
// =============================================================================

std::mutex coutMutex;   // protects std::cout from interleaved output

void searchWorker(int id, int depth) {
    // lock_guard acquires the mutex and releases it when it goes out of scope
    // (RAII pattern — even if an exception is thrown)
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << "  Worker " << id << " searching at depth " << depth << "\n";
}

void demo_thread() {
    std::cout << "\n--- std::thread & std::mutex ---\n";

    // Launch 4 threads simulating parallel search workers
    std::vector<std::thread> workers;
    for (int i = 0; i < 4; ++i) {
        workers.emplace_back(searchWorker, i, i + 3);
    }

    // Wait for all threads to finish
    for (auto& t : workers) {
        t.join();
    }
    std::cout << "All workers finished\n";
}


// =============================================================================
//  13. std::chrono — Type-safe Time
//
//  Before C++11: time() returns seconds as a plain integer. There's no type
//  system enforcement — you could accidentally add milliseconds to seconds.
//  chrono introduces strong duration types: seconds, milliseconds, etc., and
//  the compiler enforces unit consistency.
// =============================================================================

void demo_chrono() {
    std::cout << "\n--- std::chrono ---\n";

    auto start = std::chrono::high_resolution_clock::now();

    // Simulate some work
    volatile int x = 0;
    for (int i = 0; i < 1000000; ++i) x += i;

    auto end = std::chrono::high_resolution_clock::now();

    // duration_cast converts between duration units
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Work took " << us.count() << " microseconds\n";

    // Type-safe: this would be a compile error:
    // auto wrong = std::chrono::seconds(1) + std::chrono::milliseconds(500);
    // You must explicitly convert units first
    auto total = std::chrono::seconds(1)
               + std::chrono::duration_cast<std::chrono::seconds>(
                     std::chrono::milliseconds(500));
    std::cout << "1s + 500ms (as seconds) = " << total.count() << "s\n";
}


// =============================================================================
//  14. NEW STL CONTAINERS
// =============================================================================

void demo_new_containers() {
    std::cout << "\n--- New STL Containers ---\n";

    // std::array — fixed-size array with STL interface.
    // Unlike C arrays: knows its size, can be copied, works with algorithms.
    std::array<int, 8> ranks = {1, 2, 3, 4, 5, 6, 7, 8};
    std::cout << "std::array ranks: ";
    for (const auto& r : ranks) std::cout << r << " ";
    std::cout << "\n";
    std::cout << "size=" << ranks.size() << "  front=" << ranks.front()
              << "  back=" << ranks.back() << "\n";

    // std::unordered_map — hash map, O(1) average lookup (vs O(log n) for map)
    // Better for large datasets where ordering doesn't matter
    std::unordered_map<std::string, int> pieceValues;
    pieceValues["pawn"]   = 1;
    pieceValues["knight"] = 3;
    pieceValues["bishop"] = 3;
    pieceValues["rook"]   = 5;
    pieceValues["queen"]  = 9;

    std::cout << "Piece values (unordered_map):\n";
    // Note: unordered_map does NOT iterate in insertion order
    for (const auto& kv : pieceValues) {
        std::cout << "  " << kv.first << " = " << kv.second << "\n";
    }

    // std::tuple — heterogeneous fixed-size collection
    auto move = std::make_tuple(std::string("e4"), 4, 3, true);
    std::cout << "Move tuple: notation=" << std::get<0>(move)
              << "  from=" << std::get<1>(move)
              << "  to=" << std::get<2>(move)
              << "  captures=" << std::boolalpha << std::get<3>(move) << "\n";
}


// =============================================================================
//  15. static_assert
//
//  A compile-time assertion. If the condition is false, compilation fails
//  with a human-readable message. No runtime overhead at all.
//  Perfect for verifying template constraints and platform assumptions.
// =============================================================================

static_assert(sizeof(int) >= 4,   "int must be at least 32 bits");
static_assert(sizeof(char) == 1,  "char must be exactly 1 byte");

template <typename T>
void onlyForIntegers(T /*val*/) {
    static_assert(std::is_integral<T>::value,
                  "onlyForIntegers: T must be an integral type");
}

void demo_static_assert() {
    std::cout << "\n--- static_assert ---\n";
    onlyForIntegers(42);
    onlyForIntegers('A');
    // onlyForIntegers(3.14);  // compile error: "T must be an integral type"
    std::cout << "All static_asserts passed at compile time\n";
}


// =============================================================================
//  MAIN
// =============================================================================

int main() {
    std::cout << "======================================\n";
    std::cout << "  C++11 Feature Showcase\n";
    std::cout << "======================================\n";

    demo_auto();
    demo_range_for();
    demo_lambdas();
    demo_nullptr();
    demo_move();
    demo_smart_pointers();
    demo_constexpr();
    demo_enum_class();
    demo_override_final();
    demo_uniform_init();
    demo_variadic();
    demo_thread();
    demo_chrono();
    demo_new_containers();
    demo_static_assert();

    return 0;
}
