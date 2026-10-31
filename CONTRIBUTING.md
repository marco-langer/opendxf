# Contributing

Code conventions are in [docs/code-style.md](docs/code-style.md), test conventions in [docs/tests.md](docs/tests.md), and design decisions in [docs/adr](docs/adr).

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

## Pull requests

- Branches are named `<type>/<short-description>`, where type is `feature`, `fix`, `refactor`, `docs` or `chore`, e.g. `fix/reader-crlf`.
- Titles follow the commit subject style and describe the whole change, e.g. `read and write the visibility of entities`.
- The description follows the [pull request template](.github/pull_request_template.md).
