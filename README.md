# rive-rs

`rive-rs` is an experimental Rust-first crate for `rive-runtime` integration.

It currently contains:

- A core C ABI contract (`include/rive_rs_abi.h`)
- Rust `extern "C"` declarations (`src/abi.rs`)
- A safe Rust wrapper layer for core runtime types (`src/runtime.rs`)

## Scope

`rive-rs` owns **core runtime surfaces only**:

- file/artboard/animation/state-machine primitives
- core math/layout helpers
- audio/font decode primitives

Renderer-facing APIs are intentionally out of scope and are owned by renderer crates,
starting with `webgl2-rive-rs`.

## Current phase

- Core `bindings.cpp` runtime-surface parity completed at the contract level
- Safe Rust wrapper parity completed for the same core surface area
- Runtime ABI provider implementation is integration-specific and can be
  implemented separately against `rive-runtime`

## Main files

- C header: `include/rive_rs_abi.h`
- Raw Rust ABI: `src/abi.rs`
- Safe wrappers: `src/runtime.rs`
- ABI notes: `ABI_CONTRACT.md`
- Parity checklist: `PARITY_CHECKLIST.md`

## Runtime submodule

This repo tracks `rive-runtime` as a git submodule at:

- `submodules/rive-runtime`

Clone/update with:

```bash
git submodule update --init --recursive
```
