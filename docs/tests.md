# Tests

The tests use GoogleTest and GoogleMock and are registered with CTest. Run them with the release preset:

```sh
cmake --build --preset release
ctest --preset release
```

## Layout

- `tests/test_utils.hpp` has `create_example_document()`, the expected content of `tests/data/valid/example.dxf`. Tests read files into a `document` with `odxf::read(path)`.
- Test data is in `tests/data/valid/` (files the reader must accept) and `tests/data/invalid/` (files it must reject).
- New test files and matchers must be added to `tests/CMakeLists.txt`.
- Tests can include the library's private headers in `opendxf/src/`, e.g. `overload.hpp`.

## Conventions

- Structure each test with `// Arrange`, `// Act` and `// Assert` comments.
- Name test suites, tests and fixtures in PascalCase, as GoogleTest does: `TEST(Subject, Behavior)`, e.g. `TEST(InsertionUnits, WrongType)`. GoogleTest joins them with underscores into class names, so underscores in them could collide.
- Use `TEST_P` for tests that differ only in their data. Put the parameters in a struct and instantiate them with designated initializers, as in `parse_error_param` in `tests/read_test.cpp`.
- Compare documents and entities with the matchers (`is_document`, `is_header`, ...) instead of field-by-field assertions in the test.
- A bug fix first adds its test with `GTEST_SKIP()` in a commit of its own, see the [definition of done](../CONTRIBUTING.md#definition-of-done).

## What is already covered

Avoid tests that only repeat existing coverage:

- `Read.Example` compares the whole document read from `example.dxf` with `create_example_document()`, including all header variables, tables and entities.
- `Write.Example` writes `create_example_document()`, reads it back and compares it the same way.
- Header accessors are tested on `header` values built in the test, not on files.

When a field is added to the model, extend `create_example_document()`, `example.dxf` and the matcher for that type. A matcher that does not compare a field lets regressions in that field pass silently.
