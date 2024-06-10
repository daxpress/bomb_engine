use anyhow::{self, Ok};
use checker::HeaderChecker;
use generator::Generator;

mod checker;
mod parser;
mod generator;
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
    let generator = Generator::new(operations);
    generator.generate_info();
    println!("All Done.");
    Ok(())
    }
}
