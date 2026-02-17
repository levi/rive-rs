#![cfg(all(
    feature = "runtime-abi-provider",
    feature = "runtime-abi-provider-tests"
))]

use rive_rs::abi;
use rive_rs::{
    Aabb, Alignment, DataType, Factory, FileAssetLoaderCallbacks, Fit, LinearAnimationInstance,
    SmiInputType, StateMachineInstance, Vec2, compute_alignment, map_xy,
};
use std::ffi::c_void;
use std::fs;
use std::path::PathBuf;
use std::sync::atomic::{AtomicBool, AtomicI32, AtomicUsize, Ordering};

static LOADER_CALLBACK_CALLS: AtomicUsize = AtomicUsize::new(0);
static ASSET_MUTATOR_CALLS: AtomicUsize = AtomicUsize::new(0);
static SAW_IMAGE_ASSET: AtomicBool = AtomicBool::new(false);
static SAW_AUDIO_ASSET: AtomicBool = AtomicBool::new(false);
static SAW_FONT_ASSET: AtomicBool = AtomicBool::new(false);
static IMAGE_DECODE_STATUS: AtomicI32 =
    AtomicI32::new(abi::rive_rs_status::RIVE_RS_STATUS_RUNTIME_ERROR as i32);
static AUDIO_SET_STATUS: AtomicI32 =
    AtomicI32::new(abi::rive_rs_status::RIVE_RS_STATUS_RUNTIME_ERROR as i32);
static FONT_SET_STATUS: AtomicI32 =
    AtomicI32::new(abi::rive_rs_status::RIVE_RS_STATUS_RUNTIME_ERROR as i32);

const STATUS_OK: i32 = abi::rive_rs_status::RIVE_RS_STATUS_OK as i32;

unsafe extern "C" fn test_asset_loader(
    _user_data: *mut c_void,
    _asset: *mut abi::rive_rs_file_asset,
    _in_band_bytes: abi::rive_rs_bytes_view,
    _factory: *mut abi::rive_rs_factory,
) -> bool {
    LOADER_CALLBACK_CALLS.fetch_add(1, Ordering::Relaxed);
    false
}

unsafe extern "C" fn asset_mutating_loader(
    _user_data: *mut c_void,
    asset: *mut abi::rive_rs_file_asset,
    in_band_bytes: abi::rive_rs_bytes_view,
    factory: *mut abi::rive_rs_factory,
) -> bool {
    ASSET_MUTATOR_CALLS.fetch_add(1, Ordering::Relaxed);

    if asset.is_null() {
        return false;
    }

    if unsafe { abi::rive_rs_file_asset_is_image(asset) } {
        SAW_IMAGE_ASSET.store(true, Ordering::Relaxed);
        if !factory.is_null() && !in_band_bytes.ptr.is_null() && in_band_bytes.len > 0 {
            let status = unsafe { abi::rive_rs_file_asset_decode(factory, asset, in_band_bytes) };
            IMAGE_DECODE_STATUS.store(status as i32, Ordering::Relaxed);
            return true;
        }
        return false;
    }

    if unsafe { abi::rive_rs_file_asset_is_audio(asset) } {
        SAW_AUDIO_ASSET.store(true, Ordering::Relaxed);
        let status =
            unsafe { abi::rive_rs_audio_asset_set_audio_source(asset, std::ptr::null_mut()) };
        AUDIO_SET_STATUS.store(status as i32, Ordering::Relaxed);
        return true;
    }

    if unsafe { abi::rive_rs_file_asset_is_font(asset) } {
        SAW_FONT_ASSET.store(true, Ordering::Relaxed);
        let status = unsafe { abi::rive_rs_font_asset_set_font(asset, std::ptr::null_mut()) };
        FONT_SET_STATUS.store(status as i32, Ordering::Relaxed);
        return true;
    }

    false
}

fn asset_bytes(name: &str) -> Vec<u8> {
    let path = PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .join("submodules/rive-runtime/tests/unit_tests/assets")
        .join(name);
    fs::read(&path).unwrap_or_else(|err| panic!("failed to read {}: {err}", path.display()))
}

fn reset_asset_mutator_state() {
    ASSET_MUTATOR_CALLS.store(0, Ordering::Relaxed);
    SAW_IMAGE_ASSET.store(false, Ordering::Relaxed);
    SAW_AUDIO_ASSET.store(false, Ordering::Relaxed);
    SAW_FONT_ASSET.store(false, Ordering::Relaxed);
    IMAGE_DECODE_STATUS.store(
        abi::rive_rs_status::RIVE_RS_STATUS_RUNTIME_ERROR as i32,
        Ordering::Relaxed,
    );
    AUDIO_SET_STATUS.store(
        abi::rive_rs_status::RIVE_RS_STATUS_RUNTIME_ERROR as i32,
        Ordering::Relaxed,
    );
    FONT_SET_STATUS.store(
        abi::rive_rs_status::RIVE_RS_STATUS_RUNTIME_ERROR as i32,
        Ordering::Relaxed,
    );
}

#[test]
fn math_helpers_smoke() -> Result<(), Box<dyn std::error::Error>> {
    let source = Aabb {
        min_x: 0.0,
        min_y: 0.0,
        max_x: 100.0,
        max_y: 100.0,
    };
    let destination = Aabb {
        min_x: 0.0,
        min_y: 0.0,
        max_x: 300.0,
        max_y: 300.0,
    };

    let matrix = compute_alignment(
        Fit::RIVE_RS_FIT_CONTAIN,
        Alignment::RIVE_RS_ALIGNMENT_CENTER,
        &source,
        &destination,
        1.0,
    )?;

    let mapped = map_xy(&matrix, Vec2 { x: 50.0, y: 50.0 })?;
    assert!(mapped.x.is_finite());
    assert!(mapped.y.is_finite());
    Ok(())
}

#[test]
fn load_with_asset_loader_smoke() -> Result<(), Box<dyn std::error::Error>> {
    LOADER_CALLBACK_CALLS.store(0, Ordering::Relaxed);

    let factory = Factory::new()?;
    let bytes = asset_bytes("in_band_asset.riv");
    let callbacks = FileAssetLoaderCallbacks::new(Some(test_asset_loader), std::ptr::null_mut());

    // SAFETY: callback function pointer and userdata are valid for this call.
    let file = unsafe { factory.load_file_with_asset_loader(&bytes, &callbacks)? };

    assert!(file.artboard_count() > 0);
    assert!(LOADER_CALLBACK_CALLS.load(Ordering::Relaxed) > 0);
    Ok(())
}

#[test]
fn callback_asset_contract_smoke() -> Result<(), Box<dyn std::error::Error>> {
    reset_asset_mutator_state();

    let factory = Factory::new()?;
    let callbacks =
        FileAssetLoaderCallbacks::new(Some(asset_mutating_loader), std::ptr::null_mut());

    // Exercise `file_asset_decode` on an in-band image asset.
    let image_file = unsafe {
        factory.load_file_with_asset_loader(&asset_bytes("in_band_asset.riv"), &callbacks)?
    };
    assert!(image_file.artboard_count() > 0);
    assert!(ASSET_MUTATOR_CALLS.load(Ordering::Relaxed) > 0);
    assert!(SAW_IMAGE_ASSET.load(Ordering::Relaxed));
    assert_eq!(IMAGE_DECODE_STATUS.load(Ordering::Relaxed), STATUS_OK);

    // Exercise `set_audio_source` on an audio asset.
    reset_asset_mutator_state();
    let audio_file =
        unsafe { factory.load_file_with_asset_loader(&asset_bytes("sound.riv"), &callbacks)? };
    assert!(audio_file.artboard_count() > 0);
    assert!(SAW_AUDIO_ASSET.load(Ordering::Relaxed));
    assert_eq!(AUDIO_SET_STATUS.load(Ordering::Relaxed), STATUS_OK);

    // Exercise `set_font` on a hosted font asset.
    reset_asset_mutator_state();
    let font_file = unsafe {
        factory.load_file_with_asset_loader(&asset_bytes("hosted_font_file.riv"), &callbacks)?
    };
    assert!(font_file.artboard_count() > 0);
    assert!(SAW_FONT_ASSET.load(Ordering::Relaxed));
    assert_eq!(FONT_SET_STATUS.load(Ordering::Relaxed), STATUS_OK);

    Ok(())
}

#[test]
fn core_runtime_smoke_with_state_machine_and_animation() -> Result<(), Box<dyn std::error::Error>> {
    let factory = Factory::new()?;
    let file = factory.load_file(&asset_bytes("smi_test.riv"))?;

    assert!(file.artboard_count() > 0);

    let mut artboard = file.default_artboard()?;
    assert!(!artboard.name().is_empty());

    let width = artboard.width();
    let height = artboard.height();
    assert!(width.is_finite());
    assert!(height.is_finite());

    artboard.set_width(width + 1.0);
    artboard.set_height(height + 1.0);
    artboard.reset_size()?;

    let frame_origin = artboard.frame_origin();
    artboard.set_frame_origin(frame_origin);
    artboard.set_volume(artboard.volume());

    let _ = artboard.bounds();
    let _ = artboard.advance(0.0)?;

    for index in 0..artboard.event_count() {
        let _ = artboard.event_at(index)?;
    }

    for index in 0..artboard.text_value_run_count() {
        let text = artboard.text_value_run_text_at(index)?;
        let _ = artboard.text_value_run_name_at(index)?;
        artboard.set_text_value_run_text_at(index, &text)?;
    }

    if artboard.animation_count() > 0 {
        let animation = artboard.animation_by_index(0)?;
        assert!(!animation.name().is_empty());
        let _ = animation.duration();
        let _ = animation.fps();
        let _ = animation.work_start();
        let _ = animation.work_end();
        let _ = animation.enable_work_area();
        let _ = animation.loop_value();
        let _ = animation.speed();
        animation.apply(&mut artboard, 0.0, 1.0)?;

        let mut animation_instance = LinearAnimationInstance::new(animation, &mut artboard)?;
        let _ = animation_instance.advance(1.0 / 60.0)?;
        animation_instance.apply(&mut artboard, 1.0)?;
        let t = animation_instance.time();
        animation_instance.set_time(t);
        let _ = animation_instance.did_loop();
    }

    assert!(artboard.state_machine_count() > 0);
    let state_machine = artboard.state_machine_by_index(0)?;
    assert!(!state_machine.name().is_empty());

    let mut state_machine_instance = StateMachineInstance::new(state_machine, &mut artboard)?;

    for index in 0..state_machine_instance.input_count() {
        let input = state_machine_instance.input(index)?;
        match input.input_type() {
            SmiInputType::RIVE_RS_SMI_INPUT_BOOL => {
                let mut input_bool = input.as_bool()?;
                let value = input_bool.get();
                input_bool.set(value);
            }
            SmiInputType::RIVE_RS_SMI_INPUT_NUMBER => {
                let mut input_number = input.as_number()?;
                let value = input_number.get();
                input_number.set(value);
            }
            SmiInputType::RIVE_RS_SMI_INPUT_TRIGGER => {
                let mut input_trigger = input.as_trigger()?;
                input_trigger.fire();
            }
        }
    }

    state_machine_instance.pointer_down(Vec2 { x: 0.0, y: 0.0 }, 0)?;
    state_machine_instance.pointer_move(Vec2 { x: 1.0, y: 1.0 }, 0)?;
    state_machine_instance.pointer_up(Vec2 { x: 2.0, y: 2.0 }, 0)?;
    state_machine_instance.pointer_exit(Vec2 { x: 3.0, y: 3.0 }, 0)?;

    let _ = state_machine_instance.has_listeners();
    let _ = state_machine_instance.has_any_listener();

    let _ = state_machine_instance.advance(1.0 / 60.0)?;
    let _ = state_machine_instance.advance_and_apply(1.0 / 60.0)?;

    for index in 0..state_machine_instance.reported_event_count() {
        let _ = state_machine_instance.reported_event_at(index)?;
    }

    for index in 0..state_machine_instance.state_changed_count() {
        let _ = state_machine_instance.state_changed_name_at(index)?;
    }

    Ok(())
}

#[test]
fn view_model_runtime_smoke() -> Result<(), Box<dyn std::error::Error>> {
    let factory = Factory::new()?;
    let file = factory.load_file(&asset_bytes("viewmodel_runtime_file.riv"))?;

    assert!(file.view_model_count() > 0);

    let view_model = file.view_model_by_index(0)?;
    assert!(!view_model.name().is_empty());
    assert!(view_model.property_count() > 0);

    if view_model.instance_count() > 0 {
        let _ = view_model.instance_name_at(0)?;
        let _ = view_model.instance_by_index(0)?;
    }

    let mut instance = view_model.default_instance()?;
    assert!(instance.property_count() > 0);

    for index in 0..instance.property_count() {
        let property = instance.property_at(index)?;
        if property.name.is_empty() {
            continue;
        }

        let path = property.name;
        match property.data_type {
            DataType::RIVE_RS_DATA_TYPE_NUMBER => {
                let value = instance.number(&path)?;
                instance.set_number(&path, value)?;
            }
            DataType::RIVE_RS_DATA_TYPE_STRING => {
                let value = instance.string(&path)?;
                instance.set_string(&path, &value)?;
            }
            DataType::RIVE_RS_DATA_TYPE_BOOLEAN => {
                let value = instance.boolean(&path)?;
                instance.set_boolean(&path, value)?;
            }
            DataType::RIVE_RS_DATA_TYPE_COLOR => {
                let value = instance.color(&path)?;
                instance.set_color(&path, value)?;
            }
            DataType::RIVE_RS_DATA_TYPE_ENUM => {
                let index = instance.enum_index(&path)?;
                instance.set_enum_index(&path, index)?;
                let _ = instance.enum_value(&path)?;
            }
            DataType::RIVE_RS_DATA_TYPE_TRIGGER => {
                instance.fire_trigger(&path)?;
            }
            DataType::RIVE_RS_DATA_TYPE_LIST => {
                let size = instance.list_size(&path)?;
                if size > 0 {
                    let _ = instance.list_instance_at(&path, 0)?;
                }
            }
            DataType::RIVE_RS_DATA_TYPE_VIEW_MODEL => {
                let _ = instance.view_model(&path);
            }
            DataType::RIVE_RS_DATA_TYPE_ARTBOARD => {
                let bindable = file.bindable_artboard_default()?;
                instance.set_artboard(&path, &bindable)?;
            }
            _ => {}
        }

        let _ = instance.property_has_changed(&path)?;
        instance.clear_property_changes(&path)?;
    }

    Ok(())
}

#[test]
fn invalid_media_decode_returns_error() -> Result<(), Box<dyn std::error::Error>> {
    let factory = Factory::new()?;

    assert!(factory.decode_audio(b"not-a-valid-audio-payload").is_err());
    assert!(factory.decode_font(b"not-a-valid-font-payload").is_err());

    Ok(())
}
