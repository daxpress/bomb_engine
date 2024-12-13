use header_tool::{
    AccessModifier, Argument, Class, Enum, Enumerator, Function, Member, Method, Namespace, Struct,
    Variable, SUPPORTED_EXTENSIONS,
};

/// Implement this trait to have a uniform way to produce some bindings across all the entities
pub trait Pybindify {
    fn to_cpp(&self, module: &str) -> String;
}

static IGNORED_NAMESPACES: [&str; 3] = ["bomb_engine", "", "std"];

fn get_entities_code<'a, I>(
    iter: Box<dyn Iterator<Item = I> + 'a>,
    module: &str,
    separator: &str,
) -> String
where
    I: 'a + Pybindify,
{
    iter.map(|item| item.to_cpp(module))
        .collect::<Vec<String>>()
        .join(separator)
}

impl Pybindify for &Namespace {
    fn to_cpp(&self, module: &str) -> String {
        let mut body = String::new();
        // set the current module to the one passed
        let mut current_module = module;
        // change module name if new module is found
        if !IGNORED_NAMESPACES.contains(&self.name.as_str())
            && !SUPPORTED_EXTENSIONS
                .iter()
                .any(|ext| self.name.ends_with(ext))
        {
            current_module = &self.name;
            // push the using directive to avoid having to use parent information to link data
            body.push_str(&format!("\tusing namespace {};\n", current_module));
            body.push_str(&format!(
                "auto {current_module} = {module}.def_submodule(\"{current_module}\", \"\");\n"
            ));
        }

        body.push_str(&get_entities_code(
            Box::new(self.classes.iter()),
            current_module,
            "\n",
        ));
        body.push_str(&get_entities_code(
            Box::new(self.structs.iter()),
            current_module,
            "\n",
        ));
        body.push_str(&get_entities_code(
            Box::new(self.functions.iter()),
            current_module,
            "\n",
        ));
        body.push_str(&get_entities_code(
            Box::new(self.enums.iter()),
            current_module,
            "\n",
        ));
        body.push_str(&get_entities_code(
            Box::new(self.variables.iter()),
            current_module,
            "\n",
        ));
        // process namespace content

        // go to the other namespaces
        body.push_str(&get_entities_code(
            Box::new(self.namespaces.iter()),
            current_module,
            "\n",
        ));

        body
    }
}

impl Pybindify for &Class {
    fn to_cpp(&self, module: &str) -> String {
        let mut body = String::new();
        // declaration of the class
        body.push_str(&format!("\tpy::class_<{}", self.name));
        // add parent classes
        if !self.parents.is_empty() {
            body.push_str(", ");
            body.push_str(&self.parents.join(", "));
        }
        // close templ and declaration
        body.push_str(&format!(">({module}, \"{}\")\n", self.name));

        //ctors
        if !self.constructors.is_empty() {
            let ctors = self
                .constructors
                .iter()
                .map(|ctor| ctor_to_cpp(ctor))
                .collect::<Vec<String>>()
                .join("\n");
            body.push_str(&ctors);
            body.push_str("\n");
        }
        if !self.methods.is_empty() {
            // methods
            body.push_str(&get_entities_code(
                Box::new(self.methods.iter()),
                &self.name,
                "\n",
            ));
            body.push_str("\n");
        }
        if !self.members.is_empty() {
            // members
            body.push_str(&get_entities_code(
                Box::new(self.members.iter()),
                &self.name,
                "\n",
            ));
        }
        // close
        body.push_str(";\n");

        body
    }
}

fn ctor_to_cpp(ctor: &Method) -> String {
    let mut body = String::from("\t.def(py::init<");
    if !ctor.args.is_empty() {
        let args = ctor
            .args
            .iter()
            .map(|arg| arg.var_type.clone())
            .collect::<Vec<String>>();
        body.push_str(&args.join(", "));
    }
    //close
    body.push_str(">()");
    if !ctor.args.is_empty() {
        body.push_str(", ");
        body.push_str(&get_entities_code(Box::new(ctor.args.iter()), "", ", "));
    }
    body.push_str(")");
    body
}

impl Pybindify for Class {
    fn to_cpp(&self, module: &str) -> String {
        (&self).to_cpp(module)
    }
}

impl Pybindify for &Struct {
    fn to_cpp(&self, module: &str) -> String {
        // it's basically a castrated Class, plus python only has classes
        //we'll use that same code
        Class::from_struct(self).to_cpp(module)
    }
}

impl Pybindify for Struct {
    fn to_cpp(&self, module: &str) -> String {
        (&self).to_cpp(module)
    }
}

impl Pybindify for &Enum {
    fn to_cpp(&self, module: &str) -> String {
        let mut code = String::new();
        //declare enum class
        code.push_str(&format!(
            "\tpy::enum_<{}>({}, \"{}\")\n",
            self.name, module, self.name
        ));
        // fill enumerators
        code.push_str(&get_entities_code(
            Box::new(self.enumerators.iter()),
            &self.name,
            "\n",
        ));
        // close enum declaration
        code.push_str(";\n"); // .export_values() not needed for enum class
        code
    }
}

impl Pybindify for Enum {
    fn to_cpp(&self, _module: &str) -> String {
        (&self).to_cpp(_module)
    }
}

impl Pybindify for &Enumerator {
    fn to_cpp(&self, enum_name: &str) -> String {
        String::from(&format!(
            "\t.value(\"{}\", {}::{})",
            self.name, enum_name, self.name
        ))
    }
}

impl Pybindify for Enumerator {
    fn to_cpp(&self, module: &str) -> String {
        (&self).to_cpp(module)
    }
}

impl Pybindify for &Function {
    fn to_cpp(&self, module: &str) -> String {
        let mut code = String::new();
        code.push_str(&format!(
            "\t{module}.def(\"{}\", &{}, \"{}\"",
            self.name, self.name, self.brief
        ));
        if !self.args.is_empty() {
            code.push_str(", ");
        }
        // fill arguments list
        code.push_str(&get_entities_code(Box::new(self.args.iter()), module, ", "));
        //close function call
        code.push_str(");\n");
        code
    }
}

impl Pybindify for Function {
    fn to_cpp(&self, module: &str) -> String {
        (&self).to_cpp(module)
    }
}

impl Pybindify for &Method {
    fn to_cpp(&self, module: &str) -> String {
        if self.access != AccessModifier::Public {
            return String::new();
        }
        let mut code = String::new();

        if self.is_overload {
            // overload function requires this signature!!
            code.push_str(&format!("\t.def(\"{}\", py::overload_cast<", self.name));
            // fill types here
            if !self.args.is_empty() {
                let args = self
                    .args
                    .iter()
                    .map(|arg| arg.var_type.clone())
                    .collect::<Vec<String>>();
                code.push_str(&args.join(", "));
            }
            code.push_str(">");

            code.push_str(&format!("(&{}::{}), \"{}\"", module, self.name, self.brief));
            if !self.args.is_empty() {
                code.push_str(", ");
            }
            // fill arguments list
            code.push_str(&get_entities_code(Box::new(self.args.iter()), module, ", "));
            //close function call
            code.push_str(")");
            code
        } else {
            code.push_str(&format!(
                "\t.def(\"{}\", &{}::{}, \"{}\"",
                self.name, module, self.name, self.brief
            ));
            if !self.args.is_empty() {
                code.push_str(", ");
            }
            // fill arguments list
            code.push_str(&get_entities_code(Box::new(self.args.iter()), module, ", "));
            //close function call
            code.push_str(")");
            code
        }
        // will probably get bigger due to operators...
    }
}

impl Pybindify for Method {
    fn to_cpp(&self, module: &str) -> String {
        (&self).to_cpp(module)
    }
}
impl Pybindify for &Variable {
    fn to_cpp(&self, module: &str) -> String {
        String::from(&format!(
            "\t{module}.attr(\"{}\") = {};\n",
            self.name, self.name
        ))
    }
}

impl Pybindify for Variable {
    fn to_cpp(&self, _module: &str) -> String {
        (&self).to_cpp(_module)
    }
}

impl Pybindify for &Member {
    fn to_cpp(&self, module: &str) -> String {
        // ignore other access specifiers
        if self.access != AccessModifier::Public {
            return String::new();
        }
        return if self.is_static {
            if self.is_const {
                // def_readonly_static
                String::from(&format!(
                    "\t.def_readonly_static(\"{}\", &{}::{})",
                    self.name, module, self.name
                ))
            } else {
                // def_readwrite_static
                String::from(&format!(
                    "\t.def_readwrite_static(\"{}\", &{}::{})",
                    self.name, module, self.name
                ))
            }
        } else {
            if self.is_const {
                // def_readonly
                String::from(&format!(
                    "\t.def_readonly(\"{}\", &{}::{})",
                    self.name, module, self.name
                ))
            } else {
                // def_readwrite
                String::from(&format!(
                    "\t.def_readwrite(\"{}\", &{}::{})",
                    self.name, module, self.name
                ))
            }
        };
    }
}

impl Pybindify for Member {
    fn to_cpp(&self, module: &str) -> String {
        (&self).to_cpp(module)
    }
}

impl Pybindify for &Argument {
    fn to_cpp(&self, _module: &str) -> String {
        String::from(&format!("py::arg(\"{}\")", self.name))
    }
}
impl Pybindify for Argument {
    fn to_cpp(&self, _module: &str) -> String {
        (&self).to_cpp(_module)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use header_tool::AccessModifier::Public;
    use header_tool::Argument;

    #[test]
    fn test_argument() {
        let mut argument = Argument::default();
        argument.name = "some_argument".to_string();
        let expected = "py::arg(\"some_argument\")";
        assert_eq!(&argument.to_cpp(""), expected);
    }

    #[test]
    fn test_function_noargs() {
        let mut function = Function::default();
        function.name = "some_function".to_string();
        function.brief = "this is a brief".to_string();

        assert_eq!(
            function.to_cpp("m"),
            "\tm.def(\"some_function\", &some_function, \"this is a brief\");\n"
        );
    }

    #[test]
    fn test_function_one_two_three_args() {
        let mut function = Function {
            name: "some_function".to_string(),
            brief: "this is a brief".to_string(),
            args: vec![Argument {
                name: "arg1".to_string(),
                is_const: false,
                var_type: "".to_string(),
            }],
            is_const: false,
            is_static: false,
            return_type: "".to_string(),
        };

        assert_eq!(
            function.to_cpp("m"),
            "\tm.def(\"some_function\", &some_function, \"this is a brief\", \
            py::arg(\"arg1\"));\n"
        );

        function.args.push(Argument {
            name: "arg2".to_string(),
            is_const: false,
            var_type: "".to_string(),
        });

        assert_eq!(
            function.to_cpp("m"),
            "\tm.def(\"some_function\", &some_function, \"this is a brief\", \
            py::arg(\"arg1\"), py::arg(\"arg2\"));\n"
        );

        function.args.push(Argument {
            name: "arg3".to_string(),
            is_const: false,
            var_type: "".to_string(),
        });

        assert_eq!(
            function.to_cpp("m"),
            "\tm.def(\"some_function\", &some_function, \"this is a brief\", \
            py::arg(\"arg1\"), py::arg(\"arg2\"), py::arg(\"arg3\"));\n"
        );
    }

    #[test]
    fn test_enumerator() {
        let mut enumerator = Enumerator::default();
        enumerator.name = "some_enum".to_string();

        assert_eq!(
            enumerator.to_cpp("myenum"),
            "\t.value(\"some_enum\", myenum::some_enum)"
        );
    }

    #[test]
    fn test_enum() {
        let animal = Enum {
            name: "Animal".to_string(),
            brief: "".to_string(),
            enumerators: vec![
                Enumerator {
                    name: "Cat".to_string(),
                    s_value: 1,
                    u_value: 1,
                },
                Enumerator {
                    name: "Dog".to_string(),
                    s_value: 1,
                    u_value: 1,
                },
                Enumerator {
                    name: "Horse".to_string(),
                    s_value: 1,
                    u_value: 1,
                },
            ],
            underlying_type: "".to_string(),
        };

        assert_eq!(
            animal.to_cpp("m"),
            "\tpy::enum_<Animal>(m, \"Animal\")\n\
        \t.value(\"Cat\", Animal::Cat)\n\
        \t.value(\"Dog\", Animal::Dog)\n\
        \t.value(\"Horse\", Animal::Horse);\n"
        );
    }

    #[test]
    fn test_variable() {
        let mut variable = Variable::default();
        variable.name = "some_variable".to_string();
        assert_eq!(
            variable.to_cpp("m"),
            "\tm.attr(\"some_variable\") = some_variable;\n"
        );
    }

    #[test]
    fn test_method_one_two_three_args() {
        let mut method = Method {
            name: "some_function".to_string(),
            brief: "this is a brief".to_string(),
            access: Public,
            is_pure_virtual: false,
            is_virtual: false,
            args: vec![Argument {
                name: "arg1".to_string(),
                is_const: false,
                var_type: "".to_string(),
            }],
            is_const: false,
            is_static: false,
            return_type: "".to_string(),
            is_overload: false,
        };

        assert_eq!(
            method.to_cpp("m"),
            "\t.def(\"some_function\", &m::some_function, \"this is a brief\", \
            py::arg(\"arg1\"))"
        );

        method.args.push(Argument {
            name: "arg2".to_string(),
            is_const: false,
            var_type: "".to_string(),
        });

        assert_eq!(
            method.to_cpp("m"),
            "\t.def(\"some_function\", &m::some_function, \"this is a brief\", \
            \
            py::arg(\"arg1\"), py::arg(\"arg2\"))"
        );

        method.args.push(Argument {
            name: "arg3".to_string(),
            is_const: false,
            var_type: "".to_string(),
        });

        assert_eq!(
            method.to_cpp("m"),
            "\t.def(\"some_function\", &m::some_function, \"this is a brief\", \
            py::arg(\"arg1\"), py::arg(\"arg2\"), py::arg(\"arg3\"))"
        );
    }
    #[test]
    fn test_method_overload_params() {
        let method = Method {
            name: "some_function".to_string(),
            brief: "this is a brief".to_string(),
            access: Public,
            is_pure_virtual: false,
            is_virtual: false,
            args: vec![
                Argument {
                    name: "arg1".to_string(),
                    is_const: false,
                    var_type: "int".to_string(),
                },
                Argument {
                    name: "arg2".to_string(),
                    is_const: false,
                    var_type: "const char".to_string(),
                },
                Argument {
                    name: "arg3".to_string(),
                    is_const: false,
                    var_type: "Cat&".to_string(),
                },
            ],
            is_const: false,
            is_static: false,
            return_type: "".to_string(),
            is_overload: true,
        };

        assert_eq!(
            method.to_cpp("m"),
            "\t.def(\"some_function\", py::overload_cast<int, const char, Cat&>(&m::some_function), \
            \"this is a brief\", \
            py::arg(\"arg1\"), py::arg(\"arg2\"), py::arg(\"arg3\"))"
        );
    }

    #[test]
    fn test_members() {
        let mut member = Member::default();
        member.name = "some_member".to_string();
        member.brief = "".to_string();
        member.access = Public;

        member.is_static = true;
        member.is_const = true;
        assert_eq!(
            member.to_cpp("m"),
            "\t.def_readonly_static(\"some_member\", &m::some_member)"
        );
        member.is_const = false;
        assert_eq!(
            member.to_cpp("m"),
            "\t.def_readwrite_static(\"some_member\", &m::some_member)"
        );

        member.is_static = false;
        member.is_const = true;
        assert_eq!(
            member.to_cpp("m"),
            "\t.def_readonly(\"some_member\", &m::some_member)"
        );
        member.is_const = false;
        assert_eq!(
            member.to_cpp("m"),
            "\t.def_readwrite(\"some_member\", &m::some_member)"
        );
    }

    #[test]
    fn test_class() {
        let class = Class {
            name: "Cat".to_string(),
            brief: "".to_string(),
            size: 123,
            alignment: 16,
            is_abstract: false,
            parents: vec![
                "Animal".to_string(),
                "Mammal".to_string(),
                "Devil".to_string(),
            ],
            constructors: vec![Method {
                name: "Cat".to_string(),
                brief: "ctor".to_string(),
                is_const: false,
                is_static: false,
                is_virtual: false,
                is_pure_virtual: false,
                args: vec![Argument {
                    name: "kills".to_string(),
                    is_const: true,
                    var_type: "const uint64_t".to_string(),
                }],
                access: Public,
                return_type: "".to_string(),
                is_overload: false,
            }],
            destructor: Method::default(),
            members: vec![Member {
                name: "kills_count".to_string(),
                access: Public,
                is_static: false,
                var_type: "uint64_t".to_string(),
                brief: "exactly this".to_string(),
                is_const: false,
                offset: 0,
            }],
            methods: vec![
                Method {
                    name: "meow".to_string(),
                    brief: "MEOW Motherfucker!".to_string(),
                    is_const: false,
                    is_static: false,
                    is_virtual: false,
                    is_pure_virtual: false,
                    args: Vec::new(),
                    access: Public,
                    return_type: "".to_string(),
                    is_overload: false,
                },
                Method {
                    name: "judge".to_string(),
                    brief: "...loser".to_string(),
                    is_const: false,
                    is_static: false,
                    is_virtual: false,
                    is_pure_virtual: false,
                    args: Vec::new(),
                    access: Public,
                    return_type: "".to_string(),
                    is_overload: false,
                },
            ],
        };

        assert_eq!(
            class.to_cpp("m"),
            "\tpy::class_<Cat, Animal, Mammal, Devil>(m, \"Cat\")\n\
            \t.def(py::init<const uint64_t>(), py::arg(\"kills\"))\n\
            \t.def(\"meow\", &Cat::meow, \"MEOW Motherfucker!\")\n\
            \t.def(\"judge\", &Cat::judge, \"...loser\")\n\
            \t.def_readwrite(\"kills_count\", &Cat::kills_count);\n"
        );
    }

    #[test]
    fn test_struct() {
        let class = Struct {
            name: "Vec3".to_string(),
            brief: "".to_string(),
            size: 123,
            alignment: 16,
            constructors: Vec::new(),
            destructor: Method::default(),
            members: vec![
                Member {
                    name: "x".to_string(),
                    access: Public,
                    is_static: false,
                    var_type: "float".to_string(),
                    brief: "x component".to_string(),
                    is_const: false,
                    offset: 0,
                },
                Member {
                    name: "y".to_string(),
                    access: Public,
                    is_static: false,
                    var_type: "float".to_string(),
                    brief: "y component".to_string(),
                    is_const: false,
                    offset: 4,
                },
                Member {
                    name: "z".to_string(),
                    access: Public,
                    is_static: false,
                    var_type: "float".to_string(),
                    brief: "z component".to_string(),
                    is_const: false,
                    offset: 8,
                },
            ],
            methods: Vec::new(),
        };

        assert_eq!(
            class.to_cpp("m"),
            "\tpy::class_<Vec3>(m, \"Vec3\")\n\
            \t.def_readwrite(\"x\", &Vec3::x)\n\
            \t.def_readwrite(\"y\", &Vec3::y)\n\
            \t.def_readwrite(\"z\", &Vec3::z);\n"
        );
    }
}
