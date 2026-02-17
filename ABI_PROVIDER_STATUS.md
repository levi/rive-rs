# ABI Provider Status

This file tracks the in-repo C/C++ ABI provider implementation (feature:
`runtime-abi-provider`).

## Build path

- Build script: `build.rs`
- Provider implementation: `cpp/provider_core.cpp`

## Export coverage

All `RIVE_RS_API` symbols declared in `include/rive_rs_abi.h` are implemented
in `cpp/provider_core.cpp`.

## Notes

- There are no generated ABI stubs anymore.
- `FlattenedPath` functions are conditionally available behind
  `ENABLE_QUERY_FLAT_VERTICES`; when disabled they return
  `RIVE_RS_STATUS_UNSUPPORTED` (or zero for `length`).
