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
    // first print the list of headers
    println!("Headers:\n{:#?}", headers);
    let mut checker = header_checker::HeaderChecker::new();
    checker.run_check(headers);

    println!("All Done.");
    Ok(())
    }
}
