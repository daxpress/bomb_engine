use std::{env::args, fmt::Display, error::Error};
use header_tool::header_tool;

fn main() -> Result<(), anyhow::Error> {
    let args = args();
    if args.len() <= 1 {
        let error = anyhow::anyhow!(ArgsError);
        return Err(error);
    }
    let args: Vec<String> = args.collect();

    let modules = parse_arguments(&args[1..]);
    return header_tool::run(modules);
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

fn parse_arguments(args: &[String]) -> Vec<header_tool::Module> {
    let mut modules = Vec::new();
    let mut current_module= header_tool::Module::default();

    for arg in args {
        if arg.contains("--module=") {
            if !current_module.is_empty() {
                modules.push(current_module);
            }
            current_module = Default::default();
            let splits: Vec<&str> = arg.split("=").collect();
            if splits.len() != 2 {
                panic!("Error in module passing format.");
            }
            current_module.module_name = splits[1].to_string();
        }
        else {
            current_module.module_headers.push(arg.to_owned());
        }
    }
    if !current_module.is_empty() {
        modules.push(current_module);
    }
    modules
}