use super::pybindify::*;
use header_tool::Namespace;
use std::fs;
use std::io::Write;
use std::path::Path;

// we will stick all the headers in the "generated" directory, we don't need to separate them
static GENERATED_DIR: &str = "generated";
static ENGINE_SPLIT_STR: &str = "bomb_engine/engine/";
static MODULE_INITS_NAME: &str = "module_inits";

static MODULE_DECL: &str = "\nPYBIND11_MODULE(bomb_engine, m, py::mod_gil_not_used()) {\n";

static EXTENSION: &str = ".cpp";

pub fn make_gen_dir() {
    let path = Path::new(GENERATED_DIR);
    if !path.exists() {
        fs::create_dir_all(path).unwrap();
    }
}

pub fn generate_module_decls(names: &[&str]) {
    let mut file_content = "".to_string();
    // include pybind11
    file_content.push_str(BindingsBuilder::PYBIND_INCLUDE);
    // declare init methods based on what we have
    for name in names {
        file_content.push_str(&format!("void init_{}(py::module &m);\n", name));
    }
    // init for the hand built bindings
    file_content.push_str("void init_pybomb(py::module &m);\n");

    // declare the actual python module
    file_content.push_str(MODULE_DECL);
    for name in names {
        // call the init methods
        file_content.push_str(&format!("\tinit_{}(m);\n", name));
    }
    // also add a function call to init some hand built bindings
    file_content.push_str("\tinit_pybomb(m);\n");
    // end module declaration
    file_content.push_str("}");

    // create the file and fill it
    let path = format!("{}/{}{}", GENERATED_DIR, MODULE_INITS_NAME, EXTENSION);
    fs::File::create(Path::new(&path))
        .unwrap()
        .write_all(file_content.as_ref())
        .unwrap();
}

pub fn generate_header(namespace: &Namespace) {
    // here we finally look into the deserialized data in order to create a header that contains the bound data
    let builder = BindingsBuilder::new(namespace);
    // let's kick things off by including the header we want to bind
    builder.build();
}

struct BindingsBuilder<'a> {
    namespace: &'a Namespace,
    filename: &'a str,
}

impl<'a> BindingsBuilder<'a> {
    // Here some constant strings that are used
    const PYBIND_INCLUDE: &'static str =
        "#include <pybind11/pybind11.h>\n\nnamespace py = pybind11;\n\n";
    fn new(namespace: &'a Namespace) -> BindingsBuilder<'a> {
        BindingsBuilder {
            namespace,
            // make a path from the name to extract the original filename
            filename: Path::new(&namespace.name)
                .file_name()
                .unwrap()
                .to_str()
                .unwrap()
                .strip_suffix(".h")
                .unwrap(),
        }
    }

    fn include(&self, filepath: &String) -> String {
        // split the absolute path to the relative path (the one that we use to include the headers in C++)
        let header_relative = filepath.split(ENGINE_SPLIT_STR).collect::<Vec<&str>>()[1];

        format!("\n#include \"{}\"\n", header_relative)
    }

    // builds the file content and writes it to the correct file, then returns the content (mainly for tests)
    pub fn build(self) -> String {
        let mut file_content = "".to_string();

        // first the header include and the pybind11 header
        file_content.push_str(&self.include(&self.namespace.name));
        file_content.push_str(Self::PYBIND_INCLUDE);
        // define the init function where we will be building the module (NOTE: SCOPE IS OPEN!)
        file_content.push_str(&format!("void init_{}(py::module &m) {{\n", self.filename));

        // kickoff the production of the header from the namespace
        let root = self.namespace.to_cpp_binding();
        // this should be all of it...
        file_content.push_str(&root);
        file_content.push_str("}\n");

        // create the file and fill it
        let path = format!("{}/py_{}{}", GENERATED_DIR, self.filename, EXTENSION);
        fs::File::create(Path::new(&path))
            .unwrap()
            .write_all(file_content.as_ref())
            .unwrap();

        file_content
    }
}

#[cfg(test)]
mod tests {
    use super::{make_gen_dir, BindingsBuilder, GENERATED_DIR};
    use header_tool::Namespace;
    use std::path::Path;

    static TEST_FILE: &str = "src/sample_gen/json/mod1/test.json";

    #[test]
    fn gen_dir_test() {
        let path = Path::new(GENERATED_DIR);
        if path.exists() {
            std::fs::remove_dir_all(path).unwrap();
        }
        assert!(!path.exists());

        make_gen_dir();
        assert!(path.exists());
    }

    #[test]
    fn get_iclude_test() {
        let path = Path::new(TEST_FILE);

        let json = std::fs::read_to_string(path).unwrap();
        let namespace: Namespace = serde_json::from_str(&json).unwrap();

        let include = BindingsBuilder::new(&namespace).include(&namespace.name);

        assert_eq!("\n#include \"mod1/test.h\"\n", include);
    }

    #[test]
    fn test_build() {
        let path = Path::new(TEST_FILE);

        let json = std::fs::read_to_string(path).unwrap();
        let namespace: Namespace = serde_json::from_str(&json).unwrap();

        let header = BindingsBuilder::new(&namespace).build();

        let expected = r#"
#include "mod1/test.h"
#include <pybind11/pybind11.h>

namespace py = pybind11;

void init_test(py::module &m) {
}
"#;

        assert_eq!(expected, header);
    }
}
