use std::fs;
use std::path::Path;

use super::header_checker::HeaderCacheOp;
use super::header_parser::HeaderParser;

const GENERATED_DIR: &'static str = "generated";

pub struct HeaderGenerator {
    operations: Vec<HeaderCacheOp>,
}

impl HeaderGenerator {
    pub fn new(operations: Vec<HeaderCacheOp>) -> Self {
        let path = Path::new(GENERATED_DIR);
        if !path.exists() {
            fs::create_dir(path).unwrap();
        }
        HeaderGenerator { operations }
    }

    pub fn generate_headers(&self) {
        
    }
}
