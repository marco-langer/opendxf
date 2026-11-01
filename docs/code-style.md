# Code style

Formatting is handled by `clang-format` with the repo's `.clang-format`. This page covers what it does not, and why. Design decisions are in [docs/adr](adr). Where a rule applies a [C++ Core Guideline](adr/0001-follow-cpp-core-guidelines.md), it links it instead of repeating its reasoning.

**Names are snake_case like the standard library's,** including types and enumerators; GoogleTest test names are PascalCase, see [ADR 6](adr/0006-standard-library-naming.md). Private data members start with `m_`. Qualify a type with `odxf::` where a variable has its name (`odxf::layer layer;`).

**Comments explain what the code cannot: DXF facts and project decisions,** e.g. what a group code means, why the reader skips something, or a DXF default. They do not explain general C++ or CMake knowledge, repeat what a test already shows, or refer to anything outside the repo. This applies to C++ and CMake files alike.

**Brace initialization** (`const int count{ 0 };`), see [ES.23](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#res-list).

**Scalar data members always have a default member initializer,** so no member is left uninitialized, see [ES.20](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#res-always) and [Type.6](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#pro-type-memberinit). **A class-type member has one only if it may be left out:** a `std::optional` gets `{}`, because `std::nullopt` stands for a value missing from the file. Other class-type members, e.g. coordinates or `properties`, get none, so GCC's `-Wmissing-field-initializers` reports a designated initializer that leaves them out. Name them explicitly, e.g. `.properties = {}`.

**Classes and structs are `final`** unless they are designed as a base, i.e. have a public virtual or a protected destructor like `read_stream_base`, see [ADR 7](adr/0007-classes-are-final.md).

**`maybe_x` names a `std::optional` or `std::expected` that has not been checked yet,** so it is clear at the point of use whether the value is known to be there.

**Reader helpers return `bool` and store the error in `m_error`.** Helpers like `read_next()` and `parse_into()` record the error once, where it happens, with the current line. Call sites stay short (`if (!parse_into(x)) { return make_error(); }`), which matters because the reader has many of them.

**File-local functions go in an anonymous namespace,** see [SF.22](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#rs-unnamed2).

**Own headers are included before standard headers,** so the build fails if an opendxf header is not self-contained ([SF.11](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#rs-contained)).

**The copyright year is the year the file was created.** It marks the first publication, so new files use the current year and existing files keep theirs.
