use std::{fs, path::Path};

use super::Generator;
use rayon::iter::{IntoParallelRefIterator, ParallelIterator};

pub fn delete_generated(headers: &[&str], modules: &[&str]) {
    println!("Removing outdated information...");
    headers
        .par_iter()
        .for_each(|header| {
            if let Some(dir) = find_generated(header, modules) {
                remove_generated_header(header, dir);   
            }});
}

fn remove_generated_header(header: &str, module: &str) {
    let gen_name = Generator::generated_header_filepath(header, module);
    let path = Path::new(&gen_name);
    if path.exists() {
        fs::remove_file(path).unwrap();
    }
    let gen_name = Generator::generated_json_filepath(header, module);
    let path = Path::new(&gen_name);
    if path.exists() {
        fs::remove_file(path).unwrap();
    }
}

fn find_generated<'a>(header: &str, dirs: &'a [&str]) -> Option<&'a str> {
    for dir in dirs {
        let hpath = Generator::generated_header_filepath(header, dir);
        let jpath = Generator::generated_json_filepath(header, dir);
        let hfile = Path::new(&hpath);
        let jsonfile = Path::new(&jpath);

        if hfile.exists() || jsonfile.exists() {
            return Some(dir);
        }
    }
    None
}
