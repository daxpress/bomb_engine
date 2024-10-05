use std::env::args;
// use header_tool;

fn main() {
    println!("Running Pybinder...");
    let ht_gen_dir = args().skip(1).next().unwrap_or("No Arguments passed!".to_string());


    println!("{}", ht_gen_dir);
    println!("Bindings created.")
}
