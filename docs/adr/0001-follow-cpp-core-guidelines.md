# 1. Follow the C++ Core Guidelines

Status: accepted

## Context

Many design questions in a C++ library have a common, well-reasoned answer. Writing our own rules for all of them would take effort and give a less consistent result.

## Decision

opendxf follows the [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines). Where it deviates from a guideline, the ADR that makes the deviation names the guideline and gives the reason.

## Rationale

- The guidelines are widely known, so contributors find familiar rules and their reasons in one place.
- Recording deviations keeps them deliberate: a deviation without a reason is a mistake to fix.

## Consequences

- Decisions that only apply a guideline cite it instead of repeating its reasoning.
- Known deviations:

  | Guideline     | Deviation                       | Reason                              |
  | ------------- | ------------------------------- | ----------------------------------- |
  | [E.2], [I.10] | Errors are returned, not thrown | [ADR 5](0005-errors-as-expected.md) |

[E.2]: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#re-throw
[I.10]: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#ri-except
