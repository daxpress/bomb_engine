mod cpp_reflection;

use rayon::prelude::*;
use std::fs::{self, File};
use std::path::Path;

use crate::Namespace;

pub const GENERATED_HEADERS_DIR: &'static str = "generated/headers";
pub const GENERATED_JSON_DIR: &'static str = "generated/json";
pub const SUPPORTED_EXTENSIONS: [&'static str; 3] = [".h", ".hpp", ".ixx"];

pub struct Generator<'a> {
    parsed_headers: Vec<(Namespace, &'a str)>,
}

impl<'a> Generator <'a> {
    pub fn new(parsed_headers: Vec<(Namespace, &'a str)>) -> Self {
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
        self.parsed_headers
            .par_iter()
            .for_each(|(parsed_unit, module)| {
                // create the module directories in the generated dirs if not already there
                let path = format!("{}/{}", GENERATED_HEADERS_DIR, module);
                let path = Path::new(&path);
                if !path.exists() {
                    fs::create_dir_all(path).unwrap();
                }
                let path = format!("{}/{}", GENERATED_JSON_DIR, module);
                let path = Path::new(&path);
                if !path.exists() {
                    fs::create_dir_all(path).unwrap();
                }
                // now generate
                Self::generate_header(parsed_unit, &module);
                Self::generate_json(parsed_unit, &module);
            });
    }

    pub fn generated_header_filepath(header: &str, module: &str) -> String {
        Self::get_generated_path(header, module, &GENERATED_HEADERS_DIR, "generated.h")
    }

    pub fn generated_json_filepath(header: &str, module: &str) -> String {
        Self::get_generated_path(header, module, &GENERATED_JSON_DIR, "json")
    }

    fn get_generated_path(header: &str, module: &str, dir: &str, extension: &str) -> String {
        let filename = Path::new(header).file_name().unwrap().to_str().unwrap();
        let strip = SUPPORTED_EXTENSIONS
            .iter()
            .find_map(|extension| filename.strip_suffix(extension))
            .unwrap();
            format!("{}/{}/{}.{}", dir, module, strip, extension)
    }

    fn generate_header(parsed_unit: &Namespace, module: &str) {
        let file_path = Self::generated_header_filepath(&parsed_unit.name, module);
        let file_path = Path::new(&file_path);
        let _file = File::create(file_path).unwrap();
        // create the headers with the serialized data
        // TODO: use refl-cpp library to reflect code!
    }

    fn generate_json(parsed_unit: &Namespace, module: &str) {
        let file_path = Self::generated_json_filepath(&parsed_unit.name, module);
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
        let gen = Generator::generated_header_filepath(&name, "somemod");
        assert_eq!("generated/headers/somemod/file_helper.generated.h", gen)
    }
    #[test]
    fn generated_json_name_ok() {
        let name = "W:/C++/bomb_engine/engine/utilities/file_helper.h";
        let gen = Generator::generated_json_filepath(&name, "somemod");
        assert_eq!("generated/json/somemod/file_helper.json", gen)
    }
}
