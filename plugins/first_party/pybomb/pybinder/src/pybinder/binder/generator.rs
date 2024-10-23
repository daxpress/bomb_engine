use header_tool::Namespace;

// we will stick all the headers in the "generated" directory, we don't need to separate them
const _GENERATED_DIR: &'static str = "generated";
const ENGINE_SPLIT_STR: &'static str = "bomb_engine/engine/";

pub fn generate_header(namespace: &Namespace) {
    // here we finally look into the deserialized data in order to create a header that contains the bound data

    let include = get_include(&namespace.name);

    println!("{}", include);
}

fn get_include(filename: &String) -> String {
    // split the absolute path to the relative path (the one that we use to include the headers in C++)
    let header_relative = filename.split(ENGINE_SPLIT_STR).collect::<Vec<&str>>()[1];

    format!("#include \"{}\"\n", header_relative)
}

#[cfg(test)]
mod tests {
    use std::path::Path;
    use header_tool::Namespace;
    use super::get_include;

    #[test]
    fn get_iclude_test() {
        let path = Path::new("src/sample_gen/json/mod1/test.json");

        let json = std::fs::read_to_string(path).unwrap();
        let namespace: Namespace = serde_json::from_str(&json).unwrap();

        let include = get_include(&namespace.name);
        assert_eq!("#include \"mod1/test.h\"\n", include);
    }
}
