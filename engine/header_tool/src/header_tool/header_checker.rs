use std::{collections::HashMap, fs::File, path::Path};

const CACHE_NAME: &'static str = "headercache.cache";

pub struct HeaderChecker {
    headers_cache: HashMap<String, String>,
}

impl HeaderChecker {
    pub fn new() -> Self {
        HeaderChecker {
            headers_cache: HashMap::new(),
        }
    }

    pub fn run_check(&mut self, headers: &[String]) {
        let path = Path::new(CACHE_NAME);

        if !path.exists() {
            println!("No header cache found.");
            return;
        }
        if let Ok(file) = File::open(path) {
            
        };
    }

    fn filter_checks(&self) {}
}
