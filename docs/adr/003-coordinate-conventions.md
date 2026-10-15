# ADR-003: Coordinate and unit conventions

## Status

Accepted

## Context

Mesh processing code that mixes coordinate conventions or implicit unit assumptions produces subtle, hard-to-diagnose bugs (mirrored geometry, wrong-handed normals, scale mismatches).

## Decision

- All internal geometry uses a right-handed coordinate system with Z-up.
- The unit of length is millimeters (mm). Input files in other units must be converted at the import boundary.
- All tolerances are defined in `include/geomera/tolerances.hpp` with names, units, and explanatory constants. No bare epsilon values elsewhere in the codebase.
- Normals follow the counter-clockwise winding convention (vertices ordered CCW when viewed from the front face).

## Consequences

- File readers convert coordinate systems and units on import; writers convert back on export.
- Tolerance constants are centralized, documented, and tested.
- Algorithms can assume a consistent coordinate frame without per-call convention parameters.
- New developers check this ADR before introducing any tolerance or coordinate assumption.
