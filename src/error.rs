use crate::Status;

#[derive(Debug, Copy, Clone, Eq, PartialEq)]
pub struct Error {
    status: Status,
}

impl Error {
    pub const fn from_status(status: Status) -> Self {
        Self { status }
    }

    pub const fn status(self) -> Status {
        self.status
    }

    pub const fn null_handle() -> Self {
        Self {
            status: Status::RIVE_RS_STATUS_NULL,
        }
    }
}

impl core::fmt::Display for Error {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        let message = match self.status {
            Status::RIVE_RS_STATUS_OK => "ok",
            Status::RIVE_RS_STATUS_NULL => "null handle",
            Status::RIVE_RS_STATUS_INVALID_ARGUMENT => "invalid argument",
            Status::RIVE_RS_STATUS_NOT_FOUND => "not found",
            Status::RIVE_RS_STATUS_OUT_OF_RANGE => "out of range",
            Status::RIVE_RS_STATUS_UNSUPPORTED => "unsupported",
            Status::RIVE_RS_STATUS_DECODE_ERROR => "decode error",
            Status::RIVE_RS_STATUS_RUNTIME_ERROR => "runtime error",
        };
        write!(f, "{message} ({:?})", self.status)
    }
}

impl std::error::Error for Error {}
