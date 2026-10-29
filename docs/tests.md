# Tests

The tests use GoogleTest and GoogleMock and are registered with CTest. Run them with the release preset:

```sh
cmake --build --preset release
ctest --preset release
```

## Layout

- `tests/test_utils.hpp` has `createExampleDocument()`, the expected content of `tests/data/valid/example.dxf`. Tests read files into a `Document` with `odxf::read(path)`.
- Test data is in `tests/data/valid/` (files the reader must accept) and `tests/data/invalid/` (files it must reject).
- New test files and matchers must be added to `tests/CMakeLists.txt`.
- Tests can include the library's private headers in `opendxf/src/`, e.g. `overload.hpp`.

## Conventions

- Structure each test with `// Arrange`, `// Act` and `// Assert` comments.
- Name tests `TEST(subject, behavior)`, e.g. `TEST(insertionUnits, wrongType)`.
- Use `TEST_P` for tests that differ only in their data. Put the parameters in a struct and instantiate them with designated initializers, as in `ParseErrorParam` in `tests/read_test.cpp`.
- Compare documents and entities with the matchers (`IsDocument`, `IsHeader`, ...) instead of field-by-field assertions in the test.
- A bug fix comes with a test that fails without the fix.

## What is already covered

Avoid tests that only repeat existing coverage:

- `read.example` compares the whole document read from `example.dxf` with `createExampleDocument()`, including all header variables, tables and entities.
- `write.example` writes `createExampleDocument()`, reads it back and compares it the same way.
- Header accessors are tested on `Header` values built in the test, not on files.

When a field is added to the model, extend `createExampleDocument()`, `example.dxf` and the matcher for that type. A matcher that does not compare a field lets regressions in that field pass silently.
