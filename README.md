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

opendxf has two APIs:

- The drawing API reads a whole file into an `odxf::drawing` and writes one, similar to a DOM.
- The stream API reads and writes one entity at a time, so the memory use doesn't grow with the number of entities. Reading calls a callback for each entity, similar to SAX.

### Drawings

Read a whole file into an `odxf::drawing`:

```cpp
const std::expected<odxf::drawing, odxf::error> drawing{ odxf::read("drawing.dxf") };
```

Fill an `odxf::drawing` and write it:

```cpp
#include <opendxf/opendxf.hpp>

#include <print>

int main()
{
    const odxf::drawing drawing{
        .header{ .entries{ { "$ACADVER", "AC1032" } } },
        .tables{
            .line_types{ { .name{ "CONTINUOUS" }, .display_name{ "Solid Line" } } },
            .layers{ { .name{ "Layer 1" } } },
        },
        .entities{
            odxf::circle{ .properties{}, .center{}, .radius{ 1.0 } },
            odxf::line{ .properties{}, .start{ 0.0, 0.0, 0.0 }, .end{ 1.0, 1.0, 0.0 } },
        },
    };

    const std::expected<void, odxf::error> result{ odxf::write("drawing.dxf", drawing) };
    if (!result) {
        std::println("error: {}", result.error().what);
        return EXIT_FAILURE;
    }
}
```

### Streams

To read a file, derive from `odxf::read_stream_base` and override the callbacks you are interested in. The reader calls them in the order of the file: once for the header, then for each table entry and each entity. This example adds up the lengths of the lines on the layer `Walls`:

```cpp
#include <opendxf/opendxf.hpp>

#include <cmath>
#include <cstdlib>
#include <expected>
#include <print>
#include <string>
#include <utility>

double length(const odxf::line& line)
{
    return std::hypot(
        line.end.x - line.start.x, line.end.y - line.start.y, line.end.z - line.start.z);
}

class line_length final : public odxf::read_stream_base
{
public:
    explicit line_length(std::string layer)
        : m_layer{ std::move(layer) }
    {
    }

    double total{ 0.0 };

private:
    void line(const odxf::line& line) override
    {
        if (line.properties.layer == m_layer) {
            total += length(line);
        }
    }

    std::string m_layer;
};

int main()
{
    line_length walls{ "Walls" };

    const std::expected<void, odxf::error> result{ odxf::read("drawing.dxf", walls) };
    if (!result) {
        std::println("error: {}", result.error().what);
        return EXIT_FAILURE;
    }

    std::println("length of the lines on layer Walls: {}", walls.total);
}
```

To write a file, open a `write_stream` with the header and tables and pass it the entities. `close()` completes the file and reports errors; if it isn't called, the destructor completes the file and the error is lost:

```cpp
std::expected<odxf::write_stream, odxf::error> stream{
    odxf::write_stream::open("drawing.dxf", header, tables)
};
if (!stream) {
    // handle stream.error()
}

for (const odxf::line& line : compute_lines()) {
    if (!stream->entity(line)) {
        // handle the error
    }
}

if (!stream->close()) {
    // handle the error
}
```

More complete programs are in [examples](examples), including one that copies the entities of one layer to a new file by combining both streams.

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
