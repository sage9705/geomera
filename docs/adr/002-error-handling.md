# ADR-002: Error handling with tl::expected

## Status

Accepted

## Context

Geometry operations can fail in expected ways (degenerate triangles, non-manifold edges, missing files). Exceptions add hidden control flow that is hard to reason about in numerical code. Return codes lose type information.

## Decision

Use `tl::expected<T, E>` as the primary error-reporting mechanism for all fallible operations. Define a project-wide `Error` type in `include/geomera/error.hpp` carrying an error code enum and an optional message.

Reserve exceptions for truly unrecoverable states (out of memory, violated invariants that indicate bugs).

## Consequences

- Every function that can fail returns `tl::expected<T, geomera::Error>`.
- Callers use monadic chaining or explicit checks — no try/catch needed.
- Error messages propagate without stack unwinding.
- Code compiled with `-fno-exceptions` remains viable for embedded or WASM targets.
