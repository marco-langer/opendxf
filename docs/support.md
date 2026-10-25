# DXF support

opendxf covers a small subset of the DXF format. This page lists what can be read and written today.

## Sections

| Section  | Read | Write |
|----------|:----:|:-----:|
| HEADER   | ✓    | ✓     |
| CLASSES  | –    | –     |
| TABLES   | ✓    | ✓     |
| BLOCKS   | –    | –     |
| ENTITIES | ✓    | ✓     |
| OBJECTS  | –    | –     |

## Header variables

All header variables are read into and written from a generic key/value map. Typed accessors exist for:

| Variable    | Accessor                                    |
|-------------|---------------------------------------------|
| `$INSUNITS` | `insertionUnits()` / `setInsertionUnits()`  |

## Tables

| Table | Read | Write |
|-------|:----:|:-----:|
| LAYER | ✓    | ✓     |
| LTYPE | –    | ✓     |

## Entities

| Entity     | Read | Write |
|------------|:----:|:-----:|
| ARC        | ✓    | ✓     |
| CIRCLE     | ✓    | ✓     |
| ELLIPSE    | –    | ✓     |
| LINE       | ✓    | ✓     |
| LWPOLYLINE | ✓    | ✓     |
| POINT      | –    | ✓     |
| RAY        | –    | ✓     |

Unsupported tables and entities are skipped when reading.

## Colors

| Group code | Meaning     | LAYER read | Entities read | Write |
|------------|-------------|:----------:|:-------------:|:-----:|
| 62         | Color index | ✓          | ✓             | ✓     |

Entities read colors for ARC, CIRCLE, LINE and LWPOLYLINE. A missing group code 62 means 256
(BYLAYER) on entities and 7 on layers. A negative layer color means the layer is off.

## Known limitations

- Only ASCII DXF is supported, binary DXF is not.
- The reader expects the sections in the order HEADER, TABLES, BLOCKS, ENTITIES. Files with CLASSES or OBJECTS sections are rejected.
- Files without a HEADER section are rejected.
- The BLOCKS section is skipped when reading and written empty.
- `writeDxf()` does not report errors.
