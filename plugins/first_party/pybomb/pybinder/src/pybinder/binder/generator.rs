use header_tool::Namespace;
use std::fs;
use std::fs::File;
use std::io::Write;
use std::path::Path;

// we will stick all the headers in the "generated" directory, we don't need to separate them
const GENERATED_DIR: &'static str = "generated";
const ENGINE_SPLIT_STR: &'static str = "bomb_engine/engine/";

pub fn make_gen_dir() {
    let path = Path::new(GENERATED_DIR);
    if !path.exists() {
        fs::create_dir_all(path).unwrap();
    }
}

pub fn generate_header(namespace: &Namespace) {
    // here we finally look into the deserialized data in order to create a header that contains the bound data
    let mut pieces = Vec::new();

    // let's kick things off by including the header we want to bind
    pieces.push(get_include(&namespace.name));

    build_generated_file(&namespace.name, pieces);
}

fn get_include(filename: &String) -> String {
    // split the absolute path to the relative path (the one that we use to include the headers in C++)
    let header_relative = filename.split(ENGINE_SPLIT_STR).collect::<Vec<&str>>()[1];

    format!("#include \"{}\"\n", header_relative)
}

fn build_generated_file(name: &str, parts: Vec<String>) {
    // put all the pieces together
    let file_content = parts.concat();

    // make a path from the name to extract the original filename
    let filename = Path::new(name).file_name().unwrap().to_str().unwrap();
    // create the file
    let path = format!("{}/py_{}", GENERATED_DIR, filename);
    File::create(Path::new(&path))
        .unwrap()
        .write_all(file_content.as_ref())
        .unwrap();
}

#[cfg(test)]
mod tests {
    use super::get_include;
    use header_tool::Namespace;
    use std::path::Path;

    #[test]
    fn get_iclude_test() {
        let path = Path::new("src/sample_gen/json/mod1/test.json");

        let json = std::fs::read_to_string(path).unwrap();
        let namespace: Namespace = serde_json::from_str(&json).unwrap();

        let include = get_include(&namespace.name);
        assert_eq!("#include \"mod1/test.h\"\n", include);
    }
}
