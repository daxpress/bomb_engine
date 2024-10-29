mod generator;
mod pybindify;

use crate::pybinder::binder::generator::{generate_header, generate_module_decls};
use header_tool::header_tool::*;
use header_tool::Namespace;
use rayon::iter::IntoParallelIterator;
use rayon::iter::ParallelIterator;
use std::fs::remove_file;
use std::path::Path;

const CACHE_NAME: &'static str = "pybinder_cache.cache";

pub fn bind_module(module: &Module) {
    let mut checker = checker::HeaderChecker::new(CACHE_NAME);
    checker.check(&module.module_headers, &module.module_name);
    checker.close_checks();

    // remove outdated stuff
    checker
        .headers_to_delete()
        .into_par_iter()
        .for_each(|file| remove_file(Path::new(file)).unwrap());

    // make the generated directory if not present
    generator::make_gen_dir();

    // get the files to generate from (filter out the module name, it is not helpful here)
    let to_generate = checker
        .headers_to_generate()
        .iter()
        .map(|result| result.0)
        .collect::<Vec<_>>();
    // get the skipped files
    let to_skip = checker.headers_to_skip();

    // now generate the main binding file (the module declaration + inits) based on the allowed modules
    if !checker.headers_to_delete().is_empty() || !to_generate.is_empty() {
        let concatenation = [to_generate.clone(), to_skip].concat();

        let names = concatenation
            .iter()
            .map(|name| {
                Path::new(name)
                    .file_name()
                    .unwrap()
                    .to_str()
                    .unwrap()
                    .strip_suffix(".json")
                    .unwrap()
            })
            .collect::<Vec<_>>();

        generate_module_decls(names.as_slice());
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
