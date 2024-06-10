use rayon::prelude::*;
use std::fs;
use std::path::Path;

use super::checker::HeaderCacheOp;
use super::parser::HeaderParser;

const GENERATED_DIR: &'static str = "generated";
const SUPPORTED_EXTENSIONS: [&'static str; 3] = [".h", ".hpp", ".ixx"];

pub struct Generator {
    operations: Vec<HeaderCacheOp>,
    parser: HeaderParser,
}

impl Generator {
    pub fn new(operations: Vec<HeaderCacheOp>) -> Self {
        let path = Path::new(GENERATED_DIR);
        if !path.exists() {
            fs::create_dir(path).unwrap();
        }
        Generator {
            operations,
            parser: HeaderParser::new(),
        }
    }

    pub fn generate_info(&self) {
        // work on copy, we want to keep the original format to perform other stuff later
        self.operations
            .to_vec()
            .iter()
            .for_each(|operation| match operation {
                HeaderCacheOp::Skip => (),
                HeaderCacheOp::Add(header, _) => self.add_header(&header),
                HeaderCacheOp::Update(header, _) => self.update_header(&header),
                HeaderCacheOp::Remove(header) => self.remove_header(&header),
            });
    }

    fn remove_header(&self, header: &String) {
        let gen_name = Self::generated_name(header);
        let path = Path::new(&gen_name);
        if path.exists() {
            fs::remove_file(path).unwrap();
        }
    }

    fn add_header(&self, _header: &String) {}

    fn update_header(&self, _header: &String) {}

    fn generated_name(header: &String) -> String {
        let strip = SUPPORTED_EXTENSIONS
            .iter()
            .find_map(|extension| header.strip_suffix(extension))
            .unwrap();
        format!("{}/{}.generated.h", GENERATED_DIR, strip)
    }
}
