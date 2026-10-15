# ADR-001: Use vcpkg manifest mode for dependency management

## Status

Accepted

## Context

The project needs reproducible dependency resolution across developer machines and CI. Options considered: Conan, vcpkg classic mode, vcpkg manifest mode, system packages.

## Decision

Use vcpkg in manifest mode with a pinned baseline in `vcpkg.json`. This keeps dependency versions committed alongside the source and requires no global vcpkg installation.

## Consequences

- All dependencies are declared in `vcpkg.json` with a single baseline hash.
- CI and local builds resolve the same versions automatically.
- Adding a dependency means editing `vcpkg.json` and re-running CMake configure.
