#![doc = include_str!("../README.md")]

pub mod abi;
mod error;
mod runtime;

pub use error::Error;
pub use runtime::Aabb;
pub use runtime::Alignment;
pub use runtime::Artboard;
pub use runtime::AudioSource;
pub use runtime::BindableArtboard;
pub use runtime::BoneHandle;
pub use runtime::DataEnumInfo;
pub use runtime::DataType;
pub use runtime::EventInfo;
pub use runtime::EventProperty;
pub use runtime::EventPropertyType;
pub use runtime::EventPropertyValue;
pub use runtime::Factory;
pub use runtime::File;
pub use runtime::FileAsset;
pub use runtime::FileAssetLoaderCallbacks;
pub use runtime::Fit;
pub use runtime::FlattenedPath;
pub use runtime::Font;
pub use runtime::LinearAnimation;
pub use runtime::LinearAnimationInstance;
pub use runtime::Mat2D;
pub use runtime::NodeHandle;
pub use runtime::PropertyInfo;
pub use runtime::ReportedEvent;
pub use runtime::RootBoneHandle;
pub use runtime::SmiBool;
pub use runtime::SmiInput;
pub use runtime::SmiInputType;
pub use runtime::SmiNumber;
pub use runtime::SmiTrigger;
pub use runtime::StateMachine;
pub use runtime::StateMachineInstance;
pub use runtime::TextValueRunHandle;
pub use runtime::TransformComponentHandle;
pub use runtime::Vec2;
pub use runtime::ViewModel;
pub use runtime::ViewModelInstance;
pub use runtime::WebGl2Renderer;
pub use runtime::WebGpuRenderer;
pub use runtime::abi_version;
pub use runtime::compute_alignment;
pub use runtime::map_xy;
pub use runtime::mat2d_invert;
pub use runtime::mat2d_multiply;

pub type Status = abi::rive_rs_status;

pub const ABI_VERSION: u32 = 1;

#[inline]
pub fn status_ok(status: Status) -> bool {
    matches!(status, Status::RIVE_RS_STATUS_OK)
}
