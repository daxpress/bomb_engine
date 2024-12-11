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

#[derive(Debug, PartialEq, Clone)]
pub enum HeaderCacheOp {
    Add(String, String),
    Remove(String, String),
    Update(String, String),
    Skip(String, String),
}

impl HeaderCacheOp {
    pub fn get_header(&self) -> &str {
        match self {
            HeaderCacheOp::Add(header, _) => &header,
            HeaderCacheOp::Update(header, _) => &header,
            HeaderCacheOp::Remove(header, _) => &header,
            HeaderCacheOp::Skip(header, _) => &header,
        }
    }
}

#[derive(Debug, PartialEq, Clone)]
pub struct CheckerResult<'a> {
    pub header: &'a str,
    pub module: &'a str,
}

impl<'a> CheckerResult<'a> {
    pub fn new(header: &'a str, module: &'a str) -> Self {
        Self { header, module }
    }
}

/// Performs checks on the headers to know which operations to perform for each header.
/// Loads a cache (if present) upon run_check() and writes the updated one when dropped.
pub struct HeaderChecker<'a> {
    headers_cache: HashMap<String, String>,
    operations: Vec<(HeaderCacheOp, &'a str)>,
    current_mod: &'a str,
    cache_name: &'static str,
}

impl<'a> HeaderChecker<'a> {
    pub fn default() -> Self {
        let mut cache = HashMap::new();
        match Self::read_cache(CACHE_NAME) {
            Result::Ok(loaded) => cache = loaded,
            Err(_) => println!("No header cache found!"),
        };
        HeaderChecker {
            headers_cache: cache,
            operations: Vec::new(),
            current_mod: "",
            cache_name: CACHE_NAME,
        }
    }
    pub fn new(cache_name: &'static str) -> Self {
        let mut cache = HashMap::new();
        match Self::read_cache(cache_name) {
            Result::Ok(loaded) => cache = loaded,
            Err(_) => println!("No header cache found!"),
        };
        HeaderChecker {
            headers_cache: cache,
            operations: Vec::new(),
            current_mod: "",
            cache_name: cache_name,
        }
    }

    pub fn check(&mut self, headers: &[String], module: &'a str) {
        println!("Checking Headers in {}...", module);
        self.current_mod = module;
        let result: Vec<(HeaderCacheOp, &str)> = self
            .filter_headers(headers.to_vec())
            .into_iter()
            .map(|op| (op, module))
            .collect();
        self.update_cache(&result);
        self.operations.extend(result)
    }

    // call to tell the checker that the checks are over and to finalize the results.
    pub fn close_checks(&mut self) {
        // we don't know from which module the removed stuff comes from,
        // so we put an empty &str. the remover won't read it anyway
        let to_remove = self
            .find_to_remove()
            .into_iter()
            .map(|op| (op, ""))
            .collect();
        self.update_cache(&to_remove);
        self.operations.extend(to_remove);
    }

    pub fn headers_to_generate(&self) -> Vec<CheckerResult> {
        self.operations
            .iter()
            .filter(|op| match &op.0 {
                HeaderCacheOp::Add(_, _) => true,
                HeaderCacheOp::Update(_, _) => true,
                _ => false,
            })
            .map(|op| CheckerResult::new(op.0.get_header(), op.1))
            .collect()
    }

    pub fn headers_to_skip(&self) -> Vec<CheckerResult> {
        self.operations
            .iter()
            .filter(|op| match &op.0 {
                HeaderCacheOp::Skip(_, _) => true,
                _ => false,
            })
            .map(|op| CheckerResult::new(op.0.get_header(), op.1))
            .collect()
    }

    pub fn headers_to_delete(&self) -> Vec<CheckerResult> {
        let filtered: Vec<CheckerResult> = self
            .operations
            .iter()
            .filter(|op| match &op.0 {
                HeaderCacheOp::Remove(_, _) => true,
                _ => false,
            })
            .map(|op| CheckerResult::new(op.0.get_header(), op.1))
            .collect();
        filtered
    }

    /// Reads the cache from disk and returns HashMap<header, content_hash>
    fn read_cache(cache_name: &'static str) -> Result<HashMap<String, String>, Error> {
        // TODO: cache also the module for each of them
        let file = File::open(Path::new(cache_name))?;
        println!("Reading cached file...");
        let lines = io::BufReader::new(file).lines();
        let lines = lines
            .par_bridge()
            .map(|x| {
                let line = x.unwrap();
                let subs: Vec<&str> = line.split("=").collect();
                (String::from(subs[0]), String::from(subs[1]))
            })
            .collect();
        Ok(lines)
    }

    /// Takes in the headers list and checks against the cache to find
    /// the operations to perform to update the cache and generate information
    fn filter_headers(&self, headers: Vec<String>) -> Vec<HeaderCacheOp> {
        // first get the operation list from a copy of the headers vector (we still need headers after this operation)
        headers
            .to_vec()
            .par_iter()
            .map(|header| self.get_headerop(header))
            //.filter(|op| *op != HeaderCacheOp::Skip)
            .collect()
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
        // now se what operation to run
        if self.headers_cache.contains_key(header) {
            // if is in cache then:
            if self.headers_cache[header] != hash {
                HeaderCacheOp::Update(header.to_string(), hash)
            } else {
                HeaderCacheOp::Skip(header.to_string(), hash)
            }
        } else {
            HeaderCacheOp::Add(header.to_string(), hash)
        }
    }

    // called after all the checks are done to find the headers that have to be removed
    fn find_to_remove(&self) -> Vec<HeaderCacheOp> {
        // all the checked headers are in the self.operations member (except for the skipped ones)
        // this means that we can use them to check if something was removed!
        let headers: Vec<&str> = self.operations.iter().map(|op| op.0.get_header()).collect();
        self.headers_cache
            .clone()
            .into_par_iter()
            .filter(|entry| !headers.contains(&entry.0.as_str()))
            .map(|elem| HeaderCacheOp::Remove(elem.0.to_string(), elem.1.to_string()))
            .collect()
    }

    /// writes the caceh back to disk
    fn write_cache(&self) {
        // TODO: cache the module information
        let file = File::create(Path::new(self.cache_name)).unwrap();
        let mut writer = io::BufWriter::new(file);
        for (header, hash) in &self.headers_cache {
            writeln!(writer, "{header}={hash}").unwrap();
        }
        writer.flush().unwrap()
    }
    fn update_cache(&mut self, check_results: &Vec<(HeaderCacheOp, &str)>) {
        for result in check_results {
            match &result.0 {
                HeaderCacheOp::Remove(header, _) => {
                    self.headers_cache.remove(header);
                }
                HeaderCacheOp::Skip(_, _) => (),
                HeaderCacheOp::Add(header, hash) | HeaderCacheOp::Update(header, hash) => {
                    self.headers_cache
                        .insert(header.to_string(), hash.to_string());
                }
            }
        }
    }
}

impl Drop for HeaderChecker<'_> {
    fn drop(&mut self) {
        self.write_cache()
    }
}

#[cfg(test)]
mod tests {
    use super::{HeaderChecker, CACHE_NAME};
    use crate::checker::CheckerResult;
    use std::{fs, path::Path};

    const TEST_FILE: &'static str = "src/header_tool/somemod/test_header.h";
    // cache is generated in the root dir, use CACHE_NAME

    // RUN SEPARATELY!!!
    #[test]
    fn check_no_operations() {
        let mut checker = HeaderChecker::default();
        let header = vec![TEST_FILE.to_string()];
        checker.check(&header, "somemod");
        checker.close_checks();

        let generate = checker.headers_to_generate();
        assert!(generate.is_empty());

        let delete = checker.headers_to_delete();
        assert!(delete.is_empty());
    }
    #[test]
    fn check_generation_one() {
        // remove the generated cache (it will be recreated next anyway)
        if Path::new(CACHE_NAME).exists() {
            fs::remove_file(CACHE_NAME).unwrap();
        }
        // now that wee don't have the cache we can expect to have the file to be generated.

        let mut checker = HeaderChecker::default();
        let header = vec![TEST_FILE.to_string()];
        checker.check(&header, "somemod");
        checker.close_checks();

        let result = checker.headers_to_generate();
        assert!(!result.is_empty());

        let expected = vec![CheckerResult::new(TEST_FILE, "somemod")];
        assert_eq!(expected, result);
    }
}
