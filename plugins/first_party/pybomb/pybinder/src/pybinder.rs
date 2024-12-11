mod binder;

use crate::pybinder::binder::*;
use header_tool::header_tool::Module;
use path_slash::PathExt;
use std::ffi::OsStr;
use std::path::Path;

pub fn run(gen_dir: &Path) -> anyhow::Result<()> {
    println!("Running Pybinder...");

    let module = build_modules(gen_dir);
    bind_modules(&module);

    println!("Bindings created.");
    Ok(())
}

fn build_modules(gen_dir: &Path) -> Vec<Module> {
    // path to the json directory
    // inside it there are folders corresponding to the engine modules, inside them the json files to get
    let json_path = gen_dir.join("json");

    let modules = json_path.read_dir().unwrap().filter_map(|path| {
        // skip not directories
        let path = path.unwrap().path();
        if !path.is_dir() {
            return None;
        };

        let mut module = Module::default();
        module.module_name = path.file_name().unwrap().to_str().unwrap().to_string();

        // grab files
        module.module_headers = path
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

        Some(module)

    }).collect::<Vec<_>>();

    modules
}

#[cfg(test)]
mod tests {
    use crate::pybinder::build_modules;
    use std::path::Path;

    #[test]
    fn build_module_test() {
        let module = build_modules(Path::new("src/sample_gen"));
        assert_eq!(module[0].module_name, "mod1");
        assert_eq!(module[0].module_headers.len(), 2);
        assert_eq!(
            module[0].module_headers,
            [
                "src/sample_gen/json/mod1/sample.json",
                "src/sample_gen/json/mod1/test.json",
            ]
        );
        assert_eq!(module[1].module_name, "mod2");
        assert_eq!(module[1].module_headers.len(), 1);
        assert_eq!(
            module[1].module_headers,
            [
                "src/sample_gen/json/mod2/empty.json",
            ]
        );
    }
}
