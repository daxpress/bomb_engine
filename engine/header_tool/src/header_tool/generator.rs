use rayon::prelude::*;
use std::fs::{self, File};
use std::path::Path;

use crate::Namespace;

const GENERATED_HEADERS_DIR: &'static str = "generated/headers";
const GENERATED_JSON_DIR: &'static str = "generated/json";
const SUPPORTED_EXTENSIONS: [&'static str; 3] = [".h", ".hpp", ".ixx"];

pub struct Generator {
    parsed_headers: Vec<Namespace>,
}

impl Generator {
    pub fn new(parsed_headers: Vec<Namespace>) -> Self {
        let path = Path::new(GENERATED_HEADERS_DIR);
        if !path.exists() {
            fs::create_dir_all(path).unwrap();
        }
        let path = Path::new(GENERATED_JSON_DIR);
        if !path.exists() {
            fs::create_dir_all(path).unwrap();
        }
        Generator { parsed_headers }
    }

    pub fn generate_info(&self) {
        println!("Generating reflection data...");
        self.parsed_headers.par_iter().for_each(|parsed_unit|{
            Self::generate_header(parsed_unit);
            Self::generate_json(parsed_unit);
        });
    }

    pub fn generated_header_filepath(header: &str) -> String {
        let filename = Path::new(header).file_name().unwrap().to_str().unwrap();
        let strip = SUPPORTED_EXTENSIONS
            .iter()
            .find_map(|extension| filename.strip_suffix(extension))
            .unwrap();
        format!("{}/{}.generated.h", GENERATED_HEADERS_DIR, strip)
    }

    pub fn generated_json_filepath(header: &str) -> String {
        let filename = Path::new(header).file_name().unwrap().to_str().unwrap();
        let strip = SUPPORTED_EXTENSIONS
            .iter()
            .find_map(|extension| filename.strip_suffix(extension))
            .unwrap();
        format!("{}/{}.json", GENERATED_JSON_DIR, strip)
    }

    fn generate_header(parsed_unit: &Namespace){
        let file_path = Self::generated_header_filepath(&parsed_unit.name);
        let file_path = Path::new(&file_path);
        let _file = File::create(file_path).unwrap();
        // TODO:
        // create the headers with the serialized data - still thinking about the implementation
    }

    fn generate_json(parsed_unit: &Namespace){
        let file_path = Self::generated_json_filepath(&parsed_unit.name);
        let file_path = Path::new(&file_path);
        let file = File::create(file_path).unwrap();
        serde_json::to_writer_pretty(file, parsed_unit).unwrap();
    }
}

#[cfg(test)]
mod tests {
    use super::Generator;

    #[test]
    fn generated_header_name_ok() {
        let name = "W:/C++/bomb_engine/engine/utilities/file_helper.h";
        let gen = Generator::generated_header_filepath(&name);
        assert_eq!("generated/headers/file_helper.generated.h", gen)
    }
    #[test]
    fn generated_json_name_ok() {
        let name = "W:/C++/bomb_engine/engine/utilities/file_helper.h";
        let gen = Generator::generated_json_filepath(&name);
        assert_eq!("generated/json/file_helper.json", gen)
    }
}
