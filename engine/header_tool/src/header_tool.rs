use anyhow::{self, Ok};
use header_checker::HeaderChecker;
use header_generator::HeaderGenerator;

mod header_checker;
mod header_parser;
mod header_generator;
pub struct HeaderTool {

}

impl HeaderTool {
    pub fn new() -> Self {
        HeaderTool{}
    }

    pub fn run(&self, headers: &[String]) -> Result<(), anyhow::Error>{
    println!("Running Header Tool...");
    let mut checker = HeaderChecker::new();
    let operations = checker.run_check(headers);
    let generator = HeaderGenerator::new(operations);
    generator.generate_headers();
    println!("All Done.");
    Ok(())
    }
}
