pub struct Variable {
    pub key: String,
    pub value: String,
}

impl Variable {
    pub fn new(key: String, value: String) -> Self {
        Self { key, value }
    }
}
