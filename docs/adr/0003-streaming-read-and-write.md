# 3. Reading and writing stream, documents are built on the streams

Status: accepted

## Context

CAD files can be huge. A library that always holds the whole document in memory makes users pay for that even if they only need part of a file, e.g. the layers or a single entity type, or generate entities faster than they want to keep them.

DXF constrains what a writer can stream: the sections have a fixed order (HEADER, TABLES, BLOCKS, ENTITIES), each table is written as one block, and the writer derives the APPID table from all layers.

## Decision

Reading passes everything it reads, in file order, to an `IReadStream` implemented by the user: one hook per header, table entry and entity type. Hooks have empty default implementations, so users implement only the ones they need.

Writing takes the header and tables up front and returns a `WriteStream`. `WriteStream::entity()` appends one entity, `WriteStream::close()` ends and closes the file. Only entities are streamed.

Reading and writing a whole `Document` are built on the streams. Both directions use the same names, overloaded by what is passed:

| Whole document                  | Streaming                                             |
|---------------------------------|-------------------------------------------------------|
| `read(path)` returns a document | `read(path, stream)` calls an `IReadStream`           |
| `write(path, document)`         | `write(path, header, tables)` returns a `WriteStream` |

The path comes first in all of them. All of them return `std::expected` ([ADR 5](0005-errors-as-expected.md)).

## Rationale

- Memory use depends on what the user keeps, not on the size of the file.
- Users can filter or process entities while reading, without a second pass over a document, and copy a file entity by entity.
- Entities are the part of a file that grows. Header and tables are small and must be complete before the first entity anyway, so streaming them would only add a state machine with "table after entity" errors. Passing them to `write()` makes the section order a property of the API instead of a runtime check.
- The arguments already say whether a whole document or a stream is meant, so the function names don't have to.

## Consequences

- Every new entity type adds a hook to `IReadStream`.
- The writer writes the header as given. Header variables that depend on the entities, like `$EXTMIN`, `$EXTMAX` and `$HANDSEED`, are the caller's responsibility, because the header is written before the first entity.
- The destructor of `WriteStream` closes the file if `close()` wasn't called, like `std::ofstream`, and the error is lost. Callers who need to know whether the file was written completely call `close()`. A stream abandoned after an error also leaves a complete file, with the entities written so far.
- Calling `entity()` or `close()` after `close()` is an error.
