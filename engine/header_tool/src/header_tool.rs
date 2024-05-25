use anyhow::{self, Ok};

mod header_checker;

pub struct HeaderTool {

}

impl HeaderTool {
    pub fn new() -> Self {
        HeaderTool{}
    }

    pub fn run(&self, headers: &[String]) -> Result<(), anyhow::Error>{
    println!("Running Header Tool...");
    let mut checker = header_checker::HeaderChecker::new();
    let result = checker.run_check(headers);
    println!("{result:#?}");
    println!("All Done.");
    Ok(())
    }
}
