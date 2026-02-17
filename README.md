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
- Runtime ABI provider implementation now lives in this repo behind
  `runtime-abi-provider`

## Main files

- C header: `include/rive_rs_abi.h`
- Raw Rust ABI: `src/abi.rs`
- Safe wrappers: `src/runtime.rs`
- ABI notes: `ABI_CONTRACT.md`
- Parity checklist: `PARITY_CHECKLIST.md`
- Provider status: `ABI_PROVIDER_STATUS.md`

## Runtime submodule

This repo tracks `rive-runtime` as a git submodule at:

- `submodules/rive-runtime`

Clone/update with:

```bash
git submodule update --init --recursive
```

## ABI provider

`rive-rs` includes an opt-in ABI provider that builds C/C++ exports:

```bash
cargo build --features runtime-abi-provider
```

Current provider status:

- All `RIVE_RS_API` symbols in `include/rive_rs_abi.h` are implemented in
  `cpp/provider_core.cpp`
- No generated ABI stubs are used
- `FlattenedPath` APIs are gated by `ENABLE_QUERY_FLAT_VERTICES` in
  `rive-runtime` (unsupported when that macro is off)

## Tests

- Default:
  - `cargo check`
  - `cargo test`
- Provider-enabled:
  - `cargo check --features runtime-abi-provider`
  - `cargo test --features runtime-abi-provider`
- Provider + runtime smoke/lifetime/callback-asset coverage:
  - `cargo check --features \"runtime-abi-provider runtime-abi-provider-tests\"`
  - `cargo test --features \"runtime-abi-provider runtime-abi-provider-tests\"`
