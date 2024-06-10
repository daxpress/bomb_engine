use std::{fs, path::Path};

use rayon::iter::{IntoParallelRefIterator, ParallelIterator};

use super::generator::Generator;


pub fn delete(headers: &[&str]) {
    println!("Removing outdated information...");
    headers.par_iter().for_each(|header| remove_generated_header(header));
}

fn remove_generated_header(header: &str) {
    let gen_name = Generator::generated_header_filepath(header);
    let path = Path::new(&gen_name);
    if path.exists() {
        fs::remove_file(path).unwrap();
    }
    let gen_name = Generator::generated_json_filepath(header);
    let path = Path::new(&gen_name);
    if path.exists() {
        fs::remove_file(path).unwrap();
    }
}