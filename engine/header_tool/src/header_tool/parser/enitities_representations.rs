use serde::{Deserialize, Serialize};

#[derive(Default, Debug, Serialize, Deserialize, PartialEq, Clone)]
pub enum AccessModifier {
    #[default]
    Public,
    Protected,
    Private,
    None,
}

#[derive(Default, Debug, Serialize, Deserialize, PartialEq, Clone)]
pub struct Function {
    pub name: String,
    pub brief: String,
    pub return_type: String,
    pub args: Vec<Argument>,
    pub is_static: bool,
    pub is_const: bool,
}

#[derive(Default, Debug, Serialize, Deserialize, PartialEq, Clone)]
pub struct Method {
    pub name: String,
    pub brief: String,
    pub return_type: String,
    pub args: Vec<Argument>,
    pub access: AccessModifier,
    pub is_virtual: bool,
    pub is_pure_virtual: bool,
    pub is_static: bool,
    pub is_const: bool,
}

#[derive(Default, Debug, Serialize, Deserialize, PartialEq, Clone)]
pub struct Variable {
    pub name: String,
    pub brief: String,
    pub var_type: String,
    pub is_static: bool,
    pub is_const: bool,
}

#[derive(Default, Debug, Serialize, Deserialize, PartialEq, Clone)]
pub struct Member {
    pub name: String,
    pub brief: String,
    pub var_type: String,
    pub is_static: bool,
    pub is_const: bool,
    pub access: AccessModifier,
    pub offset: usize,
}

#[derive(Default, Debug, Serialize, Deserialize, PartialEq, Clone)]
pub struct Class {
    pub name: String,
    pub brief: String,
    pub size: usize,
    pub alignment: usize,
    pub constructors: Vec<Method>,
    pub destructor: Method,
    pub members: Vec<Member>,
    pub methods: Vec<Method>,
    pub is_abstract: bool,
    pub parents: Vec<String>,
}

#[derive(Default, Debug, Serialize, Deserialize, PartialEq, Clone)]
pub struct Enum {
    pub name: String,
    pub brief: String,
    pub underlying_type: String,
    pub enumerators: Vec<Enumerator>,
}

#[derive(Default, Debug, Serialize, Deserialize, PartialEq, Clone)]
pub struct Enumerator {
    pub name: String,
    pub s_value: i64,
    pub u_value: u64,
}

/// Namespace is treated as the root of header as well, so the name at the root will actually contain
/// the header file path, and after that all the internal namespaces contain the namespace name as the name implies
#[derive(Default, Debug, Serialize, Deserialize, PartialEq, Clone)]
pub struct Namespace {
    pub name: String,
    pub namespaces: Vec<Namespace>,
    pub classes: Vec<Class>,
    pub structs: Vec<Struct>,
    pub functions: Vec<Function>,
    pub enums: Vec<Enum>,
    pub variables: Vec<Variable>,
}

// Implementing here types that overlap with already defined ones (class/struct)
// to have them printed out correctly to json.

#[derive(Default, Debug, Serialize, Deserialize, PartialEq, Clone)]
pub struct Argument {
    pub name: String,
    pub var_type: String,
    pub is_const: bool,
}

#[derive(Default, Debug, Serialize, Deserialize, PartialEq, Clone)]
pub struct Struct {
    pub name: String,
    pub brief: String,
    pub size: usize,
    pub alignment: usize,
    pub constructors: Vec<Method>,
    pub destructor: Method,
    pub members: Vec<Member>,
    pub methods: Vec<Method>,
}

impl Struct {
    pub fn from_class(class: Class) -> Self {
        Struct {
            members: class.members,
            methods: class.methods,
            name: class.name,
            size: class.size,
            alignment: class.alignment,
            brief: class.brief,
            constructors: class.constructors,
            destructor: class.destructor,
        }
    }
    pub fn to_class(self) -> Class {
        Class {
            members: self.members,
            methods: self.methods,
            name: self.name,
            size: self.size,
            alignment: self.alignment,
            brief: self.brief,
            constructors: self.constructors,
            destructor: self.destructor,
            is_abstract: false,
            parents: Vec::new(),
        }
    }
}

impl Class {
    pub fn from_struct(str: &Struct) -> Self {
        Class {
            members: str.members.clone(),
            methods: str.methods.clone(),
            name: str.name.clone(),
            size: str.size,
            alignment: str.alignment,
            brief: str.brief.clone(),
            constructors: str.constructors.clone(),
            destructor: str.destructor.clone(),
            is_abstract: false,
            parents: Vec::new(),
        }
    }
}
