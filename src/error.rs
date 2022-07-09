#[derive(thiserror::Error, Debug)]
pub enum BakeError {
    #[error("{0}")]
    IO(String),
    #[error("{0}")]
    Parser(String),
    #[error("{0}")]
    Target(String),
    #[error("{0}")]
    Network(String),
}

impl From<curl::Error> for BakeError {
    fn from(src: curl::Error) -> Self {
        BakeError::Network(src.extra_description().unwrap_or("").to_string())
    }
}
