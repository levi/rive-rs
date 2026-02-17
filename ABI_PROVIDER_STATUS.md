# ABI Provider Status

This file tracks the in-repo C/C++ ABI provider scaffold (feature:
`runtime-abi-provider`).

## Build path

- Build script: `build.rs`
- Handwritten provider core: `cpp/provider_core.cpp`
- Generated stubs: `${OUT_DIR}/rive_rs_provider_stubs.cpp`

## Implemented exports

- `rive_rs_abi_version`
- `rive_rs_factory_default`
- `rive_rs_factory_ref`
- `rive_rs_factory_unref`
- `rive_rs_map_xy`
- `rive_rs_ptr_to_file_asset`
- `rive_rs_ptr_to_audio_asset`
- `rive_rs_ptr_to_image_asset`
- `rive_rs_ptr_to_font_asset`

## Stub behavior

All other `RIVE_RS_API` symbols from `include/rive_rs_abi.h` are generated as
explicit stubs returning safe defaults (`RIVE_RS_STATUS_UNSUPPORTED`, null,
zero, or empty struct values).

## Next implementation layers

1. File import/load and basic `File` lifecycle.
2. Artboard lookup + lifecycle + width/height/name.
3. Animation/state-machine query surfaces.
4. View-model surfaces.
5. Asset decode + attachment surfaces.
