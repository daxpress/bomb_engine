use anyhow::{self, Ok};
use checker::HeaderChecker;
use generator::Generator;

mod checker;
mod generator;
mod parser;
mod remover;

pub use parser::enitities_representations;
use parser::HeaderParser;

pub fn run(headers: &[String]) -> Result<(), anyhow::Error> {
    println!("Running Header Tool...");
    let mut checker = HeaderChecker::new();
    checker.run_check(headers);
    // filter the headers that need generation and the ones to remove
    let to_generate = checker.headers_to_generate();
    let to_delete = checker.headers_to_delete();
    // first remove as it also contains files to update (remove+add)
    remover::delete(&to_delete);
    // avoid parsing stuff that didn't change from last run.
    let parser = HeaderParser::new();
    let result = parser.parse_header_collection(&to_generate);
    let generator = Generator::new(result);
    generator.generate_info();
    println!("All Done.");
    Ok(())
}
