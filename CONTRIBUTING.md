# Contributing

## Definition of done

A change is done when:

- All touched C++ files are formatted with `clang-format` (the repo has a `.clang-format`).
- The release preset builds without warnings and all tests pass:

  ```sh
  cmake --preset release
  cmake --build --preset release
  ctest --preset release
  ```

- A bug fix comes with a test that fails without the fix.
- [DXF support](docs/support.md) is updated when the supported features or known limitations change.
- Commits are small and focused, with a lowercase imperative subject, e.g. `read the layer of LWPOLYLINE entities`.
