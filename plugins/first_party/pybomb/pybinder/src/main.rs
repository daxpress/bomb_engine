use anyhow::anyhow;
use pybinder;
use std::env::args;
use std::path::Path;

fn main() -> Result<(), anyhow::Error> {
    let ht_gen_dir = args().skip(1).next();

    if let None = ht_gen_dir {
        return Err(anyhow!("No generated dir provided!"));
    }
    let ht_gen_dir = ht_gen_dir.unwrap();

    pybinder::run(Path::new(&ht_gen_dir))
}
