# 4. The library depends only on the C++23 standard library

Status: accepted

## Context

Every dependency of a library becomes a dependency of every project that uses it, with its own build setup, versions and licenses.

## Decision

opendxf uses only the C++23 standard library. GoogleTest is used for the tests and is not needed to build or use the library.

## Rationale

- Users can add opendxf to their project without pulling in anything else.
- C++23 is needed for `std::expected` (see [ADR 5](0005-errors-as-expected.md)), and it brings what a DXF library needs: text formatting, variants and locale-independent number parsing.

## Consequences

- Users need a compiler and standard library with C++23 support.
- Functionality that a third-party library would provide, such as parsing helpers, is written in opendxf.
