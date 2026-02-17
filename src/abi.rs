#![allow(non_camel_case_types)]
#![allow(non_upper_case_globals)]
#![allow(clippy::missing_safety_doc)]

use core::ffi::{c_char, c_void};

#[repr(C)]
pub struct rive_rs_factory {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_file {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_artboard {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_bindable_artboard {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_view_model {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_view_model_instance {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_transform_component {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_node {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_bone {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_root_bone {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_text_value_run {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_flattened_path {
    _private: [u8; 0],
}

#[repr(C)]
pub struct rive_rs_renderer {
    _private: [u8; 0],
}

#[repr(C)]
pub struct rive_rs_linear_animation {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_linear_animation_instance {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_state_machine {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_state_machine_instance {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_smi_input {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_smi_bool {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_smi_number {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_smi_trigger {
    _private: [u8; 0],
}

#[repr(C)]
pub struct rive_rs_file_asset {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_audio_source {
    _private: [u8; 0],
}
#[repr(C)]
pub struct rive_rs_font {
    _private: [u8; 0],
}

#[repr(i32)]
#[derive(Debug, Copy, Clone, Eq, PartialEq)]
pub enum rive_rs_status {
    RIVE_RS_STATUS_OK = 0,
    RIVE_RS_STATUS_NULL = 1,
    RIVE_RS_STATUS_INVALID_ARGUMENT = 2,
    RIVE_RS_STATUS_NOT_FOUND = 3,
    RIVE_RS_STATUS_OUT_OF_RANGE = 4,
    RIVE_RS_STATUS_UNSUPPORTED = 5,
    RIVE_RS_STATUS_DECODE_ERROR = 6,
    RIVE_RS_STATUS_RUNTIME_ERROR = 7,
}

#[repr(i32)]
#[derive(Debug, Copy, Clone, Eq, PartialEq)]
pub enum rive_rs_fit {
    RIVE_RS_FIT_FILL = 0,
    RIVE_RS_FIT_CONTAIN = 1,
    RIVE_RS_FIT_COVER = 2,
    RIVE_RS_FIT_FIT_WIDTH = 3,
    RIVE_RS_FIT_FIT_HEIGHT = 4,
    RIVE_RS_FIT_NONE = 5,
    RIVE_RS_FIT_SCALE_DOWN = 6,
    RIVE_RS_FIT_LAYOUT = 7,
}

#[repr(i32)]
#[derive(Debug, Copy, Clone, Eq, PartialEq)]
pub enum rive_rs_alignment {
    RIVE_RS_ALIGNMENT_TOP_LEFT = 0,
    RIVE_RS_ALIGNMENT_TOP_CENTER = 1,
    RIVE_RS_ALIGNMENT_TOP_RIGHT = 2,
    RIVE_RS_ALIGNMENT_CENTER_LEFT = 3,
    RIVE_RS_ALIGNMENT_CENTER = 4,
    RIVE_RS_ALIGNMENT_CENTER_RIGHT = 5,
    RIVE_RS_ALIGNMENT_BOTTOM_LEFT = 6,
    RIVE_RS_ALIGNMENT_BOTTOM_CENTER = 7,
    RIVE_RS_ALIGNMENT_BOTTOM_RIGHT = 8,
}

#[repr(i32)]
#[derive(Debug, Copy, Clone, Eq, PartialEq)]
pub enum rive_rs_smi_input_type {
    RIVE_RS_SMI_INPUT_BOOL = 59,
    RIVE_RS_SMI_INPUT_NUMBER = 56,
    RIVE_RS_SMI_INPUT_TRIGGER = 58,
}

#[repr(i32)]
#[derive(Debug, Copy, Clone, Eq, PartialEq)]
pub enum rive_rs_data_type {
    RIVE_RS_DATA_TYPE_NONE = 0,
    RIVE_RS_DATA_TYPE_STRING = 1,
    RIVE_RS_DATA_TYPE_NUMBER = 2,
    RIVE_RS_DATA_TYPE_BOOLEAN = 3,
    RIVE_RS_DATA_TYPE_COLOR = 4,
    RIVE_RS_DATA_TYPE_LIST = 5,
    RIVE_RS_DATA_TYPE_ENUM = 6,
    RIVE_RS_DATA_TYPE_TRIGGER = 7,
    RIVE_RS_DATA_TYPE_VIEW_MODEL = 8,
    RIVE_RS_DATA_TYPE_INTEGER = 9,
    RIVE_RS_DATA_TYPE_LIST_INDEX = 10,
    RIVE_RS_DATA_TYPE_IMAGE = 11,
    RIVE_RS_DATA_TYPE_ARTBOARD = 12,
}

#[repr(i32)]
#[derive(Debug, Copy, Clone, Eq, PartialEq)]
pub enum rive_rs_event_property_type {
    RIVE_RS_EVENT_PROPERTY_BOOL = 1,
    RIVE_RS_EVENT_PROPERTY_NUMBER = 2,
    RIVE_RS_EVENT_PROPERTY_STRING = 3,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct rive_rs_vec2 {
    pub x: f32,
    pub y: f32,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct rive_rs_mat2d {
    pub xx: f32,
    pub xy: f32,
    pub yx: f32,
    pub yy: f32,
    pub tx: f32,
    pub ty: f32,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct rive_rs_aabb {
    pub min_x: f32,
    pub min_y: f32,
    pub max_x: f32,
    pub max_y: f32,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct rive_rs_bytes_view {
    pub ptr: *const u8,
    pub len: usize,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct rive_rs_str_view {
    pub ptr: *const c_char,
    pub len: usize,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct rive_rs_property_info {
    pub name: rive_rs_str_view,
    pub data_type: rive_rs_data_type,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct rive_rs_event_info {
    pub name: rive_rs_str_view,
    pub r#type: u32,
    pub has_url: bool,
    pub url: rive_rs_str_view,
    pub has_target: bool,
    pub target: rive_rs_str_view,
    pub property_count: usize,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct rive_rs_event_property_info {
    pub name: rive_rs_str_view,
    pub value_type: rive_rs_event_property_type,
    pub bool_value: bool,
    pub number_value: f32,
    pub string_value: rive_rs_str_view,
}

pub type rive_rs_file_asset_loader_load_contents_fn = Option<
    unsafe extern "C" fn(
        user_data: *mut c_void,
        asset: *mut rive_rs_file_asset,
        in_band_bytes: rive_rs_bytes_view,
        factory: *mut rive_rs_factory,
    ) -> bool,
>;

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct rive_rs_file_asset_loader_callbacks {
    pub load_contents: rive_rs_file_asset_loader_load_contents_fn,
    pub user_data: *mut c_void,
}

unsafe extern "C" {
    pub fn rive_rs_abi_version() -> u32;

    pub fn rive_rs_factory_default() -> *mut rive_rs_factory;
    pub fn rive_rs_factory_ref(factory: *mut rive_rs_factory);
    pub fn rive_rs_factory_unref(factory: *mut rive_rs_factory);

    pub fn rive_rs_load_file(
        factory: *mut rive_rs_factory,
        bytes: rive_rs_bytes_view,
        out_file: *mut *mut rive_rs_file,
    ) -> rive_rs_status;
    pub fn rive_rs_load_file_with_asset_loader(
        factory: *mut rive_rs_factory,
        bytes: rive_rs_bytes_view,
        callbacks: *const rive_rs_file_asset_loader_callbacks,
        out_file: *mut *mut rive_rs_file,
    ) -> rive_rs_status;
    pub fn rive_rs_file_ref(file: *mut rive_rs_file);
    pub fn rive_rs_file_unref(file: *mut rive_rs_file);

    pub fn rive_rs_file_artboard_count(file: *const rive_rs_file) -> usize;
    pub fn rive_rs_file_artboard_default(
        file: *const rive_rs_file,
        out_artboard: *mut *mut rive_rs_artboard,
    ) -> rive_rs_status;
    pub fn rive_rs_file_artboard_by_index(
        file: *const rive_rs_file,
        index: usize,
        out_artboard: *mut *mut rive_rs_artboard,
    ) -> rive_rs_status;
    pub fn rive_rs_file_artboard_by_name(
        file: *const rive_rs_file,
        name: rive_rs_str_view,
        out_artboard: *mut *mut rive_rs_artboard,
    ) -> rive_rs_status;
    pub fn rive_rs_file_view_model_count(file: *const rive_rs_file) -> usize;
    pub fn rive_rs_file_view_model_by_index(
        file: *const rive_rs_file,
        index: usize,
        out_view_model: *mut *mut rive_rs_view_model,
    ) -> rive_rs_status;
    pub fn rive_rs_file_view_model_by_name(
        file: *const rive_rs_file,
        name: rive_rs_str_view,
        out_view_model: *mut *mut rive_rs_view_model,
    ) -> rive_rs_status;
    pub fn rive_rs_file_default_artboard_view_model(
        file: *const rive_rs_file,
        artboard: *mut rive_rs_artboard,
        out_view_model: *mut *mut rive_rs_view_model,
    ) -> rive_rs_status;
    pub fn rive_rs_file_bindable_artboard_by_name(
        file: *const rive_rs_file,
        name: rive_rs_str_view,
        out_bindable_artboard: *mut *mut rive_rs_bindable_artboard,
    ) -> rive_rs_status;
    pub fn rive_rs_file_bindable_artboard_default(
        file: *const rive_rs_file,
        out_bindable_artboard: *mut *mut rive_rs_bindable_artboard,
    ) -> rive_rs_status;
    pub fn rive_rs_file_bindable_artboard_from_artboard(
        file: *const rive_rs_file,
        artboard: *mut rive_rs_artboard,
        out_bindable_artboard: *mut *mut rive_rs_bindable_artboard,
    ) -> rive_rs_status;
    pub fn rive_rs_file_has_audio(file: *const rive_rs_file) -> bool;
    pub fn rive_rs_file_enum_count(file: *const rive_rs_file) -> usize;
    pub fn rive_rs_file_enum_name_at(
        file: *const rive_rs_file,
        enum_index: usize,
        out_name: *mut rive_rs_str_view,
    ) -> rive_rs_status;
    pub fn rive_rs_file_enum_value_count(file: *const rive_rs_file, enum_index: usize) -> usize;
    pub fn rive_rs_file_enum_value_name_at(
        file: *const rive_rs_file,
        enum_index: usize,
        value_index: usize,
        out_name: *mut rive_rs_str_view,
    ) -> rive_rs_status;

    pub fn rive_rs_artboard_ref(artboard: *mut rive_rs_artboard);
    pub fn rive_rs_artboard_unref(artboard: *mut rive_rs_artboard);
    pub fn rive_rs_artboard_advance(
        artboard: *mut rive_rs_artboard,
        seconds: f32,
        out_changed: *mut bool,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_draw(
        artboard: *mut rive_rs_artboard,
        renderer: *mut rive_rs_renderer,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_did_change(artboard: *const rive_rs_artboard) -> bool;
    pub fn rive_rs_artboard_name(artboard: *const rive_rs_artboard) -> rive_rs_str_view;
    pub fn rive_rs_artboard_bounds(artboard: *const rive_rs_artboard) -> rive_rs_aabb;
    pub fn rive_rs_artboard_width(artboard: *const rive_rs_artboard) -> f32;
    pub fn rive_rs_artboard_height(artboard: *const rive_rs_artboard) -> f32;
    pub fn rive_rs_artboard_set_width(artboard: *mut rive_rs_artboard, width: f32);
    pub fn rive_rs_artboard_set_height(artboard: *mut rive_rs_artboard, height: f32);
    pub fn rive_rs_artboard_frame_origin(artboard: *const rive_rs_artboard) -> bool;
    pub fn rive_rs_artboard_set_frame_origin(artboard: *mut rive_rs_artboard, frame_origin: bool);
    pub fn rive_rs_artboard_has_audio(artboard: *const rive_rs_artboard) -> bool;
    pub fn rive_rs_artboard_volume(artboard: *const rive_rs_artboard) -> f32;
    pub fn rive_rs_artboard_set_volume(artboard: *mut rive_rs_artboard, volume: f32);
    pub fn rive_rs_artboard_reset_size(artboard: *mut rive_rs_artboard) -> rive_rs_status;
    pub fn rive_rs_artboard_animation_count(artboard: *const rive_rs_artboard) -> usize;
    pub fn rive_rs_artboard_state_machine_count(artboard: *const rive_rs_artboard) -> usize;
    pub fn rive_rs_artboard_event_count(artboard: *const rive_rs_artboard) -> usize;
    pub fn rive_rs_artboard_event_at(
        artboard: *const rive_rs_artboard,
        index: usize,
        out_event: *mut rive_rs_event_info,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_event_property_at(
        artboard: *const rive_rs_artboard,
        event_index: usize,
        property_index: usize,
        out_property: *mut rive_rs_event_property_info,
    ) -> rive_rs_status;

    pub fn rive_rs_artboard_animation_by_index(
        artboard: *mut rive_rs_artboard,
        index: usize,
        out_animation: *mut *mut rive_rs_linear_animation,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_animation_by_name(
        artboard: *mut rive_rs_artboard,
        name: rive_rs_str_view,
        out_animation: *mut *mut rive_rs_linear_animation,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_state_machine_by_index(
        artboard: *mut rive_rs_artboard,
        index: usize,
        out_state_machine: *mut *mut rive_rs_state_machine,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_state_machine_by_name(
        artboard: *mut rive_rs_artboard,
        name: rive_rs_str_view,
        out_state_machine: *mut *mut rive_rs_state_machine,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_input_by_path(
        artboard: *mut rive_rs_artboard,
        name: rive_rs_str_view,
        path: rive_rs_str_view,
        out_input: *mut *mut rive_rs_smi_input,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_text_value_run_count(artboard: *const rive_rs_artboard) -> usize;
    pub fn rive_rs_artboard_text_value_run_name_at(
        artboard: *const rive_rs_artboard,
        index: usize,
        out_name: *mut rive_rs_str_view,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_text_value_run_text_at(
        artboard: *const rive_rs_artboard,
        index: usize,
        out_text: *mut rive_rs_str_view,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_set_text_value_run_text_at(
        artboard: *mut rive_rs_artboard,
        index: usize,
        text: rive_rs_str_view,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_text_by_path_get(
        artboard: *const rive_rs_artboard,
        name: rive_rs_str_view,
        path: rive_rs_str_view,
        out_text: *mut rive_rs_str_view,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_text_by_path_set(
        artboard: *mut rive_rs_artboard,
        name: rive_rs_str_view,
        path: rive_rs_str_view,
        text: rive_rs_str_view,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_transform_component_by_name(
        artboard: *mut rive_rs_artboard,
        name: rive_rs_str_view,
        out_component: *mut *mut rive_rs_transform_component,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_node_by_name(
        artboard: *mut rive_rs_artboard,
        name: rive_rs_str_view,
        out_node: *mut *mut rive_rs_node,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_bone_by_name(
        artboard: *mut rive_rs_artboard,
        name: rive_rs_str_view,
        out_bone: *mut *mut rive_rs_bone,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_root_bone_by_name(
        artboard: *mut rive_rs_artboard,
        name: rive_rs_str_view,
        out_root_bone: *mut *mut rive_rs_root_bone,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_text_value_run_by_name(
        artboard: *mut rive_rs_artboard,
        name: rive_rs_str_view,
        out_text_value_run: *mut *mut rive_rs_text_value_run,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_text_value_run_by_index(
        artboard: *mut rive_rs_artboard,
        index: usize,
        out_text_value_run: *mut *mut rive_rs_text_value_run,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_flatten_path(
        artboard: *mut rive_rs_artboard,
        index: usize,
        transform_to_parent: bool,
        out_path: *mut *mut rive_rs_flattened_path,
    ) -> rive_rs_status;
    pub fn rive_rs_artboard_bind_view_model_instance(
        artboard: *mut rive_rs_artboard,
        instance: *mut rive_rs_view_model_instance,
    ) -> rive_rs_status;
    pub fn rive_rs_bindable_artboard_ref(bindable_artboard: *mut rive_rs_bindable_artboard);
    pub fn rive_rs_bindable_artboard_unref(bindable_artboard: *mut rive_rs_bindable_artboard);

    pub fn rive_rs_transform_component_scale_x(
        component: *const rive_rs_transform_component,
    ) -> f32;
    pub fn rive_rs_transform_component_set_scale_x(
        component: *mut rive_rs_transform_component,
        value: f32,
    );
    pub fn rive_rs_transform_component_scale_y(
        component: *const rive_rs_transform_component,
    ) -> f32;
    pub fn rive_rs_transform_component_set_scale_y(
        component: *mut rive_rs_transform_component,
        value: f32,
    );
    pub fn rive_rs_transform_component_rotation(
        component: *const rive_rs_transform_component,
    ) -> f32;
    pub fn rive_rs_transform_component_set_rotation(
        component: *mut rive_rs_transform_component,
        value: f32,
    );
    pub fn rive_rs_transform_component_world_transform(
        component: *const rive_rs_transform_component,
        out_matrix: *mut rive_rs_mat2d,
    ) -> rive_rs_status;
    pub fn rive_rs_transform_component_parent_world_transform(
        component: *const rive_rs_transform_component,
        out_matrix: *mut rive_rs_mat2d,
    ) -> rive_rs_status;

    pub fn rive_rs_node_x(node: *const rive_rs_node) -> f32;
    pub fn rive_rs_node_set_x(node: *mut rive_rs_node, value: f32);
    pub fn rive_rs_node_y(node: *const rive_rs_node) -> f32;
    pub fn rive_rs_node_set_y(node: *mut rive_rs_node, value: f32);

    pub fn rive_rs_bone_length(bone: *const rive_rs_bone) -> f32;
    pub fn rive_rs_bone_set_length(bone: *mut rive_rs_bone, value: f32);

    pub fn rive_rs_root_bone_x(root_bone: *const rive_rs_root_bone) -> f32;
    pub fn rive_rs_root_bone_set_x(root_bone: *mut rive_rs_root_bone, value: f32);
    pub fn rive_rs_root_bone_y(root_bone: *const rive_rs_root_bone) -> f32;
    pub fn rive_rs_root_bone_set_y(root_bone: *mut rive_rs_root_bone, value: f32);

    pub fn rive_rs_text_value_run_name(
        text_value_run: *const rive_rs_text_value_run,
    ) -> rive_rs_str_view;
    pub fn rive_rs_text_value_run_text(
        text_value_run: *const rive_rs_text_value_run,
    ) -> rive_rs_str_view;
    pub fn rive_rs_text_value_run_set_text(
        text_value_run: *mut rive_rs_text_value_run,
        text: rive_rs_str_view,
    ) -> rive_rs_status;

    pub fn rive_rs_flattened_path_delete(path: *mut rive_rs_flattened_path);
    pub fn rive_rs_flattened_path_length(path: *const rive_rs_flattened_path) -> usize;
    pub fn rive_rs_flattened_path_is_cubic(
        path: *const rive_rs_flattened_path,
        index: usize,
        out_is_cubic: *mut bool,
    ) -> rive_rs_status;
    pub fn rive_rs_flattened_path_x(
        path: *const rive_rs_flattened_path,
        index: usize,
        out_value: *mut f32,
    ) -> rive_rs_status;
    pub fn rive_rs_flattened_path_y(
        path: *const rive_rs_flattened_path,
        index: usize,
        out_value: *mut f32,
    ) -> rive_rs_status;
    pub fn rive_rs_flattened_path_in_x(
        path: *const rive_rs_flattened_path,
        index: usize,
        out_value: *mut f32,
    ) -> rive_rs_status;
    pub fn rive_rs_flattened_path_in_y(
        path: *const rive_rs_flattened_path,
        index: usize,
        out_value: *mut f32,
    ) -> rive_rs_status;
    pub fn rive_rs_flattened_path_out_x(
        path: *const rive_rs_flattened_path,
        index: usize,
        out_value: *mut f32,
    ) -> rive_rs_status;
    pub fn rive_rs_flattened_path_out_y(
        path: *const rive_rs_flattened_path,
        index: usize,
        out_value: *mut f32,
    ) -> rive_rs_status;

    pub fn rive_rs_linear_animation_instance_new(
        animation: *mut rive_rs_linear_animation,
        artboard: *mut rive_rs_artboard,
        out_instance: *mut *mut rive_rs_linear_animation_instance,
    ) -> rive_rs_status;
    pub fn rive_rs_linear_animation_name(
        animation: *const rive_rs_linear_animation,
    ) -> rive_rs_str_view;
    pub fn rive_rs_linear_animation_duration(animation: *const rive_rs_linear_animation) -> u32;
    pub fn rive_rs_linear_animation_fps(animation: *const rive_rs_linear_animation) -> u32;
    pub fn rive_rs_linear_animation_work_start(animation: *const rive_rs_linear_animation) -> u32;
    pub fn rive_rs_linear_animation_work_end(animation: *const rive_rs_linear_animation) -> u32;
    pub fn rive_rs_linear_animation_enable_work_area(
        animation: *const rive_rs_linear_animation,
    ) -> bool;
    pub fn rive_rs_linear_animation_loop_value(animation: *const rive_rs_linear_animation) -> u32;
    pub fn rive_rs_linear_animation_speed(animation: *const rive_rs_linear_animation) -> f32;
    pub fn rive_rs_linear_animation_apply(
        animation: *const rive_rs_linear_animation,
        artboard: *mut rive_rs_artboard,
        time: f32,
        mix: f32,
    ) -> rive_rs_status;
    pub fn rive_rs_linear_animation_instance_delete(
        instance: *mut rive_rs_linear_animation_instance,
    );
    pub fn rive_rs_linear_animation_instance_advance(
        instance: *mut rive_rs_linear_animation_instance,
        seconds: f32,
        out_looped: *mut bool,
    ) -> rive_rs_status;
    pub fn rive_rs_linear_animation_instance_apply(
        instance: *mut rive_rs_linear_animation_instance,
        artboard: *mut rive_rs_artboard,
        mix: f32,
    ) -> rive_rs_status;
    pub fn rive_rs_linear_animation_instance_time(
        instance: *const rive_rs_linear_animation_instance,
    ) -> f32;
    pub fn rive_rs_linear_animation_instance_set_time(
        instance: *mut rive_rs_linear_animation_instance,
        seconds: f32,
    );
    pub fn rive_rs_linear_animation_instance_did_loop(
        instance: *const rive_rs_linear_animation_instance,
    ) -> bool;

    pub fn rive_rs_state_machine_instance_new(
        state_machine: *mut rive_rs_state_machine,
        artboard: *mut rive_rs_artboard,
        out_instance: *mut *mut rive_rs_state_machine_instance,
    ) -> rive_rs_status;
    pub fn rive_rs_state_machine_name(
        state_machine: *const rive_rs_state_machine,
    ) -> rive_rs_str_view;
    pub fn rive_rs_state_machine_instance_delete(instance: *mut rive_rs_state_machine_instance);
    pub fn rive_rs_state_machine_instance_advance(
        instance: *mut rive_rs_state_machine_instance,
        seconds: f32,
        out_changed: *mut bool,
    ) -> rive_rs_status;
    pub fn rive_rs_state_machine_instance_advance_and_apply(
        instance: *mut rive_rs_state_machine_instance,
        seconds: f32,
        out_changed: *mut bool,
    ) -> rive_rs_status;
    pub fn rive_rs_state_machine_input_count(
        instance: *const rive_rs_state_machine_instance,
    ) -> usize;
    pub fn rive_rs_state_machine_input_at(
        instance: *mut rive_rs_state_machine_instance,
        index: usize,
        out_input: *mut *mut rive_rs_smi_input,
    ) -> rive_rs_status;
    pub fn rive_rs_smi_input_type_of(input: *const rive_rs_smi_input) -> rive_rs_smi_input_type;
    pub fn rive_rs_smi_input_name(input: *const rive_rs_smi_input) -> rive_rs_str_view;
    pub fn rive_rs_smi_input_as_bool(
        input: *mut rive_rs_smi_input,
        out_bool: *mut *mut rive_rs_smi_bool,
    ) -> rive_rs_status;
    pub fn rive_rs_smi_input_as_number(
        input: *mut rive_rs_smi_input,
        out_number: *mut *mut rive_rs_smi_number,
    ) -> rive_rs_status;
    pub fn rive_rs_smi_input_as_trigger(
        input: *mut rive_rs_smi_input,
        out_trigger: *mut *mut rive_rs_smi_trigger,
    ) -> rive_rs_status;
    pub fn rive_rs_smi_bool_get(input: *const rive_rs_smi_bool) -> bool;
    pub fn rive_rs_smi_bool_set(input: *mut rive_rs_smi_bool, value: bool);
    pub fn rive_rs_smi_number_get(input: *const rive_rs_smi_number) -> f32;
    pub fn rive_rs_smi_number_set(input: *mut rive_rs_smi_number, value: f32);
    pub fn rive_rs_smi_trigger_fire(input: *mut rive_rs_smi_trigger);
    pub fn rive_rs_state_machine_instance_pointer_down(
        instance: *mut rive_rs_state_machine_instance,
        point: rive_rs_vec2,
        pointer_id: i32,
    ) -> rive_rs_status;
    pub fn rive_rs_state_machine_instance_pointer_move(
        instance: *mut rive_rs_state_machine_instance,
        point: rive_rs_vec2,
        pointer_id: i32,
    ) -> rive_rs_status;
    pub fn rive_rs_state_machine_instance_pointer_up(
        instance: *mut rive_rs_state_machine_instance,
        point: rive_rs_vec2,
        pointer_id: i32,
    ) -> rive_rs_status;
    pub fn rive_rs_state_machine_instance_pointer_exit(
        instance: *mut rive_rs_state_machine_instance,
        point: rive_rs_vec2,
        pointer_id: i32,
    ) -> rive_rs_status;
    pub fn rive_rs_state_machine_instance_has_listeners(
        instance: *const rive_rs_state_machine_instance,
    ) -> bool;
    pub fn rive_rs_state_machine_instance_has_any_listener(
        instance: *const rive_rs_state_machine_instance,
    ) -> bool;
    pub fn rive_rs_state_machine_reported_event_count(
        instance: *const rive_rs_state_machine_instance,
    ) -> usize;
    pub fn rive_rs_state_machine_reported_event_at(
        instance: *const rive_rs_state_machine_instance,
        index: usize,
        out_event: *mut rive_rs_event_info,
        out_delay_seconds: *mut f32,
    ) -> rive_rs_status;
    pub fn rive_rs_state_machine_reported_event_property_at(
        instance: *const rive_rs_state_machine_instance,
        reported_event_index: usize,
        property_index: usize,
        out_property: *mut rive_rs_event_property_info,
    ) -> rive_rs_status;
    pub fn rive_rs_state_machine_state_changed_count(
        instance: *const rive_rs_state_machine_instance,
    ) -> usize;
    pub fn rive_rs_state_machine_state_changed_name_at(
        instance: *const rive_rs_state_machine_instance,
        index: usize,
        out_name: *mut rive_rs_str_view,
    ) -> rive_rs_status;
    pub fn rive_rs_state_machine_instance_bind_view_model_instance(
        instance: *mut rive_rs_state_machine_instance,
        view_model_instance: *mut rive_rs_view_model_instance,
    ) -> rive_rs_status;

    pub fn rive_rs_view_model_ref(view_model: *mut rive_rs_view_model);
    pub fn rive_rs_view_model_unref(view_model: *mut rive_rs_view_model);
    pub fn rive_rs_view_model_name(view_model: *const rive_rs_view_model) -> rive_rs_str_view;
    pub fn rive_rs_view_model_property_count(view_model: *const rive_rs_view_model) -> usize;
    pub fn rive_rs_view_model_instance_count(view_model: *const rive_rs_view_model) -> usize;
    pub fn rive_rs_view_model_property_at(
        view_model: *const rive_rs_view_model,
        index: usize,
        out_property: *mut rive_rs_property_info,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_name_at(
        view_model: *const rive_rs_view_model,
        index: usize,
        out_name: *mut rive_rs_str_view,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_by_index(
        view_model: *const rive_rs_view_model,
        index: usize,
        out_instance: *mut *mut rive_rs_view_model_instance,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_by_name(
        view_model: *const rive_rs_view_model,
        name: rive_rs_str_view,
        out_instance: *mut *mut rive_rs_view_model_instance,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_default_instance(
        view_model: *const rive_rs_view_model,
        out_instance: *mut *mut rive_rs_view_model_instance,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_new_instance(
        view_model: *const rive_rs_view_model,
        out_instance: *mut *mut rive_rs_view_model_instance,
    ) -> rive_rs_status;

    pub fn rive_rs_view_model_instance_ref(instance: *mut rive_rs_view_model_instance);
    pub fn rive_rs_view_model_instance_unref(instance: *mut rive_rs_view_model_instance);
    pub fn rive_rs_view_model_instance_property_count(
        instance: *const rive_rs_view_model_instance,
    ) -> usize;
    pub fn rive_rs_view_model_instance_property_at(
        instance: *const rive_rs_view_model_instance,
        index: usize,
        out_property: *mut rive_rs_property_info,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_get_number(
        instance: *const rive_rs_view_model_instance,
        path: rive_rs_str_view,
        out_value: *mut f32,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_set_number(
        instance: *mut rive_rs_view_model_instance,
        path: rive_rs_str_view,
        value: f32,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_get_string(
        instance: *const rive_rs_view_model_instance,
        path: rive_rs_str_view,
        out_value: *mut rive_rs_str_view,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_set_string(
        instance: *mut rive_rs_view_model_instance,
        path: rive_rs_str_view,
        value: rive_rs_str_view,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_get_boolean(
        instance: *const rive_rs_view_model_instance,
        path: rive_rs_str_view,
        out_value: *mut bool,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_set_boolean(
        instance: *mut rive_rs_view_model_instance,
        path: rive_rs_str_view,
        value: bool,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_get_color(
        instance: *const rive_rs_view_model_instance,
        path: rive_rs_str_view,
        out_argb: *mut i32,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_set_color(
        instance: *mut rive_rs_view_model_instance,
        path: rive_rs_str_view,
        argb: i32,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_get_enum(
        instance: *const rive_rs_view_model_instance,
        path: rive_rs_str_view,
        out_value: *mut rive_rs_str_view,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_set_enum(
        instance: *mut rive_rs_view_model_instance,
        path: rive_rs_str_view,
        value: rive_rs_str_view,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_get_enum_index(
        instance: *const rive_rs_view_model_instance,
        path: rive_rs_str_view,
        out_index: *mut u32,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_set_enum_index(
        instance: *mut rive_rs_view_model_instance,
        path: rive_rs_str_view,
        index: u32,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_fire_trigger(
        instance: *mut rive_rs_view_model_instance,
        path: rive_rs_str_view,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_get_view_model(
        instance: *const rive_rs_view_model_instance,
        path: rive_rs_str_view,
        out_instance: *mut *mut rive_rs_view_model_instance,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_replace_view_model(
        instance: *mut rive_rs_view_model_instance,
        path: rive_rs_str_view,
        value: *mut rive_rs_view_model_instance,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_property_has_changed(
        instance: *const rive_rs_view_model_instance,
        path: rive_rs_str_view,
        out_changed: *mut bool,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_clear_property_changes(
        instance: *mut rive_rs_view_model_instance,
        path: rive_rs_str_view,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_list_size(
        instance: *const rive_rs_view_model_instance,
        path: rive_rs_str_view,
        out_size: *mut usize,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_list_instance_at(
        instance: *const rive_rs_view_model_instance,
        path: rive_rs_str_view,
        index: usize,
        out_value: *mut *mut rive_rs_view_model_instance,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_list_add_instance(
        instance: *mut rive_rs_view_model_instance,
        path: rive_rs_str_view,
        value: *mut rive_rs_view_model_instance,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_list_add_instance_at(
        instance: *mut rive_rs_view_model_instance,
        path: rive_rs_str_view,
        value: *mut rive_rs_view_model_instance,
        index: usize,
        out_added: *mut bool,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_list_remove_instance(
        instance: *mut rive_rs_view_model_instance,
        path: rive_rs_str_view,
        value: *mut rive_rs_view_model_instance,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_list_remove_instance_at(
        instance: *mut rive_rs_view_model_instance,
        path: rive_rs_str_view,
        index: usize,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_list_swap(
        instance: *mut rive_rs_view_model_instance,
        path: rive_rs_str_view,
        a: u32,
        b: u32,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_set_artboard(
        instance: *mut rive_rs_view_model_instance,
        path: rive_rs_str_view,
        value: *mut rive_rs_bindable_artboard,
    ) -> rive_rs_status;
    pub fn rive_rs_view_model_instance_set_artboard_view_model(
        instance: *mut rive_rs_view_model_instance,
        path: rive_rs_str_view,
        view_model_instance: *mut rive_rs_view_model_instance,
    ) -> rive_rs_status;

    pub fn rive_rs_compute_alignment(
        fit: rive_rs_fit,
        alignment: rive_rs_alignment,
        source: *const rive_rs_aabb,
        destination: *const rive_rs_aabb,
        scale_factor: f32,
        out_matrix: *mut rive_rs_mat2d,
    ) -> rive_rs_status;
    pub fn rive_rs_map_xy(
        matrix: *const rive_rs_mat2d,
        point: rive_rs_vec2,
        out_point: *mut rive_rs_vec2,
    ) -> rive_rs_status;

    pub fn rive_rs_decode_audio(
        factory: *mut rive_rs_factory,
        bytes: rive_rs_bytes_view,
        out_audio: *mut *mut rive_rs_audio_source,
    ) -> rive_rs_status;
    pub fn rive_rs_decode_font(
        factory: *mut rive_rs_factory,
        bytes: rive_rs_bytes_view,
        out_font: *mut *mut rive_rs_font,
    ) -> rive_rs_status;
    pub fn rive_rs_audio_source_unref(audio: *mut rive_rs_audio_source);
    pub fn rive_rs_font_unref(font: *mut rive_rs_font);

    pub fn rive_rs_ptr_to_file_asset(pointer: usize) -> *mut rive_rs_file_asset;
    pub fn rive_rs_ptr_to_audio_asset(pointer: usize) -> *mut rive_rs_file_asset;
    pub fn rive_rs_ptr_to_image_asset(pointer: usize) -> *mut rive_rs_file_asset;
    pub fn rive_rs_ptr_to_font_asset(pointer: usize) -> *mut rive_rs_file_asset;
    pub fn rive_rs_file_asset_name(asset: *const rive_rs_file_asset) -> rive_rs_str_view;
    pub fn rive_rs_file_asset_cdn_base_url(asset: *const rive_rs_file_asset) -> rive_rs_str_view;
    pub fn rive_rs_file_asset_file_extension(asset: *const rive_rs_file_asset) -> rive_rs_str_view;
    pub fn rive_rs_file_asset_unique_filename(asset: *const rive_rs_file_asset)
    -> rive_rs_str_view;
    pub fn rive_rs_file_asset_is_audio(asset: *const rive_rs_file_asset) -> bool;
    pub fn rive_rs_file_asset_is_image(asset: *const rive_rs_file_asset) -> bool;
    pub fn rive_rs_file_asset_is_font(asset: *const rive_rs_file_asset) -> bool;
    pub fn rive_rs_file_asset_cdn_uuid(asset: *const rive_rs_file_asset) -> rive_rs_str_view;

    pub fn rive_rs_file_asset_decode(
        factory: *mut rive_rs_factory,
        asset: *mut rive_rs_file_asset,
        bytes: rive_rs_bytes_view,
    ) -> rive_rs_status;
    pub fn rive_rs_audio_asset_set_audio_source(
        asset: *mut rive_rs_file_asset,
        audio: *mut rive_rs_audio_source,
    ) -> rive_rs_status;
    pub fn rive_rs_font_asset_set_font(
        asset: *mut rive_rs_file_asset,
        font: *mut rive_rs_font,
    ) -> rive_rs_status;
}
