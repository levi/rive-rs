# rive-rs

`rive-rs` is an experimental Rust-first crate for `rive-runtime` integration.

It currently contains:

- A core C ABI contract (`include/rive_rs_abi.h`)
- Rust `extern "C"` declarations (`src/abi.rs`)
- A safe Rust wrapper layer for runtime and renderer-handle types (`src/runtime.rs`)

## Scope

`rive-rs` owns the ABI boundary to `rive-runtime`:

- file/artboard/animation/state-machine primitives
- WebGL2/WebGPU renderer lifecycle APIs
- render-image decode/ref/attach hooks
- core math/layout helpers
- audio/font decode primitives

Higher-level JS API shape and package integration remain owned by renderer crates
(for example `webgl2-rive-rs`).

## Current phase

- Core `bindings.cpp` runtime-surface parity completed at the contract level
- Renderer lifecycle + render-image ABI surface in place for Rust-only WebGL2 flows
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
