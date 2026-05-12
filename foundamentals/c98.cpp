// =============================================================================
//  C++98 — The Foundation
//  Compile: g++ -std=c++98 -Wall -o c98 c98.cpp && ./c98
// =============================================================================
//
//  Before C++98 there was no standard at all — every compiler did things
//  slightly differently. This release gave us the STL, templates, namespaces,
//  exceptions, and the bool type. Everything in modern C++ sits on top of this.
//
// =============================================================================

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <algorithm>   // sort, find, for_each, ...
#include <iterator>    // ostream_iterator
#include <stdexcept>   // std::runtime_error, std::out_of_range
#include <cassert>     // assert()
#include <cmath>       // sqrt()
#include <sstream>     // std::ostringstream

// =============================================================================
//  1. NAMESPACES
//  Problem they solve: without namespaces, any two libraries that define a
//  function called "init()" or a class called "Vector" would collide at link
//  time. Namespaces are essentially named scopes.
// =============================================================================

namespace chess {

    // Piece types encoded as an enum (plain C-style in C++98 — they leak into
    // the surrounding scope, which is why C++11 added "enum class").
    enum PieceType { EMPTY, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };
    enum Color     { WHITE, BLACK };

    // A simple struct for a board square
    struct Square {
        int file;   // column 0-7  (a-h)
        int rank;   // row    0-7  (1-8)
    };

    // A piece on the board
    struct Piece {
        PieceType type;
        Color     color;
    };

    // Helper: convert PieceType to a display character
    char pieceChar(PieceType t) {
        switch (t) {
            case PAWN:   return 'P';
            case KNIGHT: return 'N';
            case BISHOP: return 'B';
            case ROOK:   return 'R';
            case QUEEN:  return 'Q';
            case KING:   return 'K';
            default:     return '.';
        }
    }

}   // namespace chess

// A separate namespace — could be a different library with no name collision
namespace utils {

    std::string repeat(const std::string& s, int n) {
        std::string result;
        for (int i = 0; i < n; ++i) result += s;
        return result;
    }

}   // namespace utils


// =============================================================================
//  2. TEMPLATES — Generic Programming
//
//  Templates let you write code that works for any type. The compiler
//  instantiates a concrete version for each type you use. This is "zero cost"
//  abstraction: no runtime overhead, no virtual dispatch needed.
// =============================================================================

// --- Function template ---
// Works for any type that supports operator<
template <typename T>
const T& maxOf(const T& a, const T& b) {
    return (a > b) ? a : b;
}

// --- Class template ---
// A generic stack backed by std::vector. "Stack<int>", "Stack<std::string>"
// etc. are all valid instantiations.
template <typename T>
class Stack {
public:
    void push(const T& value) {
        data_.push_back(value);
    }

    // Returns top element by value so the caller can't hold a dangling ref
    // after a subsequent pop.
    T top() const {
        if (data_.empty()) throw std::runtime_error("Stack::top() on empty stack");
        return data_.back();
    }

    void pop() {
        if (data_.empty()) throw std::runtime_error("Stack::pop() on empty stack");
        data_.pop_back();
    }

    bool  empty() const { return data_.empty(); }
    int   size()  const { return static_cast<int>(data_.size()); }

private:
    std::vector<T> data_;
};


// =============================================================================
//  3. THE STL — Standard Template Library
//
//  Three pillars:
//    a) Containers — store objects  (vector, list, map, set, ...)
//    b) Algorithms — operate on ranges  (sort, find, count_if, ...)
//    c) Iterators  — bridge between containers and algorithms
//
//  Designed by Alexander Stepanov. The genius: algorithms are written against
//  *iterator concepts*, not concrete containers, so std::sort works on a
//  vector, a deque, or a plain C array equally well.
// =============================================================================

// A small functor (function object) — a class with operator()
// In C++98 this is how you pass custom behaviour to algorithms; C++11
// replaces most of these with lambdas.
struct PrintPiece {
    void operator()(const chess::Piece& p) const {
        std::cout << (p.color == chess::WHITE ? "W" : "B")
                  << chess::pieceChar(p.type) << " ";
    }
};

struct CompareByType {
    bool operator()(const chess::Piece& a, const chess::Piece& b) const {
        return a.type < b.type;
    }
};

void demo_stl() {
    std::cout << "\n--- STL: Containers & Algorithms ---\n";

    // std::vector: contiguous dynamic array.  O(1) random access, O(1)
    // amortised push_back, O(n) insert in the middle.
    std::vector<chess::Piece> pieces;
    pieces.push_back((chess::Piece){ chess::QUEEN,  chess::WHITE });
    pieces.push_back((chess::Piece){ chess::ROOK,   chess::WHITE });
    pieces.push_back((chess::Piece){ chess::PAWN,   chess::BLACK });
    pieces.push_back((chess::Piece){ chess::KNIGHT, chess::WHITE });
    pieces.push_back((chess::Piece){ chess::BISHOP, chess::BLACK });

    std::cout << "Before sort: ";
    std::for_each(pieces.begin(), pieces.end(), PrintPiece());
    std::cout << "\n";

    // Sort by piece type using our comparator functor
    std::sort(pieces.begin(), pieces.end(), CompareByType());

    std::cout << "After sort:  ";
    std::for_each(pieces.begin(), pieces.end(), PrintPiece());
    std::cout << "\n";

    // std::map: sorted associative container, O(log n) lookup.
    // Keys are always unique. Perfect for piece-value tables.
    std::map<chess::PieceType, int> pieceValues;
    pieceValues[chess::PAWN]   = 1;
    pieceValues[chess::KNIGHT] = 3;
    pieceValues[chess::BISHOP] = 3;
    pieceValues[chess::ROOK]   = 5;
    pieceValues[chess::QUEEN]  = 9;
    pieceValues[chess::KING]   = 0;  // priceless, but 0 for material count

    // Iterating a map gives you std::pair<const Key, Value>
    std::cout << "\nPiece values:\n";
    for (std::map<chess::PieceType,int>::iterator it = pieceValues.begin();
         it != pieceValues.end(); ++it) {
        // it->first  = key (PieceType)
        // it->second = value (int)
        std::cout << "  " << chess::pieceChar(it->first)
                  << " = " << it->second << "\n";
    }

    // std::list: doubly linked list.  O(1) insert/erase anywhere, O(n) access.
    // Less cache-friendly than vector; useful when you need stable iterators
    // or frequent middle insertions.
    std::list<std::string> moveHistory;
    moveHistory.push_back("e4");
    moveHistory.push_back("e5");
    moveHistory.push_back("Nf3");
    moveHistory.push_back("Nc6");

    std::cout << "\nMove history: ";
    for (std::list<std::string>::const_iterator it = moveHistory.begin();
         it != moveHistory.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";

    // Algorithms — work on any range [begin, end)
    std::vector<int> ranks;
    for (int i = 1; i <= 8; ++i) ranks.push_back(i);

    // find the iterator pointing to value 5
    std::vector<int>::iterator found = std::find(ranks.begin(), ranks.end(), 5);
    if (found != ranks.end()) {
        std::cout << "\nFound rank: " << *found << "\n";
    }
}


// =============================================================================
//  4. BOOL TYPE
//
//  Before C++98 (and in C), booleans were plain ints: 0 = false, non-zero =
//  true. C++98 added the native bool type. It's a small change but it
//  matters for function overloading and type safety.
// =============================================================================

bool isValidSquare(int file, int rank) {
    // Explicit bool return — can't accidentally return 2 or -1
    return (file >= 0 && file < 8 && rank >= 0 && rank < 8);
}


// =============================================================================
//  5. EXCEPTIONS
//
//  Exceptions decouple error reporting from error handling. The function that
//  detects a problem throws; the caller (or any ancestor in the call stack)
//  catches and decides what to do.
//
//  The standard exception hierarchy:
//    std::exception
//      std::runtime_error   (errors only detectable at runtime)
//      std::logic_error     (programmer mistakes: out_of_range, invalid_arg)
//      std::bad_alloc       (operator new failure)
//      ... and many more
// =============================================================================

// A Board class that uses exceptions for bounds checking
class Board {
public:
    Board() {
        // Initialise all 64 squares to EMPTY
        for (int r = 0; r < 8; ++r)
            for (int f = 0; f < 8; ++f)
                grid_[r][f] = (chess::Piece){ chess::EMPTY, chess::WHITE };
    }

    void setPiece(int file, int rank, chess::Piece p) {
        if (!isValidSquare(file, rank))
            // Inherits from std::logic_error, which inherits from std::exception
            throw std::out_of_range("setPiece: square out of range");
        grid_[rank][file] = p;
    }

    chess::Piece getPiece(int file, int rank) const {
        if (!isValidSquare(file, rank))
            throw std::out_of_range("getPiece: square out of range");
        return grid_[rank][file];
    }

private:
    chess::Piece grid_[8][8];
};

void demo_exceptions() {
    std::cout << "\n--- Exceptions ---\n";

    Board board;
    board.setPiece(4, 0, (chess::Piece){ chess::QUEEN, chess::WHITE });
    std::cout << "Placed white queen at e1\n";

    // Deliberately trigger an exception
    try {
        board.setPiece(8, 0, (chess::Piece){ chess::ROOK, chess::WHITE });
        std::cout << "Should NOT reach here\n";
    }
    catch (const std::out_of_range& e) {
        // Catch by const reference — avoids copying the exception object
        // and preserves polymorphic behaviour for derived exception types.
        std::cout << "Caught out_of_range: " << e.what() << "\n";
    }
    catch (const std::exception& e) {
        // Fallback for any other standard exception
        std::cout << "Caught std::exception: " << e.what() << "\n";
    }
}


// =============================================================================
//  6. CLASS FEATURES: CONSTRUCTORS, DESTRUCTORS, COPY, OPERATORS
//
//  C++98 laid out the "Rule of Three": if a class manages a resource (memory,
//  file handle, lock...) you must explicitly define the destructor, copy
//  constructor, and copy assignment operator. Forgetting any one leads to
//  bugs like double-free or shallow copies.
// =============================================================================

// A dynamic array of moves — manually manages heap memory to illustrate
// the Rule of Three.
class MoveList {
public:
    // Constructor
    explicit MoveList(int capacity = 16)
        : size_(0), capacity_(capacity),
          moves_(new std::string[capacity]) {}

    // Destructor — releases heap memory
    ~MoveList() {
        delete[] moves_;
    }

    // Copy constructor — deep copy
    MoveList(const MoveList& other)
        : size_(other.size_), capacity_(other.capacity_),
          moves_(new std::string[other.capacity_]) {
        for (int i = 0; i < size_; ++i)
            moves_[i] = other.moves_[i];
    }

    // Copy assignment operator — deep copy, handle self-assignment
    MoveList& operator=(const MoveList& other) {
        if (this == &other) return *this;   // guard against self-assignment
        delete[] moves_;
        capacity_ = other.capacity_;
        size_     = other.size_;
        moves_    = new std::string[capacity_];
        for (int i = 0; i < size_; ++i)
            moves_[i] = other.moves_[i];
        return *this;
    }

    void add(const std::string& move) {
        if (size_ == capacity_) {
            // Grow the buffer (doubles in size — same strategy as std::vector)
            int newCap = capacity_ * 2;
            std::string* newBuf = new std::string[newCap];
            for (int i = 0; i < size_; ++i) newBuf[i] = moves_[i];
            delete[] moves_;
            moves_    = newBuf;
            capacity_ = newCap;
        }
        moves_[size_++] = move;
    }

    int         size()              const { return size_; }
    std::string operator[](int i)   const { return moves_[i]; }

private:
    int          size_;
    int          capacity_;
    std::string* moves_;   // raw pointer — we own this memory
};

void demo_class() {
    std::cout << "\n--- Rule of Three & Operator Overloading ---\n";

    MoveList game;
    game.add("e4"); game.add("e5"); game.add("Nf3"); game.add("Nc6");

    // Copy constructor exercised here
    MoveList copy = game;
    copy.add("Bb5");   // modifying the copy does NOT affect 'game'

    std::cout << "Original (" << game.size() << " moves): ";
    for (int i = 0; i < game.size(); ++i) std::cout << game[i] << " ";
    std::cout << "\n";

    std::cout << "Copy     (" << copy.size() << " moves): ";
    for (int i = 0; i < copy.size(); ++i) std::cout << copy[i] << " ";
    std::cout << "\n";
}


// =============================================================================
//  MAIN — wire everything together
// =============================================================================

int main() {
    std::cout << "======================================\n";
    std::cout << "  C++98 Feature Showcase\n";
    std::cout << "======================================\n";

    // 1. Namespaces
    std::cout << "\n--- Namespaces ---\n";
    std::string bar = utils::repeat("-", 20);
    std::cout << bar << "\n";
    std::cout << "chess::QUEEN value: " << chess::pieceChar(chess::QUEEN) << "\n";
    std::cout << "utils::repeat('-', 20): " << bar << "\n";

    // 2. Templates
    std::cout << "\n--- Templates ---\n";
    std::cout << "maxOf(7, 3)       = " << maxOf(7, 3)       << "\n";
    std::cout << "maxOf(3.14, 2.71) = " << maxOf(3.14, 2.71) << "\n";
    std::cout << "maxOf<std::string>(\"apple\",\"banana\") = "
              << maxOf<std::string>("apple", "banana") << "\n";

    // Generic Stack
    Stack<int> s;
    s.push(10); s.push(20); s.push(30);
    std::cout << "Stack top: " << s.top() << ", size: " << s.size() << "\n";
    s.pop();
    std::cout << "After pop, top: " << s.top() << "\n";

    try {
        Stack<int> empty;
        empty.top();  // throws
    } catch (const std::runtime_error& e) {
        std::cout << "Caught empty-stack exception: " << e.what() << "\n";
    }

    // 3. STL
    demo_stl();

    // 4. Bool
    std::cout << "\n--- Bool type ---\n";
    std::cout << std::boolalpha;  // print true/false instead of 1/0
    std::cout << "isValidSquare(4,4) = " << isValidSquare(4, 4) << "\n";
    std::cout << "isValidSquare(8,0) = " << isValidSquare(8, 0) << "\n";

    // 5. Exceptions
    demo_exceptions();

    // 6. Rule of Three
    demo_class();

    return 0;
}
