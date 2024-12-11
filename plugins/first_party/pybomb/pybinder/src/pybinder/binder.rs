mod generator;
mod pybindify;

use crate::pybinder::binder::generator::{generate_header, generate_module_decls};
use header_tool::header_tool::*;
use header_tool::{CheckerResult, Namespace};
use rayon::iter::IntoParallelIterator;
use rayon::iter::ParallelIterator;
use std::collections::HashMap;
use std::fs::remove_file;
use std::path::Path;

const CACHE_NAME: &'static str = "pybinder_cache.cache";

pub fn bind_modules(modules: &Vec<Module>) {
    let mut checker = checker::HeaderChecker::new(CACHE_NAME);
    for module in modules.iter() {
        checker.check(&module.module_headers, &module.module_name);
    }
    checker.close_checks();

    // remove outdated stuff
    checker
        .headers_to_delete()
        .into_par_iter()
        .for_each(|file| remove_file(Path::new(&file.header)).unwrap());

    // make the generated directory if not present
    generator::make_gen_dir();

    // get the files to generate from (filter out the module name, it is not helpful here)
    let to_generate = checker
        .headers_to_generate()
        .iter()
        .map(|result| result.header)
        .collect::<Vec<_>>();

    // now generate the main binding file (the module declaration + inits) based on the allowed modules
    if !checker.headers_to_delete().is_empty() || !to_generate.is_empty() {
        let module_map = [checker.headers_to_generate(), checker.headers_to_skip()].concat();
        generate_module_decls(&get_module_hashmap(module_map));
    }

    // collect the deserialized information into the Namespace class to have a representation to help bind everything based on that
    let namespaces = to_generate
        .into_par_iter()
        .map(|file| deserialize_file(Path::new(file)))
        .collect::<Vec<_>>();

    // generate the files!
    namespaces
        .into_par_iter()
        .for_each(|namespace| generate_header(&namespace));
}

fn get_module_hashmap(results: Vec<CheckerResult>) -> HashMap<&str, Vec<&str>> {
    let mut map: HashMap<&str, Vec<&str>> = HashMap::new();
    for result in results.iter() {
        if map.contains_key(result.module) {
            let header = Path::new(result.header)
                .file_name()
                .unwrap()
                .to_str()
                .unwrap()
                .strip_suffix(".json")
                .unwrap();
            map.get_mut(result.module).unwrap().push(header);
        } else {
            map.insert(result.module, Vec::new());
        }
    }
    map
}

fn deserialize_file(path: &Path) -> Namespace {
    let json = std::fs::read_to_string(path).unwrap();
    serde_json::from_str(&json).unwrap()
}

#[cfg(test)]
mod tests {
    use crate::pybinder::binder::deserialize_file;
    use header_tool::Namespace;
    use std::path::Path;

    #[test]
    fn deserialize_empty_file_test() {
        let path = Path::new("src/sample_gen/json/mod2/empty.json");

        let result = deserialize_file(path);

        assert_eq!(Namespace::default(), result);
    }
}
