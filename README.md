# opendxf

A small C++23 library for reading and writing ASCII DXF files.

opendxf is at an early stage and supports only a subset of DXF. See [DXF support](docs/support.md) for what can be read and written.

## Requirements

- A C++23 compiler
- CMake 4.3.4 or newer

The library has no dependencies. The tests use [GoogleTest](https://github.com/google/googletest): an installed GoogleTest is used if CMake can find it, otherwise it is downloaded during configuration.

## Integration

Add opendxf as a subdirectory, e.g. with `FetchContent`, and link against the `opendxf` target:

```cmake
include(FetchContent)

FetchContent_Declare(
    opendxf
    GIT_REPOSITORY https://github.com/marco-langer/opendxf.git
    GIT_TAG main
)
FetchContent_MakeAvailable(opendxf)

target_link_libraries(my_app PRIVATE opendxf)
```

Tests and examples are only built when opendxf is the top-level project. Use `OPENDXF_BUILD_TESTS` and `OPENDXF_BUILD_EXAMPLES` to override this.

## Usage

### Reading

Derive from `odxf::IReadStream` and override the callbacks you are interested in:

```cpp
#include <opendxf/opendxf.hpp>

#include <print>

class LineCounter final : public odxf::IReadStream
{
public:
    int lines{ 0 };

private:
    void line(const odxf::Line& /* line */) override { lines += 1; }
};

int main()
{
    LineCounter counter;

    const std::expected<void, odxf::Error> result{ odxf::read("drawing.dxf", counter) };
    if (!result) {
        std::println("error: {}", result.error().what);
        return EXIT_FAILURE;
    }

    std::println("lines: {}", counter.lines);
}
```

### Writing

Fill an `odxf::Document` and write it:

```cpp
#include <opendxf/opendxf.hpp>

#include <print>

int main()
{
    const odxf::Document document{
        .header{ .entries{ { "$ACADVER", "AC1032" } } },
        .tables{
            .lineTypes{ { .name{ "CONTINUOUS" }, .displayName{ "Solid Line" } } },
            .layers{ { .name{ "Layer 1" } } },
        },
        .entities{
            odxf::Circle{ .center{}, .radius{ 1.0 } },
            odxf::Line{ .start{ 0.0, 0.0, 0.0 }, .end{ 1.0, 1.0, 0.0 } },
        },
    };

    const std::expected<void, odxf::Error> result{ odxf::write("drawing.dxf", document) };
    if (!result) {
        std::println("error: {}", result.error().what);
        return EXIT_FAILURE;
    }
}
```

To write entities without holding them all in memory, open a `WriteStream` with the header and tables and stream the entities. `close()` completes the file and reports errors; if it isn't called, the destructor completes the file and the error is lost:

```cpp
std::expected<odxf::WriteStream, odxf::Error> stream{
    odxf::WriteStream::open("drawing.dxf", header, tables)
};
if (!stream) {
    // handle stream.error()
}

for (const odxf::Line& line : computeLines()) {
    if (!stream->entity(line)) {
        // handle the error
    }
}

if (!stream->close()) {
    // handle the error
}
```

More complete programs are in [examples](examples).

## Building and testing

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

Replace `debug` with `release` for an optimized build.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). The reasons behind the library's design are recorded in [docs/adr](docs/adr).

## License

opendxf is licensed under the [MIT License](LICENSE).
