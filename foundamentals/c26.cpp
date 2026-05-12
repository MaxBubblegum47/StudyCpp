// =============================================================================
//  C++26 — The New Era
//  Compile: g++ -std=c++23 -Wall -o c26 c26.cpp && ./c26
//
//  NOTE: This file uses -std=c++23 because GCC 13 (Ubuntu 24.04) does NOT
//  support -std=c++26. The file is a showcase/documentation of C++26 features:
//  sections that need GCC 15/16 show the syntax and describe what they do,
//  with portable workarounds that compile on GCC 13.
//
//  To run the real C++26 features:
//    sudo add-apt-repository ppa:ubuntu-toolchain-r/test
//    sudo apt install gcc-16 g++-16
//    g++-16 -std=c++26 -freflection -Wall -o c26 c26.cpp && ./c26
//
//  Or try online: https://godbolt.org  (select "x86-64 gcc (trunk)", -std=c++26)
//
//  WG21 officially shipped C++26 on March 28, 2026.
//  Herb Sutter: "most compelling release since C++11"
// =============================================================================
//
//  Covered here:
//    1.  Contracts (pre/post/contract_assert)  — GCC 15+  (workaround shown)
//    2.  #embed directive                       — GCC 15+  (simulated)
//    3.  std::execution (sender/receiver)       — GCC 14+  (documented)
//    4.  Memory safety hardening                — all GCC  (-D_GLIBCXX_ASSERTIONS)
//    5.  Pack indexing                          — GCC 15+  (tuple workaround)
//    6.  Reflection (std::meta)                 — GCC 16   (documented)
//    7.  f-string interpolation                 — GCC 16+  (documented)
//    8.  std::simd (SIMD parallelism)           — GCC 14+  (documented)
//    9.  Erroneous behaviour (uninit reads)     — spec change (documented)
// =============================================================================

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <span>
#include <ranges>
#include <algorithm>
#include <format>
#include <type_traits>
#include <utility>
#include <cstdint>
#include <numeric>
#include <tuple>
#include <stdexcept>

// println shim (same as c23 — <print> needs GCC 14+)
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
//  1. CONTRACTS — pre/post/contract_assert
//
//  Contracts bring Design by Contract into the C++ standard.
//  Herb Sutter: "infinitely better than C's assert macro."
//
//  Three kinds of contract annotations:
//    pre (condition)         — precondition: must hold when function is called
//    post (result: cond)     — postcondition: must hold when function returns
//    contract_assert(cond)   — assertion: must hold at that point in code
//
//  Four enforcement modes (selectable per build):
//    ignore        — annotations are compiled away entirely (max performance)
//    observe       — violations call a handler but execution continues
//    enforce       — violations call the handler then terminate (default)
//    quick_enforce — fastest enforcement, no handler call, immediate abort
//
//  Compiler support: GCC 15+ with -fcontracts, Clang 18+ (partial)
//
//  SYNTAX NOTE: The C++26 syntax uses attribute-like notation.
//  If your compiler doesn't support it yet, the examples show the syntax
//  plus a workaround using manual asserts.
// =============================================================================

// ---
// Full C++26 syntax (requires GCC 15+ with -fcontracts or Clang 18+):
// ---
// int squareToIndex(int file, int rank)
//     pre (file >= 0 && file <= 7)
//     pre (rank >= 0 && rank <= 7)
//     post (result: result >= 0 && result < 64)
// {
//     contract_assert(file + rank >= 0);
//     return rank * 8 + file;
// }
// ---

// Portable version with manual checks — same INTENT as contracts:
int squareToIndex(int file, int rank) {
    // pre-condition: manually checked (contracts would do this automatically)
    if (!(file >= 0 && file <= 7))
        throw std::out_of_range(std::format("file {} out of [0,7]", file));
    if (!(rank >= 0 && rank <= 7))
        throw std::out_of_range(std::format("rank {} out of [0,7]", rank));

    int result = rank * 8 + file;

    // post-condition check
    if (!(result >= 0 && result < 64))
        throw std::logic_error("postcondition violated");

    return result;
}

// With contracts you can express the same intent directly in the signature:
// int squareToIndex(int file, int rank)
//     pre  (file >= 0 && file <= 7, "file must be in [0,7]")
//     pre  (rank >= 0 && rank <= 7, "rank must be in [0,7]")
//     post (r: r >= 0 && r < 64)
// {
//     contract_assert(file * 8 + rank < 64);
//     return rank * 8 + file;
// }

void demo_contracts() {
    std::println("\n--- Contracts (C++26 pre/post/contract_assert) ---");

    // Valid call
    int e4 = squareToIndex(4, 3);
    std::println("e4 = square {}", e4);

    // Contract violation
    try {
        squareToIndex(8, 0);  // violates pre (file <= 7)
    } catch (const std::out_of_range& e) {
        std::println("Caught precondition violation: {}", e.what());
    }

    std::println("");
    std::println("With C++26 contracts syntax (-fcontracts on GCC 15+):");
    std::println("  int f(int x)");
    std::println("      pre (x > 0)");
    std::println("      post (result: result >= x)");
    std::println("  { contract_assert(x < 1000); return x * 2; }");
    std::println("  // Violation calls handler, then terminate (enforce mode)");
    std::println("  // At Google, hardened stdlib + contracts cut segfaults 30%");
}


// =============================================================================
//  2. #embed DIRECTIVE
//
//  Include binary resources directly into the source code at compile time.
//  Before #embed:
//    - Generate a header with xxd: xxd -i icon.png > icon.h
//    - Use linker scripts to embed data
//    - Read files at runtime (not compile-time)
//
//  With #embed the data is inserted as a byte sequence by the preprocessor.
//  Zero runtime overhead, no external tools needed.
//
//  Requires: GCC 15+ / Clang 19+
// =============================================================================

void demo_embed() {
    std::println("\n--- #embed directive (C++26) ---");

    // This is what #embed looks like in real code:
    //   const unsigned char icon_data[] = {
    //     #embed "assets/icon.png"
    //   };
    //   std::println("icon size: {} bytes", sizeof(icon_data));
    //
    //   // Or with a fallback for compatibility:
    //   #if __has_embed("data.bin") == __STDC_EMBED_FOUND__
    //     static constexpr unsigned char data[] = { #embed "data.bin" };
    //   #else
    //     // runtime fallback
    //   #endif

    // Simulated equivalent (what the preprocessor would generate):
    static const unsigned char fakeIcon[] = {
        // In real code this would be the PNG bytes
        0x89, 0x50, 0x4E, 0x47,   // PNG magic bytes
        0x0D, 0x0A, 0x1A, 0x0A
    };

    std::println("Simulated embedded icon: {} bytes", sizeof(fakeIcon));
    std::print("First 4 bytes (PNG magic): ");
    for (auto b : fakeIcon) std::print("{:#04x} ", b);
    std::println("");

    std::println("With #embed (GCC 15+):");
    std::println("  const unsigned char data[] = {{ #embed \"file.bin\" }};");
    std::println("  // data[] is available at compile time — no file I/O at runtime");
}


// =============================================================================
//  3. PACK INDEXING (variadic template indexed access)
//
//  Before C++26: accessing the Nth element of a parameter pack required
//  recursive template metaprogramming tricks (std::tuple + std::get).
//
//  C++26 adds direct indexing with ...[N]:
//    template<typename... Ts>
//    using Third = Ts...[2];   // the third type in the pack
//
//  Requires: GCC 15+
// =============================================================================

// C++26 syntax:
// template<typename... Ts>
// auto getFirst(Ts... args) { return args...[0]; }
// template<typename... Ts>
// auto getLast(Ts... args)  { return args...[sizeof...(args) - 1]; }

// C++23 and earlier workaround using tuple:
template<std::size_t N, typename... Ts>
auto getAt(Ts&&... args) {
    return std::get<N>(std::make_tuple(std::forward<Ts>(args)...));
}

void demo_pack_indexing() {
    std::println("\n--- Pack Indexing (C++26) ---");

    // C++26 syntax would be:
    //   template<typename... Ts>
    //   auto first(Ts... args) { return args...[0]; }
    //   auto last (Ts... args) { return args...[sizeof...(Ts)-1]; }

    // Using the tuple-based workaround:
    auto first = getAt<0>(10, 20, 30, 40);
    auto third = getAt<2>(10, 20, 30, 40);
    std::println("first = {}", first);
    std::println("third = {}", third);

    std::println("");
    std::println("C++26 pack indexing syntax:");
    std::println("  template<typename... Ts>");
    std::println("  auto getNth(Ts... args, int n) {{ return args...[n]; }}");
    std::println("  // getNth(10,20,30,40, 2) == 30");
    std::println("  // Also for types: Ts...[0] gives first type in pack");
}


// =============================================================================
//  4. REFLECTION (std::meta) — The Marquee Feature
//
//  Compile-time introspection: inspect types, functions, enums, and generate
//  code — all at compile time with zero runtime overhead.
//
//  Called "bigger than templates" by Herb Sutter. The basis for:
//    - Automatic serialisation/deserialisation
//    - Enum-to-string (and string-to-enum)
//    - Automatic struct comparison/hashing
//    - ORM-like data binding
//    - Type-safe printf without macros
//    - ... and much more we haven't discovered yet
//
//  Syntax: ^ is the "reflection operator" — ^T gives a std::meta::info
//          for type T.  [: ... :] is the "splicer" — turns a meta::info
//          back into a C++ construct.
//
//  Requires: GCC 16 with -freflection  OR  reflection-enabled Clang fork
//            On Compiler Explorer: select "Clang (experimental reflection)"
// =============================================================================

// This is what reflection code looks like — it won't compile on standard GCC 13
// but shows the idiom clearly.

// #ifdef __cpp_reflection  // Feature test macro for reflection

// // Enum to string — zero boilerplate, works for ANY enum
// template<typename E> requires std::is_enum_v<E>
// constexpr std::string enumToString(E value) {
//     template for (constexpr auto member : std::meta::members_of(^E)) {
//         if (value == [:member:])
//             return std::string(std::meta::identifier_of(member));
//     }
//     return "<unknown>";
// }
//
// enum class PieceType { Pawn, Knight, Bishop, Rook, Queen, King };
// // enumToString(PieceType::Queen) == "Queen"  — at compile time!
//
// // Iterate struct fields — automatic "to string" for any struct
// template<typename T>
// std::string structToString(const T& obj) {
//     std::string result = "{";
//     template for (constexpr auto field : std::meta::members_of(^T)) {
//         result += std::meta::identifier_of(field);
//         result += "=";
//         result += std::to_string(obj.[:field:]);
//         result += ", ";
//     }
//     return result + "}";
// }
//
// struct Position { int file; int rank; };
// // structToString(Position{4,3}) == "{file=4, rank=3}"

// #endif

void demo_reflection() {
    std::println("\n--- Reflection / std::meta (C++26) ---");
    std::println("Requires: GCC 16 -std=c++26 -freflection");
    std::println("          OR reflection-enabled Clang fork");
    std::println("          OR Compiler Explorer > 'Clang (experimental reflection)'");
    std::println("");
    std::println("Key syntax:");
    std::println("  ^T         — reflect on type T -> std::meta::info");
    std::println("  [:info:]   — splice: turn meta::info back into C++ code");
    std::println("");
    std::println("Example — enum to string at compile time:");
    std::println("  template<typename E>");
    std::println("  constexpr std::string enumToString(E value) {{");
    std::println("    template for (constexpr auto m : std::meta::members_of(^E)) {{");
    std::println("      if (value == [:m:])");
    std::println("          return std::meta::identifier_of(m);");
    std::println("    }}");
    std::println("    return \"<unknown>\";");
    std::println("  }}");
    std::println("  // enumToString(Color::Red) == \"Red\"  — fully compile-time!");
    std::println("");
    std::println("Herb Sutter: 'Reflection alone justifies upgrading to C++26.'");
    std::println("'It is the most powerful new engine for expressing efficient");
    std::println(" abstractions that C++ has ever had.'");
}


// =============================================================================
//  5. MEMORY SAFETY HARDENING
//
//  C++26 addresses memory safety in two ways:
//
//  a) ERRONEOUS BEHAVIOUR for uninitialised reads
//     In C++98–C++23, reading an uninitialised variable was Undefined Behaviour
//     (UB) — the compiler could do literally anything. C++26 changes this to
//     "Erroneous Behaviour": implementations must define what happens
//     (typically the variable holds a specified garbage value) and may
//     diagnose it. This is safer — no more silent nasal demons.
//
//  b) HARDENED STANDARD LIBRARY (library hardening mode)
//     Bounds-checked vector/span/string access. Null pointer validation.
//     Enable with: -D_GLIBCXX_ASSERTIONS (GCC), -D_LIBCPP_HARDENING_MODE=... (Clang)
//
//     At Google: hardened stdlib fixed >1000 bugs and reduced segfault rate
//     by 30% across their production fleet (Herb Sutter, CppCon 2025).
// =============================================================================

void demo_memory_safety() {
    std::println("\n--- Memory Safety Hardening (C++26) ---");

    // a) Erroneous behaviour for uninit reads
    // In C++26, this is no longer pure UB — it's defined as reading an
    // implementation-specific value (typically 0 or a poison value),
    // which implementations may diagnose.
    //
    // WARNING: Don't actually do this — shown for illustration only:
    // int x;   // uninit
    // use(x);  // C++26: erroneous (diagnosable), not pure UB

    std::println("Erroneous behavior (C++26 spec change):");
    std::println("  int x;    // uninitialized");
    std::println("  use(x);   // was UB in C++23; now 'erroneous behavior'");
    std::println("            // Implementations may define the value and diagnose");

    // b) Hardened library — test bounds checking
    std::println("\nHardened standard library:");
    std::println("  Compile with -D_GLIBCXX_ASSERTIONS to enable:");

    std::vector<int> v = {1, 2, 3};

    // With _GLIBCXX_ASSERTIONS, v[10] triggers an assertion failure at runtime
    // rather than silent undefined behaviour.
    std::println("  v[10] without hardening: silent UB (may crash, corrupt, or not)");
    std::println("  v[10] with   hardening:  assertion failure + stack trace");
    std::println("  v.at(10):                always throws std::out_of_range (C++98+)");

    // .at() is always bounds-checked — use it when safety matters
    try {
        int val = v.at(10);   // always safe — throws on out of bounds
        (void)val;
    } catch (const std::out_of_range& e) {
        std::println("  v.at(10) threw: {}", e.what());
    }

    std::println("\nTo enable full hardening in GCC:");
    std::println("  g++ -std=c++26 -D_GLIBCXX_ASSERTIONS your_file.cpp");
}


// =============================================================================
//  6. std::execution — Unified Async Framework (sender/receiver)
//
//  std::execution provides a framework for expressing structured concurrency.
//  Three core abstractions:
//    Scheduler  — where work runs (thread pool, GPU, I/O thread, ...)
//    Sender     — a description of async work to be done
//    Receiver   — what to do with the result (value, error, or cancellation)
//
//  Operators (similar to ranges |):
//    ex::schedule(sched)       — start on scheduler
//    ex::then(fn)              — chain: run fn with the result
//    ex::when_all(s1, s2)      — wait for multiple senders
//    ex::upon_error(fn)        — handle errors
//    ex::sync_wait(sender)     — block current thread until sender completes
//
//  This is a big topic. Already in production at Citadel for trading systems.
//  Requires: GCC 14+ with libexec (https://github.com/NVIDIA/stdexec)
//            until it's bundled in the stdlib
// =============================================================================

void demo_execution() {
    std::println("\n--- std::execution sender/receiver (C++26) ---");
    std::println("Requires: stdexec library (https://github.com/NVIDIA/stdexec)");
    std::println("          or GCC 16+ with bundled implementation");
    std::println("");

    std::println("Core concept:");
    std::println("  namespace ex = std::execution;");
    std::println("");
    std::println("  // A pipeline of async work:");
    std::println("  auto result = ex::sync_wait(");
    std::println("    ex::schedule(threadPool)");
    std::println("    | ex::then([]{{ return searchBestMove(board); }})");
    std::println("    | ex::then([](Move m){{ return evaluate(m); }})");
    std::println("  );");
    std::println("");
    std::println("  // Parallel tasks:");
    std::println("  auto both = ex::when_all(");
    std::println("    ex::schedule(pool) | ex::then(searchWhite),");
    std::println("    ex::schedule(pool) | ex::then(searchBlack)");
    std::println("  );");
    std::println("");
    std::println("Herb Sutter: 'std::execution is already in production");
    std::println("  at Citadel for an entire asset class and our messaging infra.'");
}


// =============================================================================
//  7. STRING INTERPOLATION (f-strings)
//
//  C++26 adds Python-style f-string literals with a literal f prefix.
//  Values are embedded inline in the string — the compiler expands them.
//
//  Status: In C++26 spec, compiler support rolling out.
//  Requires: GCC 16+  (experimental, check -std=c++26 support for this feature)
// =============================================================================

void demo_fstrings() {
    std::println("\n--- String Interpolation / f-strings (C++26) ---");

    // Standard std::format (C++20) — still works, not deprecated:
    std::string name = "Lorenzo";
    int   elo  = 1800;
    float eval = 0.35f;

    std::string msg = std::format("Player {} (Elo {}) eval: {:.2f}", name, elo, eval);
    std::println("std::format: {}", msg);

    // C++26 f-string syntax (when compiler supports it):
    // auto msg2 = f"Player {name} (Elo {elo}) eval: {eval:.2f}";
    // std::println("{}", msg2);

    std::println("\nC++26 f-string syntax:");
    std::println("  auto s = f\"Player {{name}} (Elo {{elo}}) eval: {{eval:.2f}}\";");
    std::println("  // Equivalent to: std::format(\"Player {{}} ...\", name, elo, eval)");
    std::println("  // More readable for inline expressions");
    std::println("  // Uses same format spec as std::format ({{:width.prectype}})");
    std::println("  // Available in GCC 16+ with -std=c++26");
}


// =============================================================================
//  SUMMARY TABLE
// =============================================================================

void printSummary() {
    std::println("\n--- C++26 Feature Support Summary ---");
    std::println("{:30} {:15} {}", "Feature", "GCC Version", "Status");
    std::println("{:30} {:15} {}", "-------", "-----------", "------");

    struct Row { std::string_view feature, gcc, status; };
    std::vector<Row> rows = {
        {"Contracts (pre/post)",   "GCC 15+",   "Available with -fcontracts"},
        {"#embed",                 "GCC 15+",   "Fully supported"},
        {"Pack indexing",          "GCC 15+",   "Fully supported"},
        {"Reflection (std::meta)", "GCC 16+",   "Experimental -freflection"},
        {"std::execution",         "GCC 14+",   "Needs stdexec or GCC 16"},
        {"Memory safety hdng.",    "GCC 14+",   "-D_GLIBCXX_ASSERTIONS"},
        {"f-strings",              "GCC 16+",   "Experimental"},
        {"std::simd",              "GCC 14+",   "Available"},
        {"Erroneous behavior",     "Spec only", "Compiler-defined"},
    };
    for (const auto& r : rows) {
        std::println("{:30} {:15} {}", r.feature, r.gcc, r.status);
    }

    std::println("\nTest C++26 features online at: https://godbolt.org");
    std::println("Select 'x86-64 gcc (trunk)' and use -std=c++26");
}


// =============================================================================
//  MAIN
// =============================================================================

int main() {
    std::println("======================================");
    std::println("  C++26 Feature Showcase");
    std::println("  WG21 shipped: March 28, 2026");
    std::println("======================================");

    demo_contracts();
    demo_embed();
    demo_pack_indexing();
    demo_reflection();
    demo_memory_safety();
    demo_execution();
    demo_fstrings();
    printSummary();

    return 0;
}
