# 5. Errors are returned as values, not thrown

Status: accepted, under reconsideration (2026-10-28)

## Context

Reading fails on malformed or unsupported files, which are common in practice. The caller needs to know that reading failed and where in the file.

## Decision

The library does not throw. Functions that can fail return `std::expected<T, Error>`. `Error` carries the line number and a description.

## Rationale

- **A malformed file is an expected outcome, not an exceptional one.** A library that reads files written by other applications fails regularly, and handling that failure is part of the normal flow of the caller.
- **The caller cannot overlook the error.** opendxf is code that users include but have not written themselves. They cannot know which functions throw what without reading its sources. A returned error is part of the signature: the result cannot be used without checking it, and an unhandled error cannot crash the application that uses the library.

This deviates from Core Guidelines [E.2](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#re-throw) and [I.10](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#ri-except), which recommend exceptions for failures, see [ADR 1](0001-follow-cpp-core-guidelines.md).

## Consequences

- Every function inside the reader that can fail has to pass errors on to its caller. This makes the reader code noisy. Helpers like `parseInto` and `readEntity` reduce it, but do not remove it.
- `writeDxf()` does not report errors at all yet.

## Open questions

**How errors are passed on inside the reader.** The noise in the reader is the reason this decision is being reconsidered. One option is to keep `std::expected` at the public interface and use exceptions only inside the reader, caught once at its entry point. Both reasons above concern the public interface, so this option keeps them.

**Whether the reader should recover from errors.** Currently every parse error stops reading. Files written by other CAD applications are often slightly malformed, so a more forgiving reader may be more useful. Not every error allows that: a malformed value inside an entity could be skipped together with the entity, but a broken group code structure leaves the reader without a reliable point to continue from. A skipped entity is not a failure, so recovery also needs a way to report warnings to the caller.

The two questions are related: how errors are passed on inside the reader decides how easily it can recover at the entity level. If either is decided, a new ADR replaces this one.
