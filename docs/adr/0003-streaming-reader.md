# 3. The reader streams what it reads to user-supplied hooks

Status: accepted

## Context

CAD files can be huge. A reader that always builds the whole document in memory makes users pay for that even if they only need part of the file, e.g. the layers or a single entity type.

## Decision

`read()` does not return a document. It passes everything it reads, in file order, to an `IReadStream` implemented by the user: one hook per header, table entry and entity type. Hooks have empty default implementations, so users implement only the ones they need.

## Rationale

- Memory use depends on what the user keeps, not on the size of the file.
- Users can filter or process entities while reading, without a second pass over a document.
- Building a complete `Document` is just one possible use of the stream.

## Consequences

- The library has no function that returns a `Document`. Users who want one write a stream that collects everything, like `ReadStream` in the tests.
- Every new entity type adds a hook to the public interface.
- Writing is not symmetric: `writeDxf()` takes a complete `Document`.
