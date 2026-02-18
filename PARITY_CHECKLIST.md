# rive-rs Parity Checklist

This tracks parity against required surfaces from `wasm/src/bindings.cpp` and
`wasm/src/bindings_webgl2.cpp`.

- [x] Top-level core helpers (`load`, optional asset loader, `decodeAudio`, `decodeFont`,
      `computeAlignment`, `mapXY`, listener checks)
- [x] File/artboard/view-model lookup + bindable artboard flow
- [x] File enums + file/audio metadata
- [x] Artboard metadata (`name`, `frameOrigin`, `hasAudio`, `volume`, reset size)
- [x] Transform/node/bone/root-bone/text-run APIs
- [x] Flat path query surface
- [x] Linear animation/state-machine access
- [x] Linear animation metadata + `apply`
- [x] Linear animation instance metadata (`didLoop`) + control methods
- [x] State machine instance pointer/input/event/state-change APIs
- [x] `SMIInput` type/name + typed conversions
- [x] View model and view model instance typed/path/list operations
- [x] File-asset metadata + decode/audio/font attachment hooks
- [x] File-asset pointer reinterpret helpers (`ptrTo*Asset`)
- [x] WebGL2/WebGPU renderer lifecycle commands
- [x] Render-image decode/ref/unref hooks
- [x] Image asset + view-model image property render-image attachment hooks

Out of scope for `rive-rs`:

- `DynamicRectanizer`
