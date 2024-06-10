use serde::{Deserialize, Serialize};

#[derive(Default, Debug, Serialize, Deserialize)]
pub enum AccessModifier {
    #[default]
    Public,
    Protected,
    Private,
    None,
}

#[derive(Default, Debug, Serialize, Deserialize)]
pub struct Function {
    pub name: String,
    pub return_type: String,
    pub args: Vec<Argument>,
    pub is_static: bool,
    pub is_const: bool,
}

#[derive(Default, Debug, Serialize, Deserialize)]
pub struct Method {
    pub name: String,
    pub return_type: String,
    pub args: Vec<Argument>,
    pub access: AccessModifier,
    pub is_virtual: bool,
    pub is_pure_virtual: bool,
    pub is_static: bool,
    pub is_const: bool,
}

#[derive(Default, Debug, Serialize, Deserialize)]
pub struct Variable {
    pub name: String,
    pub var_type: String,
    pub is_static: bool,
    pub is_const: bool,
}

#[derive(Default, Debug, Serialize, Deserialize)]
pub struct Member {
    pub name: String,
    pub var_type: String,
    pub is_static: bool,
    pub is_const: bool,
    pub access: AccessModifier,
}

#[derive(Default, Debug, Serialize, Deserialize)]
pub struct Class {
    pub name: String,
    pub members: Vec<Member>,
    pub methods: Vec<Method>,
}

#[derive(Default, Debug, Serialize, Deserialize)]
pub struct Enum {
    pub name: String,
    pub underlying_type: String,
    pub enumerators: Vec<Enumerator>,
}

#[derive(Default, Debug, Serialize, Deserialize)]
pub struct Enumerator {
    pub name: String,
    pub s_value: i64,
    pub u_value: u64,
}

#[derive(Default, Debug, Serialize, Deserialize)]
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

#[derive(Default, Debug, Serialize, Deserialize)]
pub struct Argument {
    pub name: String,
    pub var_type: String,
    pub is_const: bool,
}

#[derive(Default, Debug, Serialize, Deserialize)]
pub struct Struct {
    pub name: String,
    pub members: Vec<Member>,
    pub methods: Vec<Method>,
}

impl Struct {
    pub fn from_class(class: Class) -> Self {
        Struct {
            members: class.members,
            methods: class.methods,
            name: class.name,
        }
    }
}
