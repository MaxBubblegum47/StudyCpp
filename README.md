# StudyCpp

> A personal C++ study lab — annotated examples, experiments, and reference code organised by topic.
> Written by Lorenzo ([@MaxBubblegum47](https://github.com/MaxBubblegum47)).

---

## What this is

This repo is a living notebook. Every folder is a self-contained topic with runnable code, verbose comments, and compile instructions. The goal is to have a place where I can look something up quickly, experiment, and keep track of things I've actually understood rather than just read once.

If you stumbled here and find it useful — great. It's primarily written for future-me.

---

## Structure

```
StudyCpp/
├── fundamentals/       ← C++98 through C++26, one file per standard
├── ...                 ← more topics coming
└── README.md
```

---

## Modules

### `fundamentals/` — The C++ Standards Tour

A verbose, heavily commented walkthrough of every major C++ standard release, from the 1998 foundation to the C++26 revolution. Each file is standalone and runnable.

| File | Standard | Highlights |
|------|----------|------------|
| [`c98.cpp`](fundamentals/c98.cpp) | C++98 | Namespaces, templates, STL, exceptions, Rule of Three |
| [`c03.cpp`](fundamentals/c03.cpp) | C++03 | Value initialisation fix, `vector<bool>` quirks |
| [`c11.cpp`](fundamentals/c11.cpp) | C++11 | `auto`, lambdas, move semantics, smart pointers, threads |
| [`c14.cpp`](fundamentals/c14.cpp) | C++14 | Generic lambdas, init-captures, `make_unique`, variable templates |
| [`c17.cpp`](fundamentals/c17.cpp) | C++17 | Structured bindings, `optional`, `variant`, `filesystem` |
| [`c20.cpp`](fundamentals/c20.cpp) | C++20 | Concepts, Ranges, `format`, `<=>`, `span`, `jthread` |
| [`c23.cpp`](fundamentals/c23.cpp) | C++23 | `expected`, `print`, deducing `this`, multidim `[]`, ranges additions |
| [`c26.cpp`](fundamentals/c26.cpp) | C++26 | Contracts, reflection, `#embed`, pack indexing, memory safety |

Quick compile — all tested on **GCC 13.3 / Ubuntu 24.04**:

```bash
g++ -std=c++98 -Wall -o c98 c98.cpp && ./c98
g++ -std=c++03 -Wall -o c03 c03.cpp && ./c03
g++ -std=c++11 -Wall -o c11 c11.cpp && ./c11
g++ -std=c++14 -Wall -o c14 c14.cpp && ./c14
g++ -std=c++17 -Wall -o c17 c17.cpp && ./c17
g++ -std=c++20 -Wall -o c20 c20.cpp && ./c20
g++ -std=c++23 -Wall -o c23 c23.cpp && ./c23   # <print> shim included for GCC 13
g++ -std=c++23 -Wall -o c26 c26.cpp && ./c26   # C++26 features documented; needs GCC 16 to run fully
```

> **C++26 note:** `c26.cpp` compiles on GCC 13 with `-std=c++23` — the file is a documented showcase. To run the real features (contracts, reflection, `#embed`), install GCC 16:
> ```bash
> sudo add-apt-repository ppa:ubuntu-toolchain-r/test
> sudo apt install gcc-16 g++-16
> g++-16 -std=c++26 -freflection -Wall -o c26 c26.cpp && ./c26
> ```
> Or try everything online at [godbolt.org](https://godbolt.org) — select *x86-64 gcc (trunk)* and `-std=c++26`.

---

## Philosophy

- **Verbose over terse.** Comments explain the *why*, not just the what. If it's obvious, it probably doesn't need a comment. If it's subtle, it definitely does.
- **Chess as the domain.** Examples use chess concepts (pieces, squares, move generation) as a concrete, consistent domain rather than abstract `foo`/`bar` placeholders. It keeps things grounded and will eventually feed into a chess engine project.
- **One concept at a time.** Each demo function covers a single feature so it's easy to `Ctrl+F` and find exactly what you need.
- **Compile first, understand second.** Every file must compile clean (`-Wall`) before it lives here.

---

## Roadmap

- [ ] `fundamentals/` — C++98 → C++26 ✅
- [ ] `patterns/` — Design patterns in modern C++ (CRTP, policy-based design, type erasure, ...)
- [ ] `concurrency/` — Threads, atomics, lock-free structures, `std::execution`
- [ ] `templates/` — Template metaprogramming, SFINAE, concepts deep-dive
- [ ] `chess-engine/` — Putting it all together

---

*Built one standard at a time.*

*P.s.*
For creating this repo I have exploited claude code. I will probably keep using that for boosting the development of examples and code production. Cheers from Italy