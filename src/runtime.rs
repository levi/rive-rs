use core::ffi::{c_char, c_void};
use core::ptr;
use core::slice;
use std::ptr::NonNull;

use crate::abi;
use crate::{Error, Status, status_ok};

pub type Aabb = abi::rive_rs_aabb;
pub type Alignment = abi::rive_rs_alignment;
pub type DataType = abi::rive_rs_data_type;
pub type Fit = abi::rive_rs_fit;
pub type Mat2D = abi::rive_rs_mat2d;
pub type EventPropertyType = abi::rive_rs_event_property_type;
pub type SmiInputType = abi::rive_rs_smi_input_type;
pub type Vec2 = abi::rive_rs_vec2;

#[derive(Debug, Clone)]
pub struct PropertyInfo {
    pub name: String,
    pub data_type: DataType,
}

#[derive(Debug, Clone)]
pub struct DataEnumInfo {
    pub name: String,
    pub values: Vec<String>,
}

#[derive(Debug, Clone)]
pub enum EventPropertyValue {
    Bool(bool),
    Number(f32),
    String(String),
}

#[derive(Debug, Clone)]
pub struct EventProperty {
    pub name: String,
    pub value: EventPropertyValue,
}

#[derive(Debug, Clone)]
pub struct EventInfo {
    pub name: String,
    pub event_type: u32,
    pub url: Option<String>,
    pub target: Option<String>,
    pub properties: Vec<EventProperty>,
}

#[derive(Debug, Clone)]
pub struct ReportedEvent {
    pub event: EventInfo,
    pub delay_seconds: f32,
}

#[derive(Debug, Copy, Clone)]
pub struct FileAssetLoaderCallbacks {
    raw: abi::rive_rs_file_asset_loader_callbacks,
}

impl FileAssetLoaderCallbacks {
    pub const fn new(
        load_contents: abi::rive_rs_file_asset_loader_load_contents_fn,
        user_data: *mut c_void,
    ) -> Self {
        Self {
            raw: abi::rive_rs_file_asset_loader_callbacks {
                load_contents,
                user_data,
            },
        }
    }

    fn as_abi_ptr(&self) -> *const abi::rive_rs_file_asset_loader_callbacks {
        &self.raw
    }
}

fn bytes_view(bytes: &[u8]) -> abi::rive_rs_bytes_view {
    abi::rive_rs_bytes_view {
        ptr: bytes.as_ptr(),
        len: bytes.len(),
    }
}

fn str_view(value: &str) -> abi::rive_rs_str_view {
    abi::rive_rs_str_view {
        ptr: value.as_ptr().cast::<c_char>(),
        len: value.len(),
    }
}

fn string_from_view(view: abi::rive_rs_str_view) -> String {
    if view.ptr.is_null() || view.len == 0 {
        return String::new();
    }
    // SAFETY: ABI contract requires `ptr` to point to `len` bytes of valid UTF-8 or UTF-8 compatible text.
    let bytes = unsafe { slice::from_raw_parts(view.ptr.cast::<u8>(), view.len) };
    String::from_utf8_lossy(bytes).into_owned()
}

fn property_info_from_abi(info: abi::rive_rs_property_info) -> PropertyInfo {
    PropertyInfo {
        name: string_from_view(info.name),
        data_type: info.data_type,
    }
}

fn empty_str_view() -> abi::rive_rs_str_view {
    abi::rive_rs_str_view {
        ptr: ptr::null::<c_char>(),
        len: 0,
    }
}

fn empty_event_info() -> abi::rive_rs_event_info {
    abi::rive_rs_event_info {
        name: empty_str_view(),
        r#type: 0,
        has_url: false,
        url: empty_str_view(),
        has_target: false,
        target: empty_str_view(),
        property_count: 0,
    }
}

fn empty_event_property_info() -> abi::rive_rs_event_property_info {
    abi::rive_rs_event_property_info {
        name: empty_str_view(),
        value_type: EventPropertyType::RIVE_RS_EVENT_PROPERTY_STRING,
        bool_value: false,
        number_value: 0.0,
        string_value: empty_str_view(),
    }
}

fn event_property_from_abi(info: abi::rive_rs_event_property_info) -> EventProperty {
    let value = match info.value_type {
        EventPropertyType::RIVE_RS_EVENT_PROPERTY_BOOL => EventPropertyValue::Bool(info.bool_value),
        EventPropertyType::RIVE_RS_EVENT_PROPERTY_NUMBER => {
            EventPropertyValue::Number(info.number_value)
        }
        EventPropertyType::RIVE_RS_EVENT_PROPERTY_STRING => {
            EventPropertyValue::String(string_from_view(info.string_value))
        }
    };
    EventProperty {
        name: string_from_view(info.name),
        value,
    }
}

fn status_result(status: Status) -> Result<(), Error> {
    if status_ok(status) {
        Ok(())
    } else {
        Err(Error::from_status(status))
    }
}

fn non_null<T>(raw: *mut T) -> Result<NonNull<T>, Error> {
    NonNull::new(raw).ok_or_else(Error::null_handle)
}

#[inline]
pub fn abi_version() -> u32 {
    // SAFETY: no preconditions for querying static ABI version.
    unsafe { abi::rive_rs_abi_version() }
}

pub struct Factory {
    raw: NonNull<abi::rive_rs_factory>,
}

impl Factory {
    pub fn new() -> Result<Self, Error> {
        // SAFETY: FFI constructor with no arguments.
        let raw = unsafe { abi::rive_rs_factory_default() };
        Ok(Self {
            raw: non_null(raw)?,
        })
    }

    pub fn new_webgl2() -> Result<Self, Error> {
        // SAFETY: FFI constructor with no arguments.
        let raw = unsafe { abi::rive_rs_factory_webgl2() };
        let raw = NonNull::new(raw)
            .ok_or_else(|| Error::from_status(Status::RIVE_RS_STATUS_UNSUPPORTED))?;
        Ok(Self { raw })
    }

    pub fn new_webgpu() -> Result<Self, Error> {
        // SAFETY: FFI constructor with no arguments.
        let raw = unsafe { abi::rive_rs_factory_webgpu() };
        let raw = NonNull::new(raw)
            .ok_or_else(|| Error::from_status(Status::RIVE_RS_STATUS_UNSUPPORTED))?;
        Ok(Self { raw })
    }

    pub fn as_raw(&self) -> *mut abi::rive_rs_factory {
        self.raw.as_ptr()
    }

    pub fn load_file(&self, bytes: &[u8]) -> Result<File, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: pointers are valid for call duration; out pointer is writable.
        let status = unsafe { abi::rive_rs_load_file(self.as_raw(), bytes_view(bytes), &mut out) };
        status_result(status)?;
        Ok(File {
            raw: non_null(out)?,
        })
    }

    /// # Safety
    ///
    /// The callback userdata and function pointer must remain valid for the
    /// duration expected by the underlying runtime.
    pub unsafe fn load_file_with_asset_loader(
        &self,
        bytes: &[u8],
        callbacks: &FileAssetLoaderCallbacks,
    ) -> Result<File, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: caller guarantees callback lifetime requirements.
        let status = unsafe {
            abi::rive_rs_load_file_with_asset_loader(
                self.as_raw(),
                bytes_view(bytes),
                callbacks.as_abi_ptr(),
                &mut out,
            )
        };
        status_result(status)?;
        Ok(File {
            raw: non_null(out)?,
        })
    }

    pub fn decode_audio(&self, bytes: &[u8]) -> Result<AudioSource, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: pointers are valid for call duration; out pointer is writable.
        let status =
            unsafe { abi::rive_rs_decode_audio(self.as_raw(), bytes_view(bytes), &mut out) };
        status_result(status)?;
        Ok(AudioSource {
            raw: non_null(out)?,
        })
    }

    pub fn decode_font(&self, bytes: &[u8]) -> Result<Font, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: pointers are valid for call duration; out pointer is writable.
        let status =
            unsafe { abi::rive_rs_decode_font(self.as_raw(), bytes_view(bytes), &mut out) };
        status_result(status)?;
        Ok(Font {
            raw: non_null(out)?,
        })
    }
}

impl Clone for Factory {
    fn clone(&self) -> Self {
        // SAFETY: intrusive ref-count increment on valid handle.
        unsafe { abi::rive_rs_factory_ref(self.as_raw()) };
        Self { raw: self.raw }
    }
}

impl Drop for Factory {
    fn drop(&mut self) {
        // SAFETY: intrusive ref-count decrement on valid handle.
        unsafe { abi::rive_rs_factory_unref(self.as_raw()) };
    }
}

pub struct File {
    raw: NonNull<abi::rive_rs_file>,
}

impl File {
    pub fn as_raw(&self) -> *mut abi::rive_rs_file {
        self.raw.as_ptr()
    }

    pub fn artboard_count(&self) -> usize {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_file_artboard_count(self.as_raw()) }
    }

    pub fn default_artboard(&self) -> Result<Artboard, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handle and out pointer.
        let status = unsafe { abi::rive_rs_file_artboard_default(self.as_raw(), &mut out) };
        status_result(status)?;
        Ok(Artboard {
            raw: non_null(out)?,
        })
    }

    pub fn artboard_by_index(&self, index: usize) -> Result<Artboard, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handle and out pointer.
        let status = unsafe { abi::rive_rs_file_artboard_by_index(self.as_raw(), index, &mut out) };
        status_result(status)?;
        Ok(Artboard {
            raw: non_null(out)?,
        })
    }

    pub fn artboard_by_name(&self, name: &str) -> Result<Artboard, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid pointers for call duration.
        let status =
            unsafe { abi::rive_rs_file_artboard_by_name(self.as_raw(), str_view(name), &mut out) };
        status_result(status)?;
        Ok(Artboard {
            raw: non_null(out)?,
        })
    }

    pub fn view_model_count(&self) -> usize {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_file_view_model_count(self.as_raw()) }
    }

    pub fn view_model_by_index(&self, index: usize) -> Result<ViewModel, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handle and out pointer.
        let status =
            unsafe { abi::rive_rs_file_view_model_by_index(self.as_raw(), index, &mut out) };
        status_result(status)?;
        Ok(ViewModel {
            raw: non_null(out)?,
        })
    }

    pub fn view_model_by_name(&self, name: &str) -> Result<ViewModel, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid pointers for call duration.
        let status = unsafe {
            abi::rive_rs_file_view_model_by_name(self.as_raw(), str_view(name), &mut out)
        };
        status_result(status)?;
        Ok(ViewModel {
            raw: non_null(out)?,
        })
    }

    pub fn default_artboard_view_model(&self, artboard: &mut Artboard) -> Result<ViewModel, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handles and out pointer.
        let status = unsafe {
            abi::rive_rs_file_default_artboard_view_model(
                self.as_raw(),
                artboard.as_raw(),
                &mut out,
            )
        };
        status_result(status)?;
        Ok(ViewModel {
            raw: non_null(out)?,
        })
    }

    pub fn bindable_artboard_by_name(&self, name: &str) -> Result<BindableArtboard, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid pointers for call duration.
        let status = unsafe {
            abi::rive_rs_file_bindable_artboard_by_name(self.as_raw(), str_view(name), &mut out)
        };
        status_result(status)?;
        Ok(BindableArtboard {
            raw: non_null(out)?,
        })
    }

    pub fn bindable_artboard_default(&self) -> Result<BindableArtboard, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handle and out pointer.
        let status =
            unsafe { abi::rive_rs_file_bindable_artboard_default(self.as_raw(), &mut out) };
        status_result(status)?;
        Ok(BindableArtboard {
            raw: non_null(out)?,
        })
    }

    pub fn bindable_artboard_from_artboard(
        &self,
        artboard: &mut Artboard,
    ) -> Result<BindableArtboard, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handles and out pointer.
        let status = unsafe {
            abi::rive_rs_file_bindable_artboard_from_artboard(
                self.as_raw(),
                artboard.as_raw(),
                &mut out,
            )
        };
        status_result(status)?;
        Ok(BindableArtboard {
            raw: non_null(out)?,
        })
    }

    pub fn has_audio(&self) -> bool {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_file_has_audio(self.as_raw()) }
    }

    pub fn enum_count(&self) -> usize {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_file_enum_count(self.as_raw()) }
    }

    pub fn enum_name_at(&self, enum_index: usize) -> Result<String, Error> {
        let mut out_name = empty_str_view();
        // SAFETY: valid handle and out pointer.
        let status =
            unsafe { abi::rive_rs_file_enum_name_at(self.as_raw(), enum_index, &mut out_name) };
        status_result(status)?;
        Ok(string_from_view(out_name))
    }

    pub fn enum_value_count(&self, enum_index: usize) -> usize {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_file_enum_value_count(self.as_raw(), enum_index) }
    }

    pub fn enum_value_name_at(
        &self,
        enum_index: usize,
        value_index: usize,
    ) -> Result<String, Error> {
        let mut out_name = empty_str_view();
        // SAFETY: valid handle and out pointer.
        let status = unsafe {
            abi::rive_rs_file_enum_value_name_at(
                self.as_raw(),
                enum_index,
                value_index,
                &mut out_name,
            )
        };
        status_result(status)?;
        Ok(string_from_view(out_name))
    }

    pub fn enums(&self) -> Result<Vec<DataEnumInfo>, Error> {
        let mut enums = Vec::with_capacity(self.enum_count());
        for enum_index in 0..self.enum_count() {
            let name = self.enum_name_at(enum_index)?;
            let value_count = self.enum_value_count(enum_index);
            let mut values = Vec::with_capacity(value_count);
            for value_index in 0..value_count {
                values.push(self.enum_value_name_at(enum_index, value_index)?);
            }
            enums.push(DataEnumInfo { name, values });
        }
        Ok(enums)
    }
}

impl Clone for File {
    fn clone(&self) -> Self {
        // SAFETY: intrusive ref-count increment on valid handle.
        unsafe { abi::rive_rs_file_ref(self.as_raw()) };
        Self { raw: self.raw }
    }
}

impl Drop for File {
    fn drop(&mut self) {
        // SAFETY: intrusive ref-count decrement on valid handle.
        unsafe { abi::rive_rs_file_unref(self.as_raw()) };
    }
}

pub struct Artboard {
    raw: NonNull<abi::rive_rs_artboard>,
}

impl Artboard {
    pub fn as_raw(&self) -> *mut abi::rive_rs_artboard {
        self.raw.as_ptr()
    }

    pub fn advance(&mut self, seconds: f32) -> Result<bool, Error> {
        let mut changed = false;
        // SAFETY: valid handle and out pointer.
        let status = unsafe { abi::rive_rs_artboard_advance(self.as_raw(), seconds, &mut changed) };
        status_result(status)?;
        Ok(changed)
    }

    pub fn draw_raw(&mut self, renderer: *mut abi::rive_rs_renderer) -> Result<(), Error> {
        if renderer.is_null() {
            return Err(Error::null_handle());
        }
        // SAFETY: valid pointers for call duration.
        let status = unsafe { abi::rive_rs_artboard_draw(self.as_raw(), renderer) };
        status_result(status)
    }

    pub fn draw_webgl2(&mut self, renderer: &mut WebGl2Renderer) -> Result<(), Error> {
        // SAFETY: valid handles for call duration.
        let status = unsafe { abi::rive_rs_artboard_draw_webgl2(self.as_raw(), renderer.as_raw()) };
        status_result(status)
    }

    pub fn draw_webgpu(&mut self, renderer: &mut WebGpuRenderer) -> Result<(), Error> {
        // SAFETY: valid handles for call duration.
        let status = unsafe { abi::rive_rs_artboard_draw_webgpu(self.as_raw(), renderer.as_raw()) };
        status_result(status)
    }

    pub fn did_change(&self) -> bool {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_artboard_did_change(self.as_raw()) }
    }

    pub fn name(&self) -> String {
        // SAFETY: valid handle.
        let name = unsafe { abi::rive_rs_artboard_name(self.as_raw()) };
        string_from_view(name)
    }

    pub fn bounds(&self) -> Aabb {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_artboard_bounds(self.as_raw()) }
    }

    pub fn width(&self) -> f32 {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_artboard_width(self.as_raw()) }
    }

    pub fn height(&self) -> f32 {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_artboard_height(self.as_raw()) }
    }

    pub fn set_width(&mut self, width: f32) {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_artboard_set_width(self.as_raw(), width) };
    }

    pub fn set_height(&mut self, height: f32) {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_artboard_set_height(self.as_raw(), height) };
    }

    pub fn frame_origin(&self) -> bool {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_artboard_frame_origin(self.as_raw()) }
    }

    pub fn set_frame_origin(&mut self, frame_origin: bool) {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_artboard_set_frame_origin(self.as_raw(), frame_origin) };
    }

    pub fn has_audio(&self) -> bool {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_artboard_has_audio(self.as_raw()) }
    }

    pub fn volume(&self) -> f32 {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_artboard_volume(self.as_raw()) }
    }

    pub fn set_volume(&mut self, volume: f32) {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_artboard_set_volume(self.as_raw(), volume) };
    }

    pub fn reset_size(&mut self) -> Result<(), Error> {
        // SAFETY: valid handle.
        let status = unsafe { abi::rive_rs_artboard_reset_size(self.as_raw()) };
        status_result(status)
    }

    pub fn animation_count(&self) -> usize {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_artboard_animation_count(self.as_raw()) }
    }

    pub fn state_machine_count(&self) -> usize {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_artboard_state_machine_count(self.as_raw()) }
    }

    pub fn event_count(&self) -> usize {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_artboard_event_count(self.as_raw()) }
    }

    pub fn event_at(&self, index: usize) -> Result<EventInfo, Error> {
        let mut out_event = empty_event_info();
        // SAFETY: valid handle and out pointer.
        let status =
            unsafe { abi::rive_rs_artboard_event_at(self.as_raw(), index, &mut out_event) };
        status_result(status)?;

        let property_count = out_event.property_count;
        let mut properties = Vec::with_capacity(property_count);
        for property_index in 0..property_count {
            let mut out_property = empty_event_property_info();
            // SAFETY: valid handle and out pointer.
            let status = unsafe {
                abi::rive_rs_artboard_event_property_at(
                    self.as_raw(),
                    index,
                    property_index,
                    &mut out_property,
                )
            };
            status_result(status)?;
            properties.push(event_property_from_abi(out_property));
        }

        Ok(EventInfo {
            name: string_from_view(out_event.name),
            event_type: out_event.r#type,
            url: if out_event.has_url {
                Some(string_from_view(out_event.url))
            } else {
                None
            },
            target: if out_event.has_target {
                Some(string_from_view(out_event.target))
            } else {
                None
            },
            properties,
        })
    }

    pub fn animation_by_index(&mut self, index: usize) -> Result<LinearAnimation, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handle and out pointer.
        let status =
            unsafe { abi::rive_rs_artboard_animation_by_index(self.as_raw(), index, &mut out) };
        status_result(status)?;
        Ok(LinearAnimation {
            raw: non_null(out)?,
        })
    }

    pub fn animation_by_name(&mut self, name: &str) -> Result<LinearAnimation, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid pointers for call duration.
        let status = unsafe {
            abi::rive_rs_artboard_animation_by_name(self.as_raw(), str_view(name), &mut out)
        };
        status_result(status)?;
        Ok(LinearAnimation {
            raw: non_null(out)?,
        })
    }

    pub fn state_machine_by_index(&mut self, index: usize) -> Result<StateMachine, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handle and out pointer.
        let status =
            unsafe { abi::rive_rs_artboard_state_machine_by_index(self.as_raw(), index, &mut out) };
        status_result(status)?;
        Ok(StateMachine {
            raw: non_null(out)?,
        })
    }

    pub fn state_machine_by_name(&mut self, name: &str) -> Result<StateMachine, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid pointers for call duration.
        let status = unsafe {
            abi::rive_rs_artboard_state_machine_by_name(self.as_raw(), str_view(name), &mut out)
        };
        status_result(status)?;
        Ok(StateMachine {
            raw: non_null(out)?,
        })
    }

    pub fn input_by_path(&mut self, name: &str, path: &str) -> Result<SmiInput, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handle, string views, and out pointer.
        let status = unsafe {
            abi::rive_rs_artboard_input_by_path(
                self.as_raw(),
                str_view(name),
                str_view(path),
                &mut out,
            )
        };
        status_result(status)?;
        Ok(SmiInput {
            raw: non_null(out)?,
        })
    }

    pub fn text_value_run_count(&self) -> usize {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_artboard_text_value_run_count(self.as_raw()) }
    }

    pub fn text_value_run_name_at(&self, index: usize) -> Result<String, Error> {
        let mut out_name = empty_str_view();
        // SAFETY: valid handle and out pointer.
        let status = unsafe {
            abi::rive_rs_artboard_text_value_run_name_at(self.as_raw(), index, &mut out_name)
        };
        status_result(status)?;
        Ok(string_from_view(out_name))
    }

    pub fn text_value_run_text_at(&self, index: usize) -> Result<String, Error> {
        let mut out_text = empty_str_view();
        // SAFETY: valid handle and out pointer.
        let status = unsafe {
            abi::rive_rs_artboard_text_value_run_text_at(self.as_raw(), index, &mut out_text)
        };
        status_result(status)?;
        Ok(string_from_view(out_text))
    }

    pub fn set_text_value_run_text_at(&mut self, index: usize, text: &str) -> Result<(), Error> {
        // SAFETY: valid handle and string view.
        let status = unsafe {
            abi::rive_rs_artboard_set_text_value_run_text_at(self.as_raw(), index, str_view(text))
        };
        status_result(status)
    }

    pub fn text_by_path(&self, name: &str, path: &str) -> Result<String, Error> {
        let mut out_text = empty_str_view();
        // SAFETY: valid handle, string views, and out pointer.
        let status = unsafe {
            abi::rive_rs_artboard_text_by_path_get(
                self.as_raw(),
                str_view(name),
                str_view(path),
                &mut out_text,
            )
        };
        status_result(status)?;
        Ok(string_from_view(out_text))
    }

    pub fn set_text_by_path(&mut self, name: &str, path: &str, text: &str) -> Result<(), Error> {
        // SAFETY: valid handle and string views.
        let status = unsafe {
            abi::rive_rs_artboard_text_by_path_set(
                self.as_raw(),
                str_view(name),
                str_view(path),
                str_view(text),
            )
        };
        status_result(status)
    }

    pub fn transform_component_by_name(
        &mut self,
        name: &str,
    ) -> Result<TransformComponentHandle, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handle, string view, and out pointer.
        let status = unsafe {
            abi::rive_rs_artboard_transform_component_by_name(
                self.as_raw(),
                str_view(name),
                &mut out,
            )
        };
        status_result(status)?;
        Ok(TransformComponentHandle {
            raw: non_null(out)?,
        })
    }

    pub fn node_by_name(&mut self, name: &str) -> Result<NodeHandle, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handle, string view, and out pointer.
        let status =
            unsafe { abi::rive_rs_artboard_node_by_name(self.as_raw(), str_view(name), &mut out) };
        status_result(status)?;
        Ok(NodeHandle {
            raw: non_null(out)?,
        })
    }

    pub fn bone_by_name(&mut self, name: &str) -> Result<BoneHandle, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handle, string view, and out pointer.
        let status =
            unsafe { abi::rive_rs_artboard_bone_by_name(self.as_raw(), str_view(name), &mut out) };
        status_result(status)?;
        Ok(BoneHandle {
            raw: non_null(out)?,
        })
    }

    pub fn root_bone_by_name(&mut self, name: &str) -> Result<RootBoneHandle, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handle, string view, and out pointer.
        let status = unsafe {
            abi::rive_rs_artboard_root_bone_by_name(self.as_raw(), str_view(name), &mut out)
        };
        status_result(status)?;
        Ok(RootBoneHandle {
            raw: non_null(out)?,
        })
    }

    pub fn text_value_run_by_name(&mut self, name: &str) -> Result<TextValueRunHandle, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handle, string view, and out pointer.
        let status = unsafe {
            abi::rive_rs_artboard_text_value_run_by_name(self.as_raw(), str_view(name), &mut out)
        };
        status_result(status)?;
        Ok(TextValueRunHandle {
            raw: non_null(out)?,
        })
    }

    pub fn text_value_run_by_index(&mut self, index: usize) -> Result<TextValueRunHandle, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handle and out pointer.
        let status = unsafe {
            abi::rive_rs_artboard_text_value_run_by_index(self.as_raw(), index, &mut out)
        };
        status_result(status)?;
        Ok(TextValueRunHandle {
            raw: non_null(out)?,
        })
    }

    pub fn flatten_path(
        &mut self,
        index: usize,
        transform_to_parent: bool,
    ) -> Result<FlattenedPath, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handle and out pointer.
        let status = unsafe {
            abi::rive_rs_artboard_flatten_path(self.as_raw(), index, transform_to_parent, &mut out)
        };
        status_result(status)?;
        Ok(FlattenedPath {
            raw: non_null(out)?,
        })
    }

    pub fn bind_view_model_instance(&mut self, instance: &ViewModelInstance) -> Result<(), Error> {
        // SAFETY: valid handles for call duration.
        let status = unsafe {
            abi::rive_rs_artboard_bind_view_model_instance(self.as_raw(), instance.as_raw())
        };
        status_result(status)
    }
}

impl Clone for Artboard {
    fn clone(&self) -> Self {
        // SAFETY: intrusive ref-count increment on valid handle.
        unsafe { abi::rive_rs_artboard_ref(self.as_raw()) };
        Self { raw: self.raw }
    }
}

impl Drop for Artboard {
    fn drop(&mut self) {
        // SAFETY: intrusive ref-count decrement on valid handle.
        unsafe { abi::rive_rs_artboard_unref(self.as_raw()) };
    }
}

pub struct WebGl2Renderer {
    raw: NonNull<abi::rive_rs_webgl2_renderer>,
}

impl WebGl2Renderer {
    pub fn new(width: i32, height: i32) -> Result<Self, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: out pointer is valid for call duration.
        let status = unsafe { abi::rive_rs_webgl2_renderer_new(width, height, &mut out) };
        status_result(status)?;
        Ok(Self {
            raw: non_null(out)?,
        })
    }

    pub fn as_raw(&self) -> *mut abi::rive_rs_webgl2_renderer {
        self.raw.as_ptr()
    }

    pub fn clear(&mut self) -> Result<(), Error> {
        // SAFETY: valid handle.
        let status = unsafe { abi::rive_rs_webgl2_renderer_clear(self.as_raw()) };
        status_result(status)
    }

    pub fn flush(&mut self) -> Result<(), Error> {
        // SAFETY: valid handle.
        let status = unsafe { abi::rive_rs_webgl2_renderer_flush(self.as_raw()) };
        status_result(status)
    }

    pub fn resize(&mut self, width: i32, height: i32) -> Result<(), Error> {
        // SAFETY: valid handle.
        let status = unsafe { abi::rive_rs_webgl2_renderer_resize(self.as_raw(), width, height) };
        status_result(status)
    }

    pub fn save(&mut self) -> Result<(), Error> {
        // SAFETY: valid handle.
        let status = unsafe { abi::rive_rs_webgl2_renderer_save(self.as_raw()) };
        status_result(status)
    }

    pub fn restore(&mut self) -> Result<(), Error> {
        // SAFETY: valid handle.
        let status = unsafe { abi::rive_rs_webgl2_renderer_restore(self.as_raw()) };
        status_result(status)
    }

    pub fn transform(&mut self, matrix: &Mat2D) -> Result<(), Error> {
        // SAFETY: valid pointers for call duration.
        let status = unsafe { abi::rive_rs_webgl2_renderer_transform(self.as_raw(), matrix) };
        status_result(status)
    }

    pub fn modulate_opacity(&mut self, opacity: f32) -> Result<(), Error> {
        // SAFETY: valid handle.
        let status =
            unsafe { abi::rive_rs_webgl2_renderer_modulate_opacity(self.as_raw(), opacity) };
        status_result(status)
    }

    pub fn align(
        &mut self,
        fit: Fit,
        alignment: Alignment,
        frame: &Aabb,
        content: &Aabb,
        scale_factor: f32,
    ) -> Result<(), Error> {
        // SAFETY: valid pointers for call duration.
        let status = unsafe {
            abi::rive_rs_webgl2_renderer_align(
                self.as_raw(),
                fit,
                alignment,
                frame,
                content,
                scale_factor,
            )
        };
        status_result(status)
    }

    pub fn save_clip_rect(
        &mut self,
        left: f32,
        top: f32,
        right: f32,
        bottom: f32,
    ) -> Result<(), Error> {
        // SAFETY: valid handle.
        let status = unsafe {
            abi::rive_rs_webgl2_renderer_save_clip_rect(self.as_raw(), left, top, right, bottom)
        };
        status_result(status)
    }

    pub fn restore_clip_rect(&mut self) -> Result<(), Error> {
        // SAFETY: valid handle.
        let status = unsafe { abi::rive_rs_webgl2_renderer_restore_clip_rect(self.as_raw()) };
        status_result(status)
    }
}

impl Drop for WebGl2Renderer {
    fn drop(&mut self) {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_webgl2_renderer_delete(self.as_raw()) };
    }
}

pub struct WebGpuRenderer {
    raw: NonNull<abi::rive_rs_webgpu_renderer>,
}

impl WebGpuRenderer {
    pub fn new(width: i32, height: i32) -> Result<Self, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: out pointer is valid for call duration.
        let status = unsafe { abi::rive_rs_webgpu_renderer_new(width, height, &mut out) };
        status_result(status)?;
        Ok(Self {
            raw: non_null(out)?,
        })
    }

    pub fn as_raw(&self) -> *mut abi::rive_rs_webgpu_renderer {
        self.raw.as_ptr()
    }

    pub fn clear(&mut self) -> Result<(), Error> {
        // SAFETY: valid handle.
        let status = unsafe { abi::rive_rs_webgpu_renderer_clear(self.as_raw()) };
        status_result(status)
    }

    pub fn flush(&mut self) -> Result<(), Error> {
        // SAFETY: valid handle.
        let status = unsafe { abi::rive_rs_webgpu_renderer_flush(self.as_raw()) };
        status_result(status)
    }

    pub fn resize(&mut self, width: i32, height: i32) -> Result<(), Error> {
        // SAFETY: valid handle.
        let status = unsafe { abi::rive_rs_webgpu_renderer_resize(self.as_raw(), width, height) };
        status_result(status)
    }

    pub fn save(&mut self) -> Result<(), Error> {
        // SAFETY: valid handle.
        let status = unsafe { abi::rive_rs_webgpu_renderer_save(self.as_raw()) };
        status_result(status)
    }

    pub fn restore(&mut self) -> Result<(), Error> {
        // SAFETY: valid handle.
        let status = unsafe { abi::rive_rs_webgpu_renderer_restore(self.as_raw()) };
        status_result(status)
    }

    pub fn transform(&mut self, matrix: &Mat2D) -> Result<(), Error> {
        // SAFETY: valid pointers for call duration.
        let status = unsafe { abi::rive_rs_webgpu_renderer_transform(self.as_raw(), matrix) };
        status_result(status)
    }

    pub fn modulate_opacity(&mut self, opacity: f32) -> Result<(), Error> {
        // SAFETY: valid handle.
        let status =
            unsafe { abi::rive_rs_webgpu_renderer_modulate_opacity(self.as_raw(), opacity) };
        status_result(status)
    }

    pub fn align(
        &mut self,
        fit: Fit,
        alignment: Alignment,
        frame: &Aabb,
        content: &Aabb,
        scale_factor: f32,
    ) -> Result<(), Error> {
        // SAFETY: valid pointers for call duration.
        let status = unsafe {
            abi::rive_rs_webgpu_renderer_align(
                self.as_raw(),
                fit,
                alignment,
                frame,
                content,
                scale_factor,
            )
        };
        status_result(status)
    }

    pub fn save_clip_rect(
        &mut self,
        left: f32,
        top: f32,
        right: f32,
        bottom: f32,
    ) -> Result<(), Error> {
        // SAFETY: valid handle.
        let status = unsafe {
            abi::rive_rs_webgpu_renderer_save_clip_rect(self.as_raw(), left, top, right, bottom)
        };
        status_result(status)
    }

    pub fn restore_clip_rect(&mut self) -> Result<(), Error> {
        // SAFETY: valid handle.
        let status = unsafe { abi::rive_rs_webgpu_renderer_restore_clip_rect(self.as_raw()) };
        status_result(status)
    }
}

impl Drop for WebGpuRenderer {
    fn drop(&mut self) {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_webgpu_renderer_delete(self.as_raw()) };
    }
}

#[derive(Copy, Clone)]
pub struct TransformComponentHandle {
    raw: NonNull<abi::rive_rs_transform_component>,
}

impl TransformComponentHandle {
    pub fn as_raw(&self) -> *mut abi::rive_rs_transform_component {
        self.raw.as_ptr()
    }

    pub fn scale_x(&self) -> f32 {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_transform_component_scale_x(self.as_raw()) }
    }

    pub fn set_scale_x(&mut self, value: f32) {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_transform_component_set_scale_x(self.as_raw(), value) };
    }

    pub fn scale_y(&self) -> f32 {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_transform_component_scale_y(self.as_raw()) }
    }

    pub fn set_scale_y(&mut self, value: f32) {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_transform_component_set_scale_y(self.as_raw(), value) };
    }

    pub fn rotation(&self) -> f32 {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_transform_component_rotation(self.as_raw()) }
    }

    pub fn set_rotation(&mut self, value: f32) {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_transform_component_set_rotation(self.as_raw(), value) };
    }

    pub fn world_transform(&self) -> Result<Mat2D, Error> {
        let mut out = Mat2D {
            xx: 0.0,
            xy: 0.0,
            yx: 0.0,
            yy: 0.0,
            tx: 0.0,
            ty: 0.0,
        };
        // SAFETY: valid handle and out pointer.
        let status =
            unsafe { abi::rive_rs_transform_component_world_transform(self.as_raw(), &mut out) };
        status_result(status)?;
        Ok(out)
    }

    pub fn parent_world_transform(&self) -> Result<Mat2D, Error> {
        let mut out = Mat2D {
            xx: 0.0,
            xy: 0.0,
            yx: 0.0,
            yy: 0.0,
            tx: 0.0,
            ty: 0.0,
        };
        // SAFETY: valid handle and out pointer.
        let status = unsafe {
            abi::rive_rs_transform_component_parent_world_transform(self.as_raw(), &mut out)
        };
        status_result(status)?;
        Ok(out)
    }
}

#[derive(Copy, Clone)]
pub struct NodeHandle {
    raw: NonNull<abi::rive_rs_node>,
}

impl NodeHandle {
    pub fn as_raw(&self) -> *mut abi::rive_rs_node {
        self.raw.as_ptr()
    }

    pub fn x(&self) -> f32 {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_node_x(self.as_raw()) }
    }

    pub fn set_x(&mut self, value: f32) {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_node_set_x(self.as_raw(), value) };
    }

    pub fn y(&self) -> f32 {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_node_y(self.as_raw()) }
    }

    pub fn set_y(&mut self, value: f32) {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_node_set_y(self.as_raw(), value) };
    }
}

#[derive(Copy, Clone)]
pub struct BoneHandle {
    raw: NonNull<abi::rive_rs_bone>,
}

impl BoneHandle {
    pub fn as_raw(&self) -> *mut abi::rive_rs_bone {
        self.raw.as_ptr()
    }

    pub fn length(&self) -> f32 {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_bone_length(self.as_raw()) }
    }

    pub fn set_length(&mut self, value: f32) {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_bone_set_length(self.as_raw(), value) };
    }
}

#[derive(Copy, Clone)]
pub struct RootBoneHandle {
    raw: NonNull<abi::rive_rs_root_bone>,
}

impl RootBoneHandle {
    pub fn as_raw(&self) -> *mut abi::rive_rs_root_bone {
        self.raw.as_ptr()
    }

    pub fn x(&self) -> f32 {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_root_bone_x(self.as_raw()) }
    }

    pub fn set_x(&mut self, value: f32) {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_root_bone_set_x(self.as_raw(), value) };
    }

    pub fn y(&self) -> f32 {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_root_bone_y(self.as_raw()) }
    }

    pub fn set_y(&mut self, value: f32) {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_root_bone_set_y(self.as_raw(), value) };
    }
}

#[derive(Copy, Clone)]
pub struct TextValueRunHandle {
    raw: NonNull<abi::rive_rs_text_value_run>,
}

impl TextValueRunHandle {
    pub fn as_raw(&self) -> *mut abi::rive_rs_text_value_run {
        self.raw.as_ptr()
    }

    pub fn name(&self) -> String {
        // SAFETY: valid handle.
        let name = unsafe { abi::rive_rs_text_value_run_name(self.as_raw()) };
        string_from_view(name)
    }

    pub fn text(&self) -> String {
        // SAFETY: valid handle.
        let text = unsafe { abi::rive_rs_text_value_run_text(self.as_raw()) };
        string_from_view(text)
    }

    pub fn set_text(&mut self, text: &str) -> Result<(), Error> {
        // SAFETY: valid handle and string view.
        let status = unsafe { abi::rive_rs_text_value_run_set_text(self.as_raw(), str_view(text)) };
        status_result(status)
    }
}

pub struct FlattenedPath {
    raw: NonNull<abi::rive_rs_flattened_path>,
}

impl FlattenedPath {
    pub fn as_raw(&self) -> *mut abi::rive_rs_flattened_path {
        self.raw.as_ptr()
    }

    pub fn length(&self) -> usize {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_flattened_path_length(self.as_raw()) }
    }

    pub fn is_cubic(&self, index: usize) -> Result<bool, Error> {
        let mut out_is_cubic = false;
        // SAFETY: valid handle and out pointer.
        let status = unsafe {
            abi::rive_rs_flattened_path_is_cubic(self.as_raw(), index, &mut out_is_cubic)
        };
        status_result(status)?;
        Ok(out_is_cubic)
    }

    pub fn x(&self, index: usize) -> Result<f32, Error> {
        let mut out_value = 0.0f32;
        // SAFETY: valid handle and out pointer.
        let status = unsafe { abi::rive_rs_flattened_path_x(self.as_raw(), index, &mut out_value) };
        status_result(status)?;
        Ok(out_value)
    }

    pub fn y(&self, index: usize) -> Result<f32, Error> {
        let mut out_value = 0.0f32;
        // SAFETY: valid handle and out pointer.
        let status = unsafe { abi::rive_rs_flattened_path_y(self.as_raw(), index, &mut out_value) };
        status_result(status)?;
        Ok(out_value)
    }

    pub fn in_x(&self, index: usize) -> Result<f32, Error> {
        let mut out_value = 0.0f32;
        // SAFETY: valid handle and out pointer.
        let status =
            unsafe { abi::rive_rs_flattened_path_in_x(self.as_raw(), index, &mut out_value) };
        status_result(status)?;
        Ok(out_value)
    }

    pub fn in_y(&self, index: usize) -> Result<f32, Error> {
        let mut out_value = 0.0f32;
        // SAFETY: valid handle and out pointer.
        let status =
            unsafe { abi::rive_rs_flattened_path_in_y(self.as_raw(), index, &mut out_value) };
        status_result(status)?;
        Ok(out_value)
    }

    pub fn out_x(&self, index: usize) -> Result<f32, Error> {
        let mut out_value = 0.0f32;
        // SAFETY: valid handle and out pointer.
        let status =
            unsafe { abi::rive_rs_flattened_path_out_x(self.as_raw(), index, &mut out_value) };
        status_result(status)?;
        Ok(out_value)
    }

    pub fn out_y(&self, index: usize) -> Result<f32, Error> {
        let mut out_value = 0.0f32;
        // SAFETY: valid handle and out pointer.
        let status =
            unsafe { abi::rive_rs_flattened_path_out_y(self.as_raw(), index, &mut out_value) };
        status_result(status)?;
        Ok(out_value)
    }
}

impl Drop for FlattenedPath {
    fn drop(&mut self) {
        // SAFETY: this is the owning flattened path handle.
        unsafe { abi::rive_rs_flattened_path_delete(self.as_raw()) };
    }
}

pub struct BindableArtboard {
    raw: NonNull<abi::rive_rs_bindable_artboard>,
}

impl BindableArtboard {
    pub fn as_raw(&self) -> *mut abi::rive_rs_bindable_artboard {
        self.raw.as_ptr()
    }
}

impl Clone for BindableArtboard {
    fn clone(&self) -> Self {
        // SAFETY: intrusive ref-count increment on valid handle.
        unsafe { abi::rive_rs_bindable_artboard_ref(self.as_raw()) };
        Self { raw: self.raw }
    }
}

impl Drop for BindableArtboard {
    fn drop(&mut self) {
        // SAFETY: intrusive ref-count decrement on valid handle.
        unsafe { abi::rive_rs_bindable_artboard_unref(self.as_raw()) };
    }
}

pub struct ViewModel {
    raw: NonNull<abi::rive_rs_view_model>,
}

impl ViewModel {
    pub fn as_raw(&self) -> *mut abi::rive_rs_view_model {
        self.raw.as_ptr()
    }

    pub fn name(&self) -> String {
        // SAFETY: valid handle.
        let name = unsafe { abi::rive_rs_view_model_name(self.as_raw()) };
        string_from_view(name)
    }

    pub fn property_count(&self) -> usize {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_view_model_property_count(self.as_raw()) }
    }

    pub fn instance_count(&self) -> usize {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_view_model_instance_count(self.as_raw()) }
    }

    pub fn property_at(&self, index: usize) -> Result<PropertyInfo, Error> {
        let mut out = abi::rive_rs_property_info {
            name: abi::rive_rs_str_view {
                ptr: ptr::null::<c_char>(),
                len: 0,
            },
            data_type: DataType::RIVE_RS_DATA_TYPE_NONE,
        };
        // SAFETY: valid handle and out pointer.
        let status = unsafe { abi::rive_rs_view_model_property_at(self.as_raw(), index, &mut out) };
        status_result(status)?;
        Ok(property_info_from_abi(out))
    }

    pub fn instance_name_at(&self, index: usize) -> Result<String, Error> {
        let mut out_name = abi::rive_rs_str_view {
            ptr: ptr::null::<c_char>(),
            len: 0,
        };
        // SAFETY: valid handle and out pointer.
        let status = unsafe {
            abi::rive_rs_view_model_instance_name_at(self.as_raw(), index, &mut out_name)
        };
        status_result(status)?;
        Ok(string_from_view(out_name))
    }

    pub fn instance_by_index(&self, index: usize) -> Result<ViewModelInstance, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handle and out pointer.
        let status =
            unsafe { abi::rive_rs_view_model_instance_by_index(self.as_raw(), index, &mut out) };
        status_result(status)?;
        Ok(ViewModelInstance {
            raw: non_null(out)?,
        })
    }

    pub fn instance_by_name(&self, name: &str) -> Result<ViewModelInstance, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid pointers for call duration.
        let status = unsafe {
            abi::rive_rs_view_model_instance_by_name(self.as_raw(), str_view(name), &mut out)
        };
        status_result(status)?;
        Ok(ViewModelInstance {
            raw: non_null(out)?,
        })
    }

    pub fn default_instance(&self) -> Result<ViewModelInstance, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handle and out pointer.
        let status = unsafe { abi::rive_rs_view_model_default_instance(self.as_raw(), &mut out) };
        status_result(status)?;
        Ok(ViewModelInstance {
            raw: non_null(out)?,
        })
    }

    pub fn new_instance(&self) -> Result<ViewModelInstance, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handle and out pointer.
        let status = unsafe { abi::rive_rs_view_model_new_instance(self.as_raw(), &mut out) };
        status_result(status)?;
        Ok(ViewModelInstance {
            raw: non_null(out)?,
        })
    }
}

impl Clone for ViewModel {
    fn clone(&self) -> Self {
        // SAFETY: intrusive ref-count increment on valid handle.
        unsafe { abi::rive_rs_view_model_ref(self.as_raw()) };
        Self { raw: self.raw }
    }
}

impl Drop for ViewModel {
    fn drop(&mut self) {
        // SAFETY: intrusive ref-count decrement on valid handle.
        unsafe { abi::rive_rs_view_model_unref(self.as_raw()) };
    }
}

pub struct ViewModelInstance {
    raw: NonNull<abi::rive_rs_view_model_instance>,
}

impl ViewModelInstance {
    pub fn as_raw(&self) -> *mut abi::rive_rs_view_model_instance {
        self.raw.as_ptr()
    }

    pub fn property_count(&self) -> usize {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_view_model_instance_property_count(self.as_raw()) }
    }

    pub fn property_at(&self, index: usize) -> Result<PropertyInfo, Error> {
        let mut out = abi::rive_rs_property_info {
            name: abi::rive_rs_str_view {
                ptr: ptr::null::<c_char>(),
                len: 0,
            },
            data_type: DataType::RIVE_RS_DATA_TYPE_NONE,
        };
        // SAFETY: valid handle and out pointer.
        let status =
            unsafe { abi::rive_rs_view_model_instance_property_at(self.as_raw(), index, &mut out) };
        status_result(status)?;
        Ok(property_info_from_abi(out))
    }

    pub fn number(&self, path: &str) -> Result<f32, Error> {
        let mut out_value = 0.0f32;
        // SAFETY: valid handle, path view, and out pointer.
        let status = unsafe {
            abi::rive_rs_view_model_instance_get_number(
                self.as_raw(),
                str_view(path),
                &mut out_value,
            )
        };
        status_result(status)?;
        Ok(out_value)
    }

    pub fn set_number(&mut self, path: &str, value: f32) -> Result<(), Error> {
        // SAFETY: valid handle and path view.
        let status = unsafe {
            abi::rive_rs_view_model_instance_set_number(self.as_raw(), str_view(path), value)
        };
        status_result(status)
    }

    pub fn string(&self, path: &str) -> Result<String, Error> {
        let mut out_value = empty_str_view();
        // SAFETY: valid handle, path view, and out pointer.
        let status = unsafe {
            abi::rive_rs_view_model_instance_get_string(
                self.as_raw(),
                str_view(path),
                &mut out_value,
            )
        };
        status_result(status)?;
        Ok(string_from_view(out_value))
    }

    pub fn set_string(&mut self, path: &str, value: &str) -> Result<(), Error> {
        // SAFETY: valid handle and string views.
        let status = unsafe {
            abi::rive_rs_view_model_instance_set_string(
                self.as_raw(),
                str_view(path),
                str_view(value),
            )
        };
        status_result(status)
    }

    pub fn boolean(&self, path: &str) -> Result<bool, Error> {
        let mut out_value = false;
        // SAFETY: valid handle, path view, and out pointer.
        let status = unsafe {
            abi::rive_rs_view_model_instance_get_boolean(
                self.as_raw(),
                str_view(path),
                &mut out_value,
            )
        };
        status_result(status)?;
        Ok(out_value)
    }

    pub fn set_boolean(&mut self, path: &str, value: bool) -> Result<(), Error> {
        // SAFETY: valid handle and path view.
        let status = unsafe {
            abi::rive_rs_view_model_instance_set_boolean(self.as_raw(), str_view(path), value)
        };
        status_result(status)
    }

    pub fn color(&self, path: &str) -> Result<i32, Error> {
        let mut out_argb = 0i32;
        // SAFETY: valid handle, path view, and out pointer.
        let status = unsafe {
            abi::rive_rs_view_model_instance_get_color(self.as_raw(), str_view(path), &mut out_argb)
        };
        status_result(status)?;
        Ok(out_argb)
    }

    pub fn set_color(&mut self, path: &str, argb: i32) -> Result<(), Error> {
        // SAFETY: valid handle and path view.
        let status = unsafe {
            abi::rive_rs_view_model_instance_set_color(self.as_raw(), str_view(path), argb)
        };
        status_result(status)
    }

    pub fn enum_value(&self, path: &str) -> Result<String, Error> {
        let mut out_value = empty_str_view();
        // SAFETY: valid handle, path view, and out pointer.
        let status = unsafe {
            abi::rive_rs_view_model_instance_get_enum(self.as_raw(), str_view(path), &mut out_value)
        };
        status_result(status)?;
        Ok(string_from_view(out_value))
    }

    pub fn set_enum_value(&mut self, path: &str, value: &str) -> Result<(), Error> {
        // SAFETY: valid handle and string views.
        let status = unsafe {
            abi::rive_rs_view_model_instance_set_enum(
                self.as_raw(),
                str_view(path),
                str_view(value),
            )
        };
        status_result(status)
    }

    pub fn enum_index(&self, path: &str) -> Result<u32, Error> {
        let mut out_index = 0u32;
        // SAFETY: valid handle, path view, and out pointer.
        let status = unsafe {
            abi::rive_rs_view_model_instance_get_enum_index(
                self.as_raw(),
                str_view(path),
                &mut out_index,
            )
        };
        status_result(status)?;
        Ok(out_index)
    }

    pub fn set_enum_index(&mut self, path: &str, index: u32) -> Result<(), Error> {
        // SAFETY: valid handle and path view.
        let status = unsafe {
            abi::rive_rs_view_model_instance_set_enum_index(self.as_raw(), str_view(path), index)
        };
        status_result(status)
    }

    pub fn fire_trigger(&mut self, path: &str) -> Result<(), Error> {
        // SAFETY: valid handle and path view.
        let status =
            unsafe { abi::rive_rs_view_model_instance_fire_trigger(self.as_raw(), str_view(path)) };
        status_result(status)
    }

    pub fn view_model(&self, path: &str) -> Result<ViewModelInstance, Error> {
        let mut out_instance = ptr::null_mut();
        // SAFETY: valid handle, path view, and out pointer.
        let status = unsafe {
            abi::rive_rs_view_model_instance_get_view_model(
                self.as_raw(),
                str_view(path),
                &mut out_instance,
            )
        };
        status_result(status)?;
        Ok(ViewModelInstance {
            raw: non_null(out_instance)?,
        })
    }

    pub fn replace_view_model(
        &mut self,
        path: &str,
        value: &ViewModelInstance,
    ) -> Result<(), Error> {
        // SAFETY: valid handles and path view.
        let status = unsafe {
            abi::rive_rs_view_model_instance_replace_view_model(
                self.as_raw(),
                str_view(path),
                value.as_raw(),
            )
        };
        status_result(status)
    }

    pub fn property_has_changed(&self, path: &str) -> Result<bool, Error> {
        let mut out_changed = false;
        // SAFETY: valid handle, path view, and out pointer.
        let status = unsafe {
            abi::rive_rs_view_model_instance_property_has_changed(
                self.as_raw(),
                str_view(path),
                &mut out_changed,
            )
        };
        status_result(status)?;
        Ok(out_changed)
    }

    pub fn clear_property_changes(&mut self, path: &str) -> Result<(), Error> {
        // SAFETY: valid handle and path view.
        let status = unsafe {
            abi::rive_rs_view_model_instance_clear_property_changes(self.as_raw(), str_view(path))
        };
        status_result(status)
    }

    pub fn list_size(&self, path: &str) -> Result<usize, Error> {
        let mut out_size = 0usize;
        // SAFETY: valid handle, path view, and out pointer.
        let status = unsafe {
            abi::rive_rs_view_model_instance_list_size(self.as_raw(), str_view(path), &mut out_size)
        };
        status_result(status)?;
        Ok(out_size)
    }

    pub fn list_instance_at(&self, path: &str, index: usize) -> Result<ViewModelInstance, Error> {
        let mut out_value = ptr::null_mut();
        // SAFETY: valid handle, path view, and out pointer.
        let status = unsafe {
            abi::rive_rs_view_model_instance_list_instance_at(
                self.as_raw(),
                str_view(path),
                index,
                &mut out_value,
            )
        };
        status_result(status)?;
        Ok(ViewModelInstance {
            raw: non_null(out_value)?,
        })
    }

    pub fn list_add_instance(
        &mut self,
        path: &str,
        value: &ViewModelInstance,
    ) -> Result<(), Error> {
        // SAFETY: valid handles and path view.
        let status = unsafe {
            abi::rive_rs_view_model_instance_list_add_instance(
                self.as_raw(),
                str_view(path),
                value.as_raw(),
            )
        };
        status_result(status)
    }

    pub fn list_add_instance_at(
        &mut self,
        path: &str,
        value: &ViewModelInstance,
        index: usize,
    ) -> Result<bool, Error> {
        let mut out_added = false;
        // SAFETY: valid handles, path view, and out pointer.
        let status = unsafe {
            abi::rive_rs_view_model_instance_list_add_instance_at(
                self.as_raw(),
                str_view(path),
                value.as_raw(),
                index,
                &mut out_added,
            )
        };
        status_result(status)?;
        Ok(out_added)
    }

    pub fn list_remove_instance(
        &mut self,
        path: &str,
        value: &ViewModelInstance,
    ) -> Result<(), Error> {
        // SAFETY: valid handles and path view.
        let status = unsafe {
            abi::rive_rs_view_model_instance_list_remove_instance(
                self.as_raw(),
                str_view(path),
                value.as_raw(),
            )
        };
        status_result(status)
    }

    pub fn list_remove_instance_at(&mut self, path: &str, index: usize) -> Result<(), Error> {
        // SAFETY: valid handle and path view.
        let status = unsafe {
            abi::rive_rs_view_model_instance_list_remove_instance_at(
                self.as_raw(),
                str_view(path),
                index,
            )
        };
        status_result(status)
    }

    pub fn list_swap(&mut self, path: &str, a: u32, b: u32) -> Result<(), Error> {
        // SAFETY: valid handle and path view.
        let status = unsafe {
            abi::rive_rs_view_model_instance_list_swap(self.as_raw(), str_view(path), a, b)
        };
        status_result(status)
    }

    pub fn set_artboard(&mut self, path: &str, value: &BindableArtboard) -> Result<(), Error> {
        // SAFETY: valid handles and path view.
        let status = unsafe {
            abi::rive_rs_view_model_instance_set_artboard(
                self.as_raw(),
                str_view(path),
                value.as_raw(),
            )
        };
        status_result(status)
    }

    pub fn set_artboard_view_model(
        &mut self,
        path: &str,
        view_model_instance: &ViewModelInstance,
    ) -> Result<(), Error> {
        // SAFETY: valid handles and path view.
        let status = unsafe {
            abi::rive_rs_view_model_instance_set_artboard_view_model(
                self.as_raw(),
                str_view(path),
                view_model_instance.as_raw(),
            )
        };
        status_result(status)
    }

    pub fn set_image(&mut self, path: &str, value: Option<&RenderImage>) -> Result<(), Error> {
        let raw = value.map_or(ptr::null_mut(), |image| image.as_raw());
        // SAFETY: valid handles and path view.
        let status = unsafe {
            abi::rive_rs_view_model_instance_set_image(self.as_raw(), str_view(path), raw)
        };
        status_result(status)
    }

    pub fn image(&self, path: &str) -> Result<Option<RenderImage>, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handle, path view, and out pointer.
        let status = unsafe {
            abi::rive_rs_view_model_instance_get_image(self.as_raw(), str_view(path), &mut out)
        };
        status_result(status)?;
        Ok(NonNull::new(out).map(|raw| RenderImage { raw }))
    }
}

impl Clone for ViewModelInstance {
    fn clone(&self) -> Self {
        // SAFETY: intrusive ref-count increment on valid handle.
        unsafe { abi::rive_rs_view_model_instance_ref(self.as_raw()) };
        Self { raw: self.raw }
    }
}

impl Drop for ViewModelInstance {
    fn drop(&mut self) {
        // SAFETY: intrusive ref-count decrement on valid handle.
        unsafe { abi::rive_rs_view_model_instance_unref(self.as_raw()) };
    }
}

#[derive(Copy, Clone)]
pub struct LinearAnimation {
    raw: NonNull<abi::rive_rs_linear_animation>,
}

impl LinearAnimation {
    pub fn as_raw(&self) -> *mut abi::rive_rs_linear_animation {
        self.raw.as_ptr()
    }

    pub fn name(&self) -> String {
        // SAFETY: valid handle.
        let name = unsafe { abi::rive_rs_linear_animation_name(self.as_raw()) };
        string_from_view(name)
    }

    pub fn duration(&self) -> u32 {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_linear_animation_duration(self.as_raw()) }
    }

    pub fn fps(&self) -> u32 {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_linear_animation_fps(self.as_raw()) }
    }

    pub fn work_start(&self) -> u32 {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_linear_animation_work_start(self.as_raw()) }
    }

    pub fn work_end(&self) -> u32 {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_linear_animation_work_end(self.as_raw()) }
    }

    pub fn enable_work_area(&self) -> bool {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_linear_animation_enable_work_area(self.as_raw()) }
    }

    pub fn loop_value(&self) -> u32 {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_linear_animation_loop_value(self.as_raw()) }
    }

    pub fn speed(&self) -> f32 {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_linear_animation_speed(self.as_raw()) }
    }

    pub fn apply(&self, artboard: &mut Artboard, time: f32, mix: f32) -> Result<(), Error> {
        // SAFETY: valid handles for call duration.
        let status = unsafe {
            abi::rive_rs_linear_animation_apply(self.as_raw(), artboard.as_raw(), time, mix)
        };
        status_result(status)
    }
}

pub struct LinearAnimationInstance {
    raw: NonNull<abi::rive_rs_linear_animation_instance>,
}

impl LinearAnimationInstance {
    pub fn new(animation: LinearAnimation, artboard: &mut Artboard) -> Result<Self, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handles and out pointer.
        let status = unsafe {
            abi::rive_rs_linear_animation_instance_new(
                animation.as_raw(),
                artboard.as_raw(),
                &mut out,
            )
        };
        status_result(status)?;
        Ok(Self {
            raw: non_null(out)?,
        })
    }

    pub fn as_raw(&self) -> *mut abi::rive_rs_linear_animation_instance {
        self.raw.as_ptr()
    }

    pub fn advance(&mut self, seconds: f32) -> Result<bool, Error> {
        let mut looped = false;
        // SAFETY: valid handle and out pointer.
        let status = unsafe {
            abi::rive_rs_linear_animation_instance_advance(self.as_raw(), seconds, &mut looped)
        };
        status_result(status)?;
        Ok(looped)
    }

    pub fn apply(&mut self, artboard: &mut Artboard, mix: f32) -> Result<(), Error> {
        // SAFETY: valid handles for call duration.
        let status = unsafe {
            abi::rive_rs_linear_animation_instance_apply(self.as_raw(), artboard.as_raw(), mix)
        };
        status_result(status)
    }

    pub fn time(&self) -> f32 {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_linear_animation_instance_time(self.as_raw()) }
    }

    pub fn set_time(&mut self, seconds: f32) {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_linear_animation_instance_set_time(self.as_raw(), seconds) };
    }

    pub fn did_loop(&self) -> bool {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_linear_animation_instance_did_loop(self.as_raw()) }
    }
}

impl Drop for LinearAnimationInstance {
    fn drop(&mut self) {
        // SAFETY: this is the owning instance handle.
        unsafe { abi::rive_rs_linear_animation_instance_delete(self.as_raw()) };
    }
}

#[derive(Copy, Clone)]
pub struct StateMachine {
    raw: NonNull<abi::rive_rs_state_machine>,
}

impl StateMachine {
    pub fn as_raw(&self) -> *mut abi::rive_rs_state_machine {
        self.raw.as_ptr()
    }

    pub fn name(&self) -> String {
        // SAFETY: valid handle.
        let name = unsafe { abi::rive_rs_state_machine_name(self.as_raw()) };
        string_from_view(name)
    }
}

pub struct StateMachineInstance {
    raw: NonNull<abi::rive_rs_state_machine_instance>,
}

impl StateMachineInstance {
    pub fn new(state_machine: StateMachine, artboard: &mut Artboard) -> Result<Self, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handles and out pointer.
        let status = unsafe {
            abi::rive_rs_state_machine_instance_new(
                state_machine.as_raw(),
                artboard.as_raw(),
                &mut out,
            )
        };
        status_result(status)?;
        Ok(Self {
            raw: non_null(out)?,
        })
    }

    pub fn as_raw(&self) -> *mut abi::rive_rs_state_machine_instance {
        self.raw.as_ptr()
    }

    pub fn advance(&mut self, seconds: f32) -> Result<bool, Error> {
        let mut changed = false;
        // SAFETY: valid handle and out pointer.
        let status = unsafe {
            abi::rive_rs_state_machine_instance_advance(self.as_raw(), seconds, &mut changed)
        };
        status_result(status)?;
        Ok(changed)
    }

    pub fn advance_and_apply(&mut self, seconds: f32) -> Result<bool, Error> {
        let mut changed = false;
        // SAFETY: valid handle and out pointer.
        let status = unsafe {
            abi::rive_rs_state_machine_instance_advance_and_apply(
                self.as_raw(),
                seconds,
                &mut changed,
            )
        };
        status_result(status)?;
        Ok(changed)
    }

    pub fn input_count(&self) -> usize {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_state_machine_input_count(self.as_raw()) }
    }

    pub fn input(&mut self, index: usize) -> Result<SmiInput, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid handle and out pointer.
        let status = unsafe { abi::rive_rs_state_machine_input_at(self.as_raw(), index, &mut out) };
        status_result(status)?;
        Ok(SmiInput {
            raw: non_null(out)?,
        })
    }

    pub fn pointer_down(&mut self, point: Vec2, pointer_id: i32) -> Result<(), Error> {
        // SAFETY: valid handle and plain value arguments.
        let status = unsafe {
            abi::rive_rs_state_machine_instance_pointer_down(self.as_raw(), point, pointer_id)
        };
        status_result(status)
    }

    pub fn pointer_move(&mut self, point: Vec2, pointer_id: i32) -> Result<(), Error> {
        // SAFETY: valid handle and plain value arguments.
        let status = unsafe {
            abi::rive_rs_state_machine_instance_pointer_move(self.as_raw(), point, pointer_id)
        };
        status_result(status)
    }

    pub fn pointer_up(&mut self, point: Vec2, pointer_id: i32) -> Result<(), Error> {
        // SAFETY: valid handle and plain value arguments.
        let status = unsafe {
            abi::rive_rs_state_machine_instance_pointer_up(self.as_raw(), point, pointer_id)
        };
        status_result(status)
    }

    pub fn pointer_exit(&mut self, point: Vec2, pointer_id: i32) -> Result<(), Error> {
        // SAFETY: valid handle and plain value arguments.
        let status = unsafe {
            abi::rive_rs_state_machine_instance_pointer_exit(self.as_raw(), point, pointer_id)
        };
        status_result(status)
    }

    pub fn has_listeners(&self) -> bool {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_state_machine_instance_has_listeners(self.as_raw()) }
    }

    pub fn has_any_listener(&self) -> bool {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_state_machine_instance_has_any_listener(self.as_raw()) }
    }

    pub fn reported_event_count(&self) -> usize {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_state_machine_reported_event_count(self.as_raw()) }
    }

    pub fn reported_event_at(&self, index: usize) -> Result<ReportedEvent, Error> {
        let mut out_event = empty_event_info();
        let mut out_delay_seconds = 0.0f32;
        // SAFETY: valid handle and out pointers.
        let status = unsafe {
            abi::rive_rs_state_machine_reported_event_at(
                self.as_raw(),
                index,
                &mut out_event,
                &mut out_delay_seconds,
            )
        };
        status_result(status)?;

        let property_count = out_event.property_count;
        let mut properties = Vec::with_capacity(property_count);
        for property_index in 0..property_count {
            let mut out_property = empty_event_property_info();
            // SAFETY: valid handle and out pointer.
            let status = unsafe {
                abi::rive_rs_state_machine_reported_event_property_at(
                    self.as_raw(),
                    index,
                    property_index,
                    &mut out_property,
                )
            };
            status_result(status)?;
            properties.push(event_property_from_abi(out_property));
        }

        Ok(ReportedEvent {
            event: EventInfo {
                name: string_from_view(out_event.name),
                event_type: out_event.r#type,
                url: if out_event.has_url {
                    Some(string_from_view(out_event.url))
                } else {
                    None
                },
                target: if out_event.has_target {
                    Some(string_from_view(out_event.target))
                } else {
                    None
                },
                properties,
            },
            delay_seconds: out_delay_seconds,
        })
    }

    pub fn state_changed_count(&self) -> usize {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_state_machine_state_changed_count(self.as_raw()) }
    }

    pub fn state_changed_name_at(&self, index: usize) -> Result<String, Error> {
        let mut out_name = empty_str_view();
        // SAFETY: valid handle and out pointer.
        let status = unsafe {
            abi::rive_rs_state_machine_state_changed_name_at(self.as_raw(), index, &mut out_name)
        };
        status_result(status)?;
        Ok(string_from_view(out_name))
    }

    pub fn bind_view_model_instance(&mut self, instance: &ViewModelInstance) -> Result<(), Error> {
        // SAFETY: valid handles for call duration.
        let status = unsafe {
            abi::rive_rs_state_machine_instance_bind_view_model_instance(
                self.as_raw(),
                instance.as_raw(),
            )
        };
        status_result(status)
    }
}

impl Drop for StateMachineInstance {
    fn drop(&mut self) {
        // SAFETY: this is the owning instance handle.
        unsafe { abi::rive_rs_state_machine_instance_delete(self.as_raw()) };
    }
}

#[derive(Copy, Clone)]
pub struct SmiInput {
    raw: NonNull<abi::rive_rs_smi_input>,
}

impl SmiInput {
    pub fn as_raw(&self) -> *mut abi::rive_rs_smi_input {
        self.raw.as_ptr()
    }

    pub fn input_type(&self) -> SmiInputType {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_smi_input_type_of(self.as_raw()) }
    }

    pub fn name(&self) -> String {
        // SAFETY: valid handle.
        let name = unsafe { abi::rive_rs_smi_input_name(self.as_raw()) };
        string_from_view(name)
    }

    pub fn as_bool(&self) -> Result<SmiBool, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid input handle and out pointer.
        let status = unsafe { abi::rive_rs_smi_input_as_bool(self.as_raw(), &mut out) };
        status_result(status)?;
        Ok(SmiBool {
            raw: non_null(out)?,
        })
    }

    pub fn as_number(&self) -> Result<SmiNumber, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid input handle and out pointer.
        let status = unsafe { abi::rive_rs_smi_input_as_number(self.as_raw(), &mut out) };
        status_result(status)?;
        Ok(SmiNumber {
            raw: non_null(out)?,
        })
    }

    pub fn as_trigger(&self) -> Result<SmiTrigger, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: valid input handle and out pointer.
        let status = unsafe { abi::rive_rs_smi_input_as_trigger(self.as_raw(), &mut out) };
        status_result(status)?;
        Ok(SmiTrigger {
            raw: non_null(out)?,
        })
    }
}

#[derive(Copy, Clone)]
pub struct SmiBool {
    raw: NonNull<abi::rive_rs_smi_bool>,
}

impl SmiBool {
    pub fn get(&self) -> bool {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_smi_bool_get(self.raw.as_ptr()) }
    }

    pub fn set(&mut self, value: bool) {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_smi_bool_set(self.raw.as_ptr(), value) };
    }
}

#[derive(Copy, Clone)]
pub struct SmiNumber {
    raw: NonNull<abi::rive_rs_smi_number>,
}

impl SmiNumber {
    pub fn get(&self) -> f32 {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_smi_number_get(self.raw.as_ptr()) }
    }

    pub fn set(&mut self, value: f32) {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_smi_number_set(self.raw.as_ptr(), value) };
    }
}

#[derive(Copy, Clone)]
pub struct SmiTrigger {
    raw: NonNull<abi::rive_rs_smi_trigger>,
}

impl SmiTrigger {
    pub fn fire(&mut self) {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_smi_trigger_fire(self.raw.as_ptr()) };
    }
}

pub struct AudioSource {
    raw: NonNull<abi::rive_rs_audio_source>,
}

impl AudioSource {
    pub fn as_raw(&self) -> *mut abi::rive_rs_audio_source {
        self.raw.as_ptr()
    }
}

impl Drop for AudioSource {
    fn drop(&mut self) {
        // SAFETY: intrusive ref-count decrement on valid handle.
        unsafe { abi::rive_rs_audio_source_unref(self.as_raw()) };
    }
}

pub struct FileAsset {
    raw: NonNull<abi::rive_rs_file_asset>,
}

impl FileAsset {
    /// # Safety
    ///
    /// `raw` must point to a valid `rive_rs_file_asset` for the lifetime of this wrapper.
    pub unsafe fn from_raw(raw: *mut abi::rive_rs_file_asset) -> Result<Self, Error> {
        Ok(Self {
            raw: non_null(raw)?,
        })
    }

    pub fn as_raw(&self) -> *mut abi::rive_rs_file_asset {
        self.raw.as_ptr()
    }

    /// # Safety
    ///
    /// The pointer must be a valid runtime `FileAsset*`.
    pub unsafe fn from_pointer(pointer: usize) -> Result<Self, Error> {
        // SAFETY: caller guarantees pointer validity.
        let raw = unsafe { abi::rive_rs_ptr_to_file_asset(pointer) };
        // SAFETY: pointer is guaranteed valid by caller.
        unsafe { Self::from_raw(raw) }
    }

    /// # Safety
    ///
    /// The pointer must be a valid runtime `AudioAsset*`.
    pub unsafe fn from_audio_pointer(pointer: usize) -> Result<Self, Error> {
        // SAFETY: caller guarantees pointer validity.
        let raw = unsafe { abi::rive_rs_ptr_to_audio_asset(pointer) };
        // SAFETY: pointer is guaranteed valid by caller.
        unsafe { Self::from_raw(raw) }
    }

    /// # Safety
    ///
    /// The pointer must be a valid runtime `ImageAsset*`.
    pub unsafe fn from_image_pointer(pointer: usize) -> Result<Self, Error> {
        // SAFETY: caller guarantees pointer validity.
        let raw = unsafe { abi::rive_rs_ptr_to_image_asset(pointer) };
        // SAFETY: pointer is guaranteed valid by caller.
        unsafe { Self::from_raw(raw) }
    }

    /// # Safety
    ///
    /// The pointer must be a valid runtime `FontAsset*`.
    pub unsafe fn from_font_pointer(pointer: usize) -> Result<Self, Error> {
        // SAFETY: caller guarantees pointer validity.
        let raw = unsafe { abi::rive_rs_ptr_to_font_asset(pointer) };
        // SAFETY: pointer is guaranteed valid by caller.
        unsafe { Self::from_raw(raw) }
    }

    pub fn name(&self) -> String {
        // SAFETY: valid handle.
        let name = unsafe { abi::rive_rs_file_asset_name(self.as_raw()) };
        string_from_view(name)
    }

    pub fn cdn_base_url(&self) -> String {
        // SAFETY: valid handle.
        let value = unsafe { abi::rive_rs_file_asset_cdn_base_url(self.as_raw()) };
        string_from_view(value)
    }

    pub fn file_extension(&self) -> String {
        // SAFETY: valid handle.
        let value = unsafe { abi::rive_rs_file_asset_file_extension(self.as_raw()) };
        string_from_view(value)
    }

    pub fn unique_filename(&self) -> String {
        // SAFETY: valid handle.
        let value = unsafe { abi::rive_rs_file_asset_unique_filename(self.as_raw()) };
        string_from_view(value)
    }

    pub fn is_audio(&self) -> bool {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_file_asset_is_audio(self.as_raw()) }
    }

    pub fn is_image(&self) -> bool {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_file_asset_is_image(self.as_raw()) }
    }

    pub fn is_font(&self) -> bool {
        // SAFETY: valid handle.
        unsafe { abi::rive_rs_file_asset_is_font(self.as_raw()) }
    }

    pub fn cdn_uuid(&self) -> String {
        // SAFETY: valid handle.
        let value = unsafe { abi::rive_rs_file_asset_cdn_uuid(self.as_raw()) };
        string_from_view(value)
    }

    pub fn decode(&mut self, factory: &Factory, bytes: &[u8]) -> Result<(), Error> {
        // SAFETY: valid handles for call duration.
        let status = unsafe {
            abi::rive_rs_file_asset_decode(factory.as_raw(), self.as_raw(), bytes_view(bytes))
        };
        status_result(status)
    }

    pub fn set_audio_source(&mut self, audio: &AudioSource) -> Result<(), Error> {
        // SAFETY: valid handles for call duration.
        let status =
            unsafe { abi::rive_rs_audio_asset_set_audio_source(self.as_raw(), audio.as_raw()) };
        status_result(status)
    }

    pub fn set_font(&mut self, font: &Font) -> Result<(), Error> {
        // SAFETY: valid handles for call duration.
        let status = unsafe { abi::rive_rs_font_asset_set_font(self.as_raw(), font.as_raw()) };
        status_result(status)
    }

    pub fn set_render_image(&mut self, image: Option<&RenderImage>) -> Result<(), Error> {
        let raw = image.map_or(ptr::null_mut(), |value| value.as_raw());
        // SAFETY: valid handles for call duration.
        let status = unsafe { abi::rive_rs_image_asset_set_render_image(self.as_raw(), raw) };
        status_result(status)
    }
}

pub struct RenderImage {
    raw: NonNull<abi::rive_rs_render_image>,
}

impl RenderImage {
    pub fn decode_webgl2(bytes: &[u8]) -> Result<Self, Error> {
        let mut out = ptr::null_mut();
        // SAFETY: pointers are valid for call duration; out pointer is writable.
        let status = unsafe { abi::rive_rs_decode_webgl2_image(bytes_view(bytes), &mut out) };
        status_result(status)?;
        Ok(Self {
            raw: non_null(out)?,
        })
    }

    pub fn as_raw(&self) -> *mut abi::rive_rs_render_image {
        self.raw.as_ptr()
    }
}

impl Clone for RenderImage {
    fn clone(&self) -> Self {
        // SAFETY: intrusive ref-count increment on valid handle.
        unsafe { abi::rive_rs_render_image_ref(self.as_raw()) };
        Self { raw: self.raw }
    }
}

impl Drop for RenderImage {
    fn drop(&mut self) {
        // SAFETY: intrusive ref-count decrement on valid handle.
        unsafe { abi::rive_rs_render_image_unref(self.as_raw()) };
    }
}

pub struct Font {
    raw: NonNull<abi::rive_rs_font>,
}

impl Font {
    pub fn as_raw(&self) -> *mut abi::rive_rs_font {
        self.raw.as_ptr()
    }
}

impl Drop for Font {
    fn drop(&mut self) {
        // SAFETY: intrusive ref-count decrement on valid handle.
        unsafe { abi::rive_rs_font_unref(self.as_raw()) };
    }
}

pub fn mat2d_invert(matrix: &Mat2D) -> Option<Mat2D> {
    let det = matrix.xx * matrix.yy - matrix.xy * matrix.yx;
    if det == 0.0 {
        return None;
    }
    let inv_det = 1.0 / det;
    Some(Mat2D {
        xx: matrix.yy * inv_det,
        xy: -matrix.xy * inv_det,
        yx: -matrix.yx * inv_det,
        yy: matrix.xx * inv_det,
        tx: (matrix.yx * matrix.ty - matrix.yy * matrix.tx) * inv_det,
        ty: (matrix.xy * matrix.tx - matrix.xx * matrix.ty) * inv_det,
    })
}

pub fn mat2d_multiply(a: &Mat2D, b: &Mat2D) -> Mat2D {
    Mat2D {
        xx: a.xx * b.xx + a.yx * b.xy,
        xy: a.xy * b.xx + a.yy * b.xy,
        yx: a.xx * b.yx + a.yx * b.yy,
        yy: a.xy * b.yx + a.yy * b.yy,
        tx: a.xx * b.tx + a.yx * b.ty + a.tx,
        ty: a.xy * b.tx + a.yy * b.ty + a.ty,
    }
}

pub fn compute_alignment(
    fit: Fit,
    alignment: Alignment,
    source: &Aabb,
    destination: &Aabb,
    scale_factor: f32,
) -> Result<Mat2D, Error> {
    let mut out = Mat2D {
        xx: 0.0,
        xy: 0.0,
        yx: 0.0,
        yy: 0.0,
        tx: 0.0,
        ty: 0.0,
    };
    // SAFETY: all pointers are valid borrows for call duration.
    let status = unsafe {
        abi::rive_rs_compute_alignment(fit, alignment, source, destination, scale_factor, &mut out)
    };
    status_result(status)?;
    Ok(out)
}

pub fn map_xy(matrix: &Mat2D, point: Vec2) -> Result<Vec2, Error> {
    let mut out = Vec2 { x: 0.0, y: 0.0 };
    // SAFETY: all pointers are valid borrows for call duration.
    let status = unsafe { abi::rive_rs_map_xy(matrix, point, &mut out) };
    status_result(status)?;
    Ok(out)
}
