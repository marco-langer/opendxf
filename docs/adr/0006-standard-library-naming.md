# 6. Names follow the standard library's style

Status: accepted

## Context

opendxf depends only on the standard library ([ADR 4](0004-standard-library-only.md)), so user code that calls opendxf mostly calls the standard library around it. Mixed naming styles in one expression, like `document.tables.lineTypes.push_back(...)`, make it harder to read.

## Decision

All names are snake_case, like the standard library's ([NL.10](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#rl-underscore)): types, functions, variables, enumerators, constants, concepts and file names. Template parameters stay PascalCase (`typename... Ts`), as in the standard. Private data members keep the `m_` prefix.

The abstract base class of the read stream is named `read_stream_base`, after `std::ios_base` and `std::streambuf`, instead of carrying an `I` prefix.

## Rationale

- Code that uses opendxf reads like code that uses the standard library.
- One style for everything leaves no rule about which kind of name gets which case.

## Consequences

- A type and a variable can no longer differ by case alone. Where a declaration names a variable like its type, the type is qualified: `odxf::header header;`, `void header(const odxf::header& header)`. GCC reports a member that changes the meaning of a type name (`-Wchanges-meaning`).
- A nested enum can't share its name with the member that holds it. The error kind is the namespace-level `error_type`, and the layer flags enum is `layer::flag`, held in `layer::flags`.
- GoogleTest test suites and tests are snake_case too, although GoogleTest advises against underscores in them. Two tests could only collide if their combined names were equal, which the compiler reports as a redefinition.
