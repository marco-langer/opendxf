# 7. Classes are final unless designed as a base

Status: accepted

## Context

A class that is not designed as a base breaks in a derived class, and how depends on the kind of class:

- **Polymorphic classes:** implementation inheritance couples a derived class to details of its base that the base never promised to keep: which virtual functions call which, in what order, and what protected state means. This is the fragile base class problem.
- **Value types:** a derived class can violate the value semantics of its base (Effective Java, Item 10). Operations defined by the base, such as equality, hashing and copying through the base type, cannot account for the state the derived class adds. In C++, copying or passing a derived object by value as its base slices it ([C.67](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#rc-copy-virtual)), and deleting it through a pointer to a base without a virtual destructor is undefined behavior ([C.35](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#rc-dtor-virtual)). Most opendxf types are value types, e.g. the entities and `header`.

The established answer to both is to design and document a class for inheritance or else prohibit it (Effective Java, Item 19), and to prefer composition otherwise.

Newer languages encode this in their defaults. Kotlin classes are closed unless marked `open`. Swift classes can only be subclassed outside their module if they are marked `open`. Java added `sealed` classes. Rust and Go have no implementation inheritance at all.

C++ takes the opposite default: classes are open and `final` is the opt-out. Core Guideline [C.139](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#rh-final) recommends using `final` sparingly. Its note limits the rule to classes with virtual functions that are meant as interfaces of a hierarchy, and says that not every class is meant as a base, e.g. `std::vector`.

## Decision

Every class and struct is `final` unless it is designed as a base. This includes aggregates, tag types and types internal to the library, its tests and its examples, and the leaf classes of a hierarchy, e.g. `document_stream` and the streams in the examples.

A class designed as a base is one of:

- **A polymorphic base,** with a public virtual destructor, e.g. `read_stream_base`. The GoogleTest fixtures are bases too, because GoogleTest derives each test from its fixture. They get a public virtual destructor from `testing::Test` without declaring one.
- **A non-polymorphic base,** e.g. a mixin, with a protected non-virtual destructor, so it cannot be deleted through a pointer to it. opendxf has none today.

The destructor is a review check, not a definition: a class without such a destructor, declared or inherited, is not designed as a base and has to be `final`. A virtual destructor alone does not make a class a base; `document_stream` has one and is final.

Inheritance is for interfaces, like `read_stream_base`, whose virtual functions are the points where users extend opendxf ([C.129](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#rh-kind)). Implementation is reused through composition.

For value types, this is what C.139's note describes. Making the leaf classes of a hierarchy `final` deviates from C.139, see [ADR 1](0001-follow-cpp-core-guidelines.md).

## Rationale

- **Types are not misused as a base.** Deriving from a class that is not designed for it compiles and fails only later, through broken equality, slicing or undefined behavior. `final` turns that into a compile error.
- **Leaf classes are not designed as bases either.** A class like `document_stream` overrides the virtual functions of its base for its own purpose. A class derived from it depends on those overrides, which is the fragile base class problem again.
- **Implementation details stay free to change.** A class that cannot be derived from has no hidden contract with derived classes, also once it gains virtual functions or protected members.
- **Extension points are explicit.** The few types designed as a base stand out and follow the rules for bases. Everything else is extended by composition: users wrap an opendxf type instead of deriving from it.
- **The decision can be relaxed later.** Removing `final` is source-compatible; adding it once users derive from a type is not. Removing it can still change the layout of other types, see Consequences.

## Alternatives

- **Follow C.139 and leave classes open.** Nothing would then tell a type designed as a base from one that is not, and deriving from a type without a virtual destructor would compile.
- **Make only value types final and leave leaf classes open,** as C.139 asks. Two rules instead of one, and the leaf classes are no more designed as bases than the value types.
- **Make only the public API types final.** Two rules instead of one, and an internal type that becomes public has to be remembered and changed.

## Consequences

- Inheritance that is not about polymorphism, e.g. mixins or overload sets (`struct overload : Ts...`), cannot use opendxf types as bases. This is accepted: such types are extended by composition, see above.
- libstdc++ stores an empty function object of a container without using space for it, through `[[no_unique_address]]` or the empty base optimization, but for ABI compatibility only if it is not final. So with libstdc++, the `std::unordered_map` in `header` is 8 bytes larger because `header_key_hash` is final; other standard libraries may differ. opendxf cannot change this other than by removing `final`, which would in turn change the layout of `header`.
