# fundamentals/ — The C++ Standards Tour

One `.cpp` file per standard, C++98 through C++26. Every file is standalone, self-contained, and runnable. The comments explain *why* each feature exists, what problem it solves, and what the code before it looked like.

The examples use chess as a consistent domain — pieces, squares, move generation — which keeps things concrete and less abstract than generic `Foo`/`Bar` examples.

---

## Files

### `c98.cpp` — The Foundation
The first ISO standard. Everything in modern C++ sits on this.
- Namespaces (`chess::`, `utils::`)
- Function and class templates (`maxOf<T>`, `Stack<T>`)
- STL containers and algorithms (`vector`, `map`, `list`, `sort`, `for_each`)
- Exceptions (`try`/`catch`/`throw`, `std::out_of_range`)
- `bool` type
- Rule of Three: destructor + copy constructor + copy assignment

### `c03.cpp` — The Maintenance Release
No new language features — strictly clarifications and bug fixes.
- Value initialisation: `T()` is now guaranteed to zero-initialise POD types
- `std::vector<bool>` proxy specialisation (and why it's a trap)
- `throw()` exception specification behaviour formalised

### `c11.cpp` — The Revolution
The biggest upgrade in C++ history (until C++26). Modern C++ starts here.
- `auto` and `decltype`
- Range-based for loops
- Lambdas with all capture modes (`[]`, `[=]`, `[&]`, `[x]`, `[&x]`)
- `nullptr`
- Move semantics and rvalue references (`T&&`)
- Smart pointers: `unique_ptr`, `shared_ptr`, `weak_ptr`
- `constexpr` (basic)
- `enum class` (scoped enums)
- `override` and `final`
- Uniform initialisation and `initializer_list`
- Variadic templates
- `std::thread`, `std::mutex`, `std::lock_guard`
- `std::chrono`
- `std::array`, `std::unordered_map`, `std::tuple`
- `static_assert`

### `c14.cpp` — The Polish
Quality-of-life fixes for C++11's rough edges.
- Generic lambdas (`auto` parameters)
- Lambda init-captures (enables moving into lambdas)
- Return type deduction for functions (`auto` return)
- Relaxed `constexpr` (loops and locals allowed)
- Binary literals (`0b1010'1010`) and digit separators (`1'000'000`)
- `std::make_unique` (embarrassingly missing from C++11)
- Variable templates (`pi<float>`, `pi<double>`)
- `[[deprecated]]` attribute

### `c17.cpp` — The Vocabulary Release
Not a revolution, but hugely useful for everyday code.
- Structured bindings (`auto [key, val] = ...`)
- `if`/`switch` with initialisers
- `std::optional<T>` — nullable types without sentinels
- `std::variant<Ts...>` + `std::visit` — type-safe unions
- `std::any` — type-erased single values
- `std::string_view` — non-owning string references
- `if constexpr` — compile-time branching in templates
- Fold expressions
- Class Template Argument Deduction (CTAD)
- Parallel STL algorithms
- `std::filesystem`
- Guaranteed copy elision (RVO mandatory)

### `c20.cpp` — The Four Pillars
Second revolution after C++11. Four headline features.
- **Concepts** — named type constraints, readable error messages, overload resolution
- **Ranges & Views** — lazy composable pipelines with `|` operator
- **Coroutines** — `co_await`, `co_yield`, `co_return`
- **Modules** — `import` replaces `#include` (build-system dependent)
- `std::format` — type-safe Python-style string formatting
- `<=>` spaceship operator — one definition generates all comparisons
- `std::span<T>` — non-owning view over contiguous data
- `consteval` and `constinit`
- Designated initialisers
- `std::jthread` and `std::stop_token`
- `std::bit_cast`
- `std::numbers` (π, e, φ, √2 at correct precision)
- `std::erase` / `std::erase_if`

### `c23.cpp` — The Refinement
ISO published December 2024. Polishes C++20 and adds immediately practical features.

> **GCC note:** `<print>` and `<flat_map>` require GCC 14+. This file includes a `std::println` shim so it compiles clean on GCC 13. `deducing this` needs GCC 14+; workarounds are shown.

- `std::print` / `std::println` — finally a proper print function
- `import std;` — entire stdlib as one module
- `std::expected<T, E>` — error handling without exceptions (Rust-style `Result`)
- Deducing `this` (explicit object parameter) — recursive lambdas, CRTP without boilerplate
- Multidimensional `operator[]` — `board[file, rank]`, `matrix[row, col]`
- `std::flat_map` / `std::flat_set` — cache-friendly sorted containers
- `std::move_only_function` — `std::function` that accepts move-only callables
- `std::byteswap`
- Ranges additions: `zip`, `chunk`, `slide`, `stride`, `ranges::to<T>`
- `std::stacktrace`

### `c26.cpp` — The New Era
WG21 shipped March 28, 2026. Herb Sutter: *"most compelling release since C++11."*

> **Compiler note:** Compiles with `-std=c++23` on GCC 13. Full features need GCC 16 with `-freflection`, or [godbolt.org](https://godbolt.org) (trunk gcc, `-std=c++26`).

- **Reflection (`std::meta`)** — zero-cost compile-time introspection. Herb Sutter calls it bigger than templates.
- **Contracts** (`pre`/`post`/`contract_assert`) — Design by Contract in the standard
- `std::execution` — unified async framework (sender/receiver)
- Memory safety hardening — erroneous behaviour for uninit reads, bounds-checked stdlib
- `#embed` — include binary resources at compile time
- Pack indexing (`args...[N]`)
- f-string interpolation
- `std::simd`

---

## Compile all

```bash
cd fundamentals/

g++ -std=c++98 -Wall -o c98 c98.cpp && ./c98
g++ -std=c++03 -Wall -o c03 c03.cpp && ./c03
g++ -std=c++11 -Wall -o c11 c11.cpp && ./c11
g++ -std=c++14 -Wall -o c14 c14.cpp && ./c14
g++ -std=c++17 -Wall -o c17 c17.cpp && ./c17
g++ -std=c++20 -Wall -o c20 c20.cpp && ./c20
g++ -std=c++23 -Wall -o c23 c23.cpp && ./c23
g++ -std=c++23 -Wall -o c26 c26.cpp && ./c26
```

Or as a one-liner:
```bash
for std in 98 03 11 14 17 20; do
  g++ -std=c++$std -Wall -o c$std c$std.cpp && ./c$std
done
g++ -std=c++23 -Wall -o c23 c23.cpp && ./c23
g++ -std=c++23 -Wall -o c26 c26.cpp && ./c26
```