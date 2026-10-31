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
- A design decision, e.g. about the model, the structure of the reader or writer, error handling, dependencies or naming, is recorded in a new [ADR](docs/adr) or in an update of the existing one.

## Commits

- Each commit is one logical change that builds and passes the tests.
- The subject is `<type>: <summary>`, e.g. `fix: accept CRLF line endings in the reader`. The type is one of:
  - `feat`: a new feature
  - `fix`: a bug fix
  - `refactor`: a code change that neither adds a feature nor fixes a bug
  - `docs`: documentation only
  - `test`: tests only
  - `build`: CMake, presets and other build configuration
  - `chore`: anything else, e.g. repository configuration
- A change that breaks the public API adds a `!` after the type, e.g. `refactor!: rename functions to snake case`, and a `BREAKING CHANGE:` line in the body that says what users have to change.
- The summary is a lowercase imperative of about 50 characters (at most 72 for the whole subject) without a trailing period.
- After a blank line, the body explains why the change is needed and, where it isn't obvious from the diff, how it works: the problem or use case, the relevant DXF behavior, and alternatives that were rejected. Wrap it at 72 characters.
- Leave out the body only when the subject says everything, e.g. `docs: fix a typo in the README`.

## Pull requests

- Branches are named `<type>/<short-description>` with the commit types above, e.g. `fix/reader-crlf`.
- Titles follow the commit subject format and describe the whole change, e.g. `feat: read and write the visibility of entities`.
- The description follows the [pull request template](.github/pull_request_template.md).
