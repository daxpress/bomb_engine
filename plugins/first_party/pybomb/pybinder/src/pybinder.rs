mod binder;

use crate::pybinder::binder::*;
use header_tool::header_tool::Module;
use path_slash::PathExt;
use std::ffi::OsStr;
use std::path::Path;

pub fn run(gen_dir: &Path) -> anyhow::Result<()> {
    println!("Running Pybinder...");

    let module = build_module(gen_dir);
    bind_module(&module);

    println!("Bindings created.");
    Ok(())
}

fn build_module(gen_dir: &Path) -> Module {
    // use json generated files to detect changes for the cache

    let mut module = Module {
        module_name: "pybomb".to_string(),
        module_headers: Vec::new(),
    };

    // path to the json directory
    // inside it there are folders corresponding to the engine modules, inside them the json files to get
    let json_path = gen_dir.join("json");

    module.module_headers = json_path
        .read_dir()
        .unwrap()
        .filter_map(|path| {
            // skip not directories
            let path = path.unwrap().path();
            if !path.is_dir() {
                return None;
            }
            // grab files in subdirs
            let files = path
                .read_dir()
                .unwrap()
                .filter_map(|path| {
                    let path = path.unwrap().path();
                    if path.is_file() && path.extension() == Some(OsStr::new("json")) {
                        // return path as string with slashes as separators to work with UNIX and WINDOWS
                        return Some(path.to_slash().unwrap().to_string());
                    };
                    None
                })
                .collect::<Vec<_>>();
            Some(files)
        })
        .flatten()
        .collect::<Vec<_>>(); // flatten to a single vector

    module
}

#[cfg(test)]
mod tests {
    use crate::pybinder::build_module;
    use std::path::Path;

    #[test]
    fn build_module_test() {
        let module = build_module(Path::new("src/sample_gen"));
        assert_eq!(module.module_headers.len(), 3);
        assert_eq!(
            module.module_headers,
            [
                "src/sample_gen/json/mod1/sample.json",
                "src/sample_gen/json/mod1/test.json",
                "src/sample_gen/json/mod2/empty.json",
            ]
        );
    }
}
