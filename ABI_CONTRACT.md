# rive-rs ABI Contract

This document records the `rive-runtime` C ABI owned by `rive-rs`.

## Goals

- Expose `rive-runtime` access through a plain C ABI (`extern "C"`).
- Keep runtime FFI centralized in one crate.
- Provide ergonomic Rust wrappers so downstream code avoids raw C calls.

## Coverage in this contract

- Factory lifecycle
- File load and file lifecycle (+ optional asset-loader callbacks)
- File enum + audio metadata APIs
- Artboard access/advance/draw hooks + frame/audio/volume metadata
- Animation and state machine APIs + metadata APIs
- View model and bindable artboard APIs
- Path-based typed view-model value access (number/string/bool/color/enum/trigger/nested vm)
- View-model list and bindable-artboard property operations
- Event/report surfaces and listener/state-change queries
- Text value-run and path-based text/input access APIs
- Transform/node/bone/text-run query + mutation helpers
- Optional flat-path geometry query surface
- Matrix helper parity (`invert`, `multiply`) in safe Rust
- Core layout/math helpers (`compute_alignment`, `map_xy`)
- Audio/font decode and file-asset decode hooks
- File-asset metadata and pointer reinterpret helpers

## Explicitly excluded

- Renderer/WebGL2 C ABI (renderer commands, render image, rectanizer)
- JS harness bindings and package/build wiring
- ABI provider implementations against `rive-runtime`

## Contract artifacts

- C header: `include/rive_rs_abi.h`
- Rust mirror: `src/abi.rs`

## Notes

- The contract and safe Rust API now cover the core non-renderer binding
  surface from `wasm/src/bindings.cpp`.
- ABI provider implementation against `rive-runtime` is integration work and
  can be delivered separately.
