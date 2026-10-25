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
| `$LTSCALE`  | `lineTypeScale()` / `setLineTypeScale()`    |

## Tables

| Table | Read | Write |
|-------|:----:|:-----:|
| LAYER | ✓    | ✓     |
| LTYPE | ✓    | ✓     |

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
| 420        | True color  | ✓          | ✓             | ✓     |

Entities read colors for ARC, CIRCLE, LINE and LWPOLYLINE. A missing group code 62 means 256
(BYLAYER) on entities and 7 on layers. A negative layer color means the layer is off.
A true color (`0x00RRGGBB`) takes precedence over the color index, which files keep as a fallback
for older readers. Color names (group code 430) are not supported.

## Lineweights

| Group code | Meaning    | LAYER read | Entities read | Write |
|------------|------------|:----------:|:-------------:|:-----:|
| 370        | Lineweight | ✓          | ✓             | ✓     |

Entities read lineweights for ARC, CIRCLE, LINE and LWPOLYLINE. Lineweights are in hundredths of
a millimeter. Entities can also be -1 (BYLAYER), -2 (BYBLOCK) or -3 (DEFAULT), layers -3
(DEFAULT). DEFAULT means the default lineweight of the application; DXF files don't store it. A
missing group code 370 means -1 on entities and -3 on layers. Values are stored as read, without
checking them against the lineweights DXF allows.

## Line types

| Group code | Meaning                       | LTYPE read | LAYER read | Entities read | Write |
|------------|-------------------------------|:----------:|:----------:|:-------------:|:-----:|
| 3          | Description                   | ✓          |            |               | ✓     |
| 49         | Dash, gap or dot length       | ✓          |            |               | ✓     |
| 74         | Complex element flags         | ✓          |            |               | ✓     |
| 6          | Line type name                |            | ✓          | ✓             | ✓     |
| 48         | Line type scale               |            |            | ✓             | ✓     |

Entities read line types for ARC, CIRCLE, LINE and LWPOLYLINE. Lengths are in drawing units: a
positive length is a dash, a negative one a gap and 0 a dot. The total pattern length (40) is
computed when writing and not stored. A missing group code 6 means BYLAYER on entities and
CONTINUOUS on layers; a missing 48 means 1.0.

Complex elements, which embed a text or a shape, only keep their flags (74). Their details (75,
340, 46, 50, 44, 45 and 9) are skipped when reading and not written, so a written complex line
type is incomplete.

## Known limitations

- Only ASCII DXF is supported, binary DXF is not.
- The reader expects the sections in the order HEADER, TABLES, BLOCKS, ENTITIES. Files with CLASSES or OBJECTS sections are rejected.
- Files without a HEADER section are rejected.
- The BLOCKS section is skipped when reading and written empty.
- `writeDxf()` does not report errors.
