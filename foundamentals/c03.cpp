// =============================================================================
//  C++03 — The Maintenance Release
//  Compile: g++ -std=c++03 -Wall -o c03 c03.cpp && ./c03
//
//  NOTE: g++ treats -std=c++03 and -std=c++98 as equivalent because C++03
//        introduced NO new language features. It was a purely editorial and
//        technical corrigendum to fix inconsistencies and clarify wording in
//        the C++98 standard. You will get the same binary either way.
// =============================================================================
//
//  What actually changed in C++03:
//
//  1. VALUE INITIALISATION FIX
//     In C++98 the expression "T()" for a non-class type was underspecified.
//     C++03 made it clear: T() zero-initialises the object.
//     Example:  int x = int();   // guaranteed to be 0
//
//  2. std::vector<bool> SPECIALISATION CLARIFIED
//     The C++98 wording left the space-efficient bit-packing behaviour of
//     std::vector<bool> ambiguous. C++03 explicitly required it, along with
//     the quirky reference proxy type that comes with it.
//
//  3. EXCEPTION SPECIFICATION RULES TIGHTENED
//     The behaviour of violating a throw() exception specification was
//     clarified to call std::unexpected() -> std::terminate().
//
//  4. TEMPLATE INSTANTIATION RULES
//     Several edge cases around dependent names and two-phase name lookup
//     were clarified, fixing real ambiguities across compilers.
//
//  The examples below demonstrate the TWO behaviours that were most
//  practically relevant: value initialisation and vector<bool> quirks.
//
// =============================================================================

#include <iostream>
#include <vector>
#include <string>
#include <cassert>

// =============================================================================
//  1. VALUE INITIALISATION (the most important C++03 fix)
//
//  C++98 had three initialisation forms:
//    default-initialisation : T x;         (leaves POD types uninitialised)
//    copy-initialisation    : T x = T();   (unspecified for POD in C++98)
//    direct-initialisation  : T x(T());
//
//  C++03 formalised "value initialisation" triggered by T():
//    - POD types (int, double, pointers...)  -> zero-initialised
//    - Class types with a user constructor  -> that constructor is called
//    - Class types with no user constructor -> all members zero-initialised
//
//  This was critical for writing generic code. Before C++03 a template like:
//    template<typename T> T defaultValue() { return T(); }
//  had undefined behaviour for T=int (might return garbage on some compilers).
//  After C++03 it reliably returns 0.
// =============================================================================

// Generic function that creates a zero/default value of any type
// After C++03 this is well-defined even for T = int, double, pointer types.
template <typename T>
T makeDefault() {
    return T();
}

struct Stats {
    int    wins;
    int    losses;
    double score;
    // No user-defined constructor — C++03 guarantees T() zero-inits all fields
};

void demo_value_init() {
    std::cout << "\n--- Value Initialisation (C++03 fix) ---\n";

    // POD types: T() guarantees zero in C++03
    int    i  = makeDefault<int>();
    double d  = makeDefault<double>();
    int*   p  = makeDefault<int*>();

    std::cout << "int()    = " << i << "  (guaranteed 0, was undefined in C++98)\n";
    std::cout << "double() = " << d << "  (guaranteed 0.0)\n";
    std::cout << "int*()   = " << (p == 0 ? "null" : "non-null") << "  (guaranteed null)\n";

    // Struct without constructor: all fields zero-initialised
    Stats s = Stats();
    std::cout << "Stats().wins   = " << s.wins   << "\n";
    std::cout << "Stats().losses = " << s.losses << "\n";
    std::cout << "Stats().score  = " << s.score  << "\n";

    // Practical use: zero-initialise arrays generically
    // This pattern appears everywhere in generic containers
    int arr[5] = {};  // all 5 elements zero — also clarified by C++03
    std::cout << "int arr[5] = {} -> arr[0]=" << arr[0]
              << " arr[4]=" << arr[4] << "\n";
}


// =============================================================================
//  2. std::vector<bool> SPECIALISATION
//
//  std::vector<bool> is a special case. Unlike std::vector<int>, it does NOT
//  store each bool as a separate byte. Instead it packs bits — each bool
//  takes 1 bit, so a vector of 64 bools fits in one 8-byte word.
//
//  The catch: because you can't have a pointer/reference to a single bit,
//  v[i] does NOT return a bool&. It returns a proxy object. C++03 made this
//  behaviour mandatory and clarified how the proxy works.
//
//  This is widely considered a design mistake in the STL — hence why C++23
//  added std::mdspan and there are proposals for std::vector<bool> reform.
//  But it's important to know about it because it WILL bite you if you write
//  generic code that assumes vector<T>::reference is T&.
// =============================================================================

void demo_vector_bool() {
    std::cout << "\n--- std::vector<bool> Specialisation (C++03 clarified) ---\n";

    // An 8x8 chess board visibility mask: which squares are attacked?
    std::vector<bool> attacked(64, false);
    attacked[27] = true;   // d4 attacked
    attacked[36] = true;   // e5 attacked

    std::cout << "Square d4 (27) attacked: " << std::boolalpha << (bool)attacked[27] << "\n";
    std::cout << "Square a1 (0)  attacked: " << (bool)attacked[0]  << "\n";

    // The proxy trap — this is why vector<bool> is special:
    // auto ref = attacked[27];  // ref is NOT bool&, it's a proxy object!
    // In C++03 this behaviour is specified. In C++11 you can write:
    //   bool val = attacked[27];   <- always safe, copies the value

    // Count attacked squares using std::count
    // (std::count works through iterators, so it handles the proxy correctly)
    int countAttacked = 0;
    for (int i = 0; i < 64; ++i)
        if (attacked[i]) ++countAttacked;
    std::cout << "Total attacked squares: " << countAttacked << "\n";

    // Contrast with vector<char> which has no proxy — this behaves normally
    std::vector<char> flags(64, 0);
    flags[27] = 1;
    char& realRef = flags[27];   // this is a genuine char& — no proxy
    realRef = 0;                 // modifying through reference works fine
    std::cout << "vector<char>[27] after reset via ref: " << (int)flags[27] << "\n";
}


// =============================================================================
//  3. EXCEPTION SPECIFICATION CLARIFICATION
//
//  C++98 introduced throw() to declare that a function never throws.
//  C++03 clarified: if a function declared throw() does throw, the runtime
//  calls std::unexpected(), which by default calls std::terminate().
//
//  NOTE: throw() was deprecated in C++11 and replaced by noexcept.
//  noexcept is stricter (the compiler can optimise more aggressively) and
//  is what you should use in modern code. It's shown here for history.
// =============================================================================

// throw() says: "this function promises never to throw"
// Violating it in C++03 calls unexpected() -> terminate()
int safeAdd(int a, int b) throw() {
    // Fine — no throw here
    return a + b;
}

void demo_exception_spec() {
    std::cout << "\n--- Exception Specifications (C++03 clarification) ---\n";
    int result = safeAdd(3, 4);
    std::cout << "safeAdd(3, 4) = " << result << "\n";
    std::cout << "throw() exception spec: calls terminate() if violated\n";
    std::cout << "(Replaced by noexcept in C++11)\n";
}


// =============================================================================
//  MAIN
// =============================================================================

int main() {
    std::cout << "======================================\n";
    std::cout << "  C++03 Feature Showcase\n";
    std::cout << "  (No new features — clarifications)\n";
    std::cout << "======================================\n";

    demo_value_init();
    demo_vector_bool();
    demo_exception_spec();

    std::cout << "\n--- Summary ---\n";
    std::cout << "C++03 had zero new language features.\n";
    std::cout << "Its real contribution was removing ambiguities that caused\n";
    std::cout << "different compilers to behave differently for the same code.\n";
    std::cout << "Value initialisation (T()) is the one you'll encounter most.\n";

    return 0;
}
