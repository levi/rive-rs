#![cfg(all(
    feature = "runtime-abi-provider",
    feature = "runtime-abi-provider-tests"
))]

use rive_rs::Factory;
use std::fs;
use std::path::PathBuf;

fn asset_bytes(name: &str) -> Vec<u8> {
    let path = PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .join("submodules/rive-runtime/tests/unit_tests/assets")
        .join(name);
    fs::read(&path).unwrap_or_else(|err| panic!("failed to read {}: {err}", path.display()))
}

#[test]
fn ref_unref_lifetime_paths_for_core_handles() -> Result<(), Box<dyn std::error::Error>> {
    let factory = Factory::new()?;
    let factory_clone = factory.clone();
    drop(factory_clone);

    let file = factory.load_file(&asset_bytes("viewmodel_runtime_file.riv"))?;
    let file_clone = file.clone();
    drop(file_clone);

    let mut artboard = file.default_artboard()?;
    let artboard_clone = artboard.clone();
    drop(artboard_clone);

    let view_model = file.view_model_by_index(0)?;
    let view_model_clone = view_model.clone();
    drop(view_model_clone);

    let view_model_instance = view_model.default_instance()?;
    let view_model_instance_clone = view_model_instance.clone();
    drop(view_model_instance_clone);

    // Original handles should remain valid after clone/drop churn.
    assert!(file.artboard_count() > 0);
    assert!(!artboard.name().is_empty());
    assert!(view_model.property_count() > 0);
    assert!(view_model_instance.property_count() > 0);
    let _ = artboard.advance(0.0)?;

    Ok(())
}
