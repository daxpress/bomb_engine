use header_tool::Namespace;

/// Implement this trait to have a uniform way to produce some bindings across all the entities
pub trait Pybindify {
    fn to_cpp_binding(&self) -> String;
}

impl Pybindify for Namespace {
    fn to_cpp_binding(&self) -> String {
        let string = String::new();
        
        
        
        string
    }
}


#[cfg(test)]
mod tests {

}