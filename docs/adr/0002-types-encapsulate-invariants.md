# 2. Types encapsulate only their invariants

Status: accepted

## Context

opendxf needs an in-memory representation of DXF objects: header, tables and entities. Entities share common properties (layer, color, line type, ...) and differ in their geometry.

A common design for this is a class hierarchy in which each entity type implements operations like reading, writing or drawing itself.

## Decision

A type encapsulates only its invariants. Everything else is a free function that operates on the type.

- A type without invariants, like most of the model, is plain data with public fields.
- A type with an invariant keeps its data private and offers only the member functions that maintain the invariant.
- All other operations, such as reading and writing DXF or the typed header accessors (`insertionUnits(header)`), are free functions.

Common properties of entities are in an `EntityProperties` struct that each entity holds as its `properties` member. Entities share no base class.

`Entity` is a `std::variant` of all entity types, and a document keeps its entities in one vector in file order, which is the default draw order in DXF. Operations on any entity use `std::visit`.

## Rationale

- This applies Core Guidelines [C.2](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#rc-struct) (a class for an invariant, a struct for independent data) and [C.4](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#rc-member) (a member function only if it needs access to the representation), see [ADR 1](0001-follow-cpp-core-guidelines.md).
- The set of entity types is fixed by the DXF format and grows slowly. The set of operations on them grows: reading, writing, and later e.g. bounding boxes or transformations. Keeping operations outside the model makes adding an operation cheap, which is the case we expect more often.
- The model knows nothing about DXF syntax. Users work with lines and circles, not with group codes.
- Entities can be built in one expression with named fields, which keeps user code and tests readable.

## Consequences

- Code shared by all entities has to be written once explicitly, e.g. `readEntityProperty` and `readEntity` in the reader. Without that, it is duplicated per entity type.
- Common properties are accessed through the member, e.g. `line.properties.layer`. The member also lets an entity including its common properties be built in one designated initializer, which C++23 doesn't allow for base classes. C++29 adds this for base classes (P2287); the member stays because the `Entity` variant needs no common base.
- Adding an entity type touches the model, the reader, the writer, `IReadStream`, the test matchers and the test data.
