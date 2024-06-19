use anyhow::{self, Ok};
use checker::HeaderChecker;
use generator::Generator;

mod checker;
mod generator;
mod parser;
mod remover;

pub use parser::enitities_representations;
use parser::HeaderParser;

pub fn run(modules: Vec<Module>) -> Result<(), anyhow::Error> {
    println!("Running Header Tool...");
    let mut checker = HeaderChecker::new();
    for module in modules.iter() {
        checker.check(&module.module_headers, &module.module_name);
    }
    checker.close_checks();

    // remove stuff
    // TODO: simplify the removal by caching the module as well
    let to_delete = checker.headers_to_delete();
    let modules: Vec<&str> = modules.iter().map(|module| module.module_name.as_ref()).collect();
    remover::delete_generated(&to_delete, &modules);

    // parse data
    let to_generate = checker.headers_to_generate();
    let parser = HeaderParser::new();
    let result = parser.parse_header_collection(&to_generate); // needs the mod info
    // generate data
    let generator = Generator::new(result);
    generator.generate_info();

    println!("All Done.");
    Ok(())
}

#[derive(Default, Debug)]
pub struct Module {
    pub module_name: String,
    pub module_headers: Vec<String>,
}

impl Module {
    pub fn is_empty(&self) -> bool {
        self.module_headers.is_empty()
    }
}
