use std::{env::args, fmt::Display, error::Error};
use header_tool::header_tool;

fn main() -> Result<(), anyhow::Error> {
    let args = args();
    if args.len() <= 1 {
        let error = anyhow::anyhow!(ArgsError);
        return Err(error);
    }
    let args: Vec<String> = args.collect();

    return header_tool::run(&args[1..]);
}

#[derive(Debug, Clone)]
struct ArgsError;

impl Display for ArgsError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "No headers were passed!")
    }
}

impl Error for ArgsError {
    fn description(&self) -> &str {
        "No headers were passed!"
    }
}