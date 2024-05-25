use anyhow::{Error, Ok};
use rayon::prelude::*;
use sha1_smol::Sha1;
use std::{
    collections::HashMap,
    fs::File,
    io::{self, BufRead, Read, Write},
    path::Path,
};

const CACHE_NAME: &'static str = "headercache.cache";

/// Performs checks on the headers to know which operations to perform for each header.
/// Loads a cache (if present) upon run_check() and writes the updated one when dropped.
pub struct HeaderChecker {
    headers_cache: HashMap<String, String>,
}

#[derive(Debug, PartialEq)]
pub enum HeaderCacheOp {
    Add(String, String),
    Remove(String),
    Update(String, String),
    Skip,
}

impl HeaderChecker {
    pub fn new() -> Self {
        HeaderChecker {
            headers_cache: HashMap::new(),
        }
    }

    pub fn run_check(&mut self, headers: &[String]) -> Vec<HeaderCacheOp> {
        println!("Checking Headers...");
        match self.read_cache() {
            Result::Ok(cache) => self.headers_cache = cache,
            Err(_) => println!("No header cache found!"),
        };
        let result = self.filter_headers(headers.to_vec());
        //println!("Check result: {:#?}", result);
        self.update_cache(&result);
        return result;
    }

    /// Reads the cache from disk and returns HashMap<header, content_hash>
    fn read_cache(&self) -> Result<HashMap<String, String>, Error> {
        let file = File::open(Path::new(CACHE_NAME))?;
        println!("Reading cached file...");
        let lines = io::BufReader::new(file).lines();
        let lines = lines
            .par_bridge()
            .map(|x| {
                let line = x.unwrap();
                let subs: Vec<&str> = line.split(":").collect();
                (String::from(subs[0]), String::from(subs[1]))
            })
            .collect();
        Ok(lines)
    }

    /// Takes in the headers list and checks against the cache to find
    /// the operations to perform to update the cache and generate information
    fn filter_headers(&self, headers: Vec<String>) -> Vec<HeaderCacheOp> {
        // first get the operation list from a copy of the headers vector (we still need headers after this operation)
        let mut operation_list: Vec<HeaderCacheOp> = headers
            .to_vec()
            .par_iter()
            .map(|header| self.get_headerop(header))
            .filter(|op| *op != HeaderCacheOp::Skip)
            .collect();
        // because we could only get the Add and Update operations, now we check for Removals
        // by comparing the cached headers with the passed headers instead;
        // to do this we first need to get them relative to the engine dir.
        let headers: Vec<String> = headers
            .par_iter()
            .map(|header| Self::relative_header(header))
            .collect();
        let mut remove: Vec<HeaderCacheOp> = self
            .headers_cache
            .clone()
            .into_par_iter()
            .filter(|entry| !headers.contains(&entry.0))
            .map(|elem| HeaderCacheOp::Remove(elem.0.to_string()))
            .collect();
        operation_list.append(&mut remove);
        operation_list
    }

    /// Returns a `HeaderCacheOp` for the passed header
    fn get_headerop(&self, header: &String) -> HeaderCacheOp {
        let mut file = File::open(Path::new(header)).unwrap();
        // hash the content to detect changes in between sessions
        let mut content = Default::default();
        file.read_to_end(&mut content).unwrap();
        // using Sha1 instead of Md5 this time - it doesn't really change much in this context
        let hasher = Sha1::from(&content);
        let hash = hasher.digest().to_string();
        // we trim the headers to have a path relative to bomb_engine
        let trimmed = Self::relative_header(header);
        // now se what operation to run
        if self.headers_cache.contains_key(&trimmed) {
            // if is in cache then:
            if self.headers_cache[&trimmed] != hash {
                HeaderCacheOp::Update(trimmed, hash)
            } else {
                HeaderCacheOp::Skip
            }
        } else {
            HeaderCacheOp::Add(trimmed, hash)
        }
    }

    /// Returns a header file with path relative to bomb_engine dir
    fn relative_header(header: &String) -> String {
        const ENGINE_DIR: &'static str = "/bomb_engine/";
        let start = header.find(ENGINE_DIR).unwrap() + ENGINE_DIR.len();
        header.split_at(start).1.to_string()
    }

    /// writes the caceh back to disk
    fn write_cache(&self) {
        let file = File::create(Path::new(CACHE_NAME)).unwrap();
        let mut writer = io::BufWriter::new(file);
        for (header, hash) in &self.headers_cache {
            writeln!(writer, "{header}:{hash}").unwrap();
        }
        writer.flush().unwrap()
    }
    fn update_cache(&mut self, check_results: &Vec<HeaderCacheOp>) {
        for result in check_results {
            match result {
                HeaderCacheOp::Remove(header) => {
                    self.headers_cache.remove(header);
                }
                HeaderCacheOp::Skip => (),
                HeaderCacheOp::Add(header, hash) | HeaderCacheOp::Update(header, hash) => {
                    self.headers_cache.insert(header.to_string(), hash.to_string());
                }
            }
        }
    }
}

impl Drop for HeaderChecker {
    fn drop(&mut self) {
        self.write_cache()
    }
}
