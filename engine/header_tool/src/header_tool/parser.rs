use std::collections::HashSet;

use clang::*;

pub mod enitities_representations;
use enitities_representations::*;

pub struct HeaderParser {
    clang: Clang,
    supported_entities: HashSet<EntityKind>,
}

const ARGS: [&'static str; 7] = [
    "--language=c++",
    "--std=c++23",
    "--stdlib=libc++23",
    "--system-header-prefix=test",
    // the following definitions are needed in order to not include other headers in the tu to read info.
    // it really isn't a problem as the funcionality must be coherent anyway.
    "-DBE_NAMESPACE=bomb_engine",
    "-Dexpose=clang::annotate(\"expose\")",
    "-Dhide=clang::annotate(\"hide\")",
];

const EXPOSE_NAME: &'static str = "expose";
const HIDE_NAME: &'static str = "hide";

impl HeaderParser {
    pub fn new() -> Self {
        // using hashet because we might support a lot more in future and having O(1) access
        // is very important with this task as it gets repeated A LOT
        let supported_entities = HashSet::from([
            EntityKind::ClassDecl,
            EntityKind::StructDecl,
            EntityKind::EnumDecl,
            EntityKind::FunctionDecl,
            EntityKind::VarDecl,
            EntityKind::Namespace,
        ]);

        Self {
            clang: Clang::new().unwrap(),
            supported_entities,
        }
    }

    /// Parses all the headers in a slice.
    pub fn parse_header_collection(&self, headers: &[&str]) -> Vec<Namespace> {
        let index = Index::new(&self.clang, false, false);
        headers
            .iter()
            .map(|header| self.parse_header(&index, header))
            .collect()
    }

    /// Used to filter the entities to only the ones we want to support.
    fn entity_is_supported(&self, kind: &EntityKind) -> bool {
        self.supported_entities.contains(&kind)
    }

    /// kicks off the parsing of a header.
    fn parse_header(&self, index: &Index, header: &str) -> Namespace {
        // get the translation unit for the header
        let tu = index
            .parser(header)
            .skip_function_bodies(true)
            .ignore_non_errors_from_included_files(true)
            .detailed_preprocessing_record(true)
            .arguments(&ARGS)
            .parse()
            .unwrap();

        // we pass the first entity (no namespace basically) to start the parsing
        // of the information contained inside the unit.
        // the engine code style wants just one namespace, however some might not follow
        // that and even if it's not okay to do so we will support multiple namespaces.
        // because of the guidelines it should be unlikely to have crazy namespaces nesting,
        // so it is fine to use a recursive algorithm here.
        self.parse_namespace(&tu.get_entity())
            .unwrap_or(Namespace::default())
    }

    /// Recursively visits the namespace to find entites to expose.
    fn parse_namespace(&self, namespace: &Entity) -> Option<Namespace> {
        if !namespace.is_in_main_file() {
            return None;
        }
        let mut namespace_info = Namespace::default();
        // if the namespace is empty the name will be the header file instead.
        namespace_info.name = namespace.get_display_name().unwrap();
        let parsed_entities: Vec<Entity> = namespace
            .get_children()
            .into_iter()
            .filter(|child| self.entity_is_supported(&child.get_kind()))
            .collect();

        for entity in parsed_entities.iter() {
            match entity.get_kind() {
                EntityKind::Namespace => {
                    if let Some(namespace) = self.parse_namespace(entity) {
                        namespace_info.namespaces.push(namespace);
                    }
                }
                EntityKind::ClassDecl => {
                    if let Some(class) = Self::parse_class(entity) {
                        namespace_info.classes.push(class)
                    }
                }
                EntityKind::StructDecl => {
                    if let Some(st) = Self::parse_struct(entity) {
                        namespace_info.structs.push(st)
                    }
                }
                EntityKind::EnumDecl => {
                    if let Some(en) = Self::parse_enum(entity) {
                        namespace_info.enums.push(en)
                    }
                }
                EntityKind::FunctionDecl => {
                    if let Some(func) = Self::parse_func(entity) {
                        namespace_info.functions.push(func)
                    }
                }
                EntityKind::VarDecl => {
                    if let Some(var) = Self::parse_variable(entity) {
                        namespace_info.variables.push(var)
                    }
                }
                _ => (),
            }
        }
        Some(namespace_info)
    }

    fn marked_as_exposed(entity: &Entity) -> bool {
        entity.has_attributes()
            && entity.get_children().iter().any(|child| {
                child.get_kind() == EntityKind::AnnotateAttr
                    && child.get_display_name().unwrap() == EXPOSE_NAME
            })
    }

    fn marked_as_hidden(entity: &Entity) -> bool {
        entity.has_attributes()
            && entity.get_children().iter().any(|child| {
                child.get_kind() == EntityKind::AnnotateAttr
                    && child.get_display_name().unwrap() == HIDE_NAME
            })
    }

    fn parse_class(class: &Entity) -> Option<Class> {
        if !Self::marked_as_exposed(class) {
            return None;
        }
        let mut class_info = Class::default();

        class_info.name = class.get_display_name().unwrap();
        class_info.size = class.get_type().unwrap().get_sizeof().unwrap();
        class_info.alignment = class.get_type().unwrap().get_alignof().unwrap();
        class_info.brief = class.get_comment_brief().unwrap_or_default();
        // get methods
        let methods: Vec<Entity> = class
            .get_children()
            .into_iter()
            .filter(|child| child.get_kind() == EntityKind::Method)
            .collect();

        for method in methods.iter() {
            if let Some(method) = Self::parse_method(method) {
                class_info.methods.push(method);
            }
        }

        let members: Vec<Entity> = class
            .get_children()
            .into_iter()
            .filter(|child| child.get_kind() == EntityKind::FieldDecl)
            .collect();

        for member in members.iter() {
            if let Some(member) = Self::parse_member(member, class) {
                class_info.members.push(member);
            }
        }
        // apparently they are not of type FieldDecl... we also have to remove the static methods from list
        // as they are instead considered of type Method and can be successfully parsed with
        // the same method of the non static methods.
        let static_members: Vec<Entity> = class
            .get_children()
            .into_iter()
            .filter(|child| {
                child.get_storage_class().unwrap_or(StorageClass::Auto) == StorageClass::Static
                    && !child.is_static_method()
            })
            .collect();

        for member in static_members.iter() {
            if let Some(member) = Self::parse_static_member(member, class) {
                class_info.members.push(member);
            }
        }

        Some(class_info)
    }

    fn parse_struct(st: &Entity) -> Option<Struct> {
        if let Some(class) = Self::parse_class(st) {
            return Some(Struct::from_class(class));
        }
        None
    }

    fn parse_enum(en: &Entity) -> Option<Enum> {
        if !Self::marked_as_exposed(en) {
            return None;
        }
        let mut enum_info = Enum::default();
        enum_info.name = en.get_display_name().unwrap();
        enum_info.underlying_type = en.get_enum_underlying_type().unwrap().get_display_name();
        enum_info.brief = en.get_comment_brief().unwrap_or_default();
        let values: Vec<Entity> = en
            .get_children()
            .into_iter()
            .filter(|child| child.get_kind() == EntityKind::EnumConstantDecl)
            .collect();
        if !values.is_empty() {
            enum_info.enumerators = values
                .iter()
                .map(|enumerator| Enumerator {
                    name: enumerator.get_display_name().unwrap(),
                    s_value: enumerator.get_enum_constant_value().unwrap().0,
                    u_value: enumerator.get_enum_constant_value().unwrap().1,
                })
                .collect();
        }

        Some(enum_info)
    }

    fn parse_func(func: &Entity) -> Option<Function> {
        if !Self::marked_as_exposed(func) {
            return None;
        }
        let mut func_info = Function::default();
        func_info.name = func.get_display_name().unwrap();
        func_info.is_const = func.is_const_method();
        func_info.is_static = func.is_static_method();
        func_info.return_type = func.get_result_type().unwrap().get_display_name();
        func_info.brief = func.get_comment_brief().unwrap_or("".to_string());
        if let Some(args) = func.get_arguments() {
            func_info.args = Self::parse_args(args);
        };

        Some(func_info)
    }

    fn parse_variable(var: &Entity) -> Option<Variable> {
        if !Self::marked_as_exposed(var) {
            return None;
        }
        let mut var_info = Variable::default();
        var_info.name = var.get_display_name().unwrap_or_default();
        var_info.var_type = var.get_type().unwrap().get_display_name();
        var_info.is_const = var.get_type().unwrap().is_const_qualified();
        var_info.is_static =
            var.get_storage_class().unwrap_or(StorageClass::Auto) == StorageClass::Static;
        var_info.brief = var.get_comment_brief().unwrap_or_default();

        Some(var_info)
    }

    fn parse_method(method: &Entity) -> Option<Method> {
        if Self::marked_as_hidden(method) {
            return None;
        }
        let mut method_info = Method::default();
        method_info.name = method.get_display_name().unwrap();
        method_info.access = match method.get_accessibility().unwrap() {
            Accessibility::Public => AccessModifier::Public,
            Accessibility::Protected => AccessModifier::Protected,
            Accessibility::Private => AccessModifier::Private,
        };
        method_info.return_type = method.get_result_type().unwrap().get_display_name();
        method_info.is_const = method.is_const_method();
        method_info.is_virtual = method.is_virtual_method();
        method_info.is_pure_virtual = method.is_pure_virtual_method();
        method_info.is_static = method.is_static_method();
        method_info.brief = method.get_comment_brief().unwrap_or_default();

        if let Some(args) = method.get_arguments() {
            method_info.args.reserve(args.len());
            method_info.args = Self::parse_args(args)
        }
        Some(method_info)
    }

    fn parse_args(args: Vec<Entity>) -> Vec<Argument> {
        args.iter()
            .map(|arg| {
                let mut arg_info = Argument::default();
                arg_info.name = arg.get_display_name().unwrap_or_default();
                arg_info.var_type = arg.get_type().unwrap().get_display_name();
                arg_info.is_const = arg.get_type().unwrap().is_const_qualified();
                arg_info
            })
            .collect()
    }

    fn parse_member(member: &Entity, owner: &Entity) -> Option<Member> {
        if Self::marked_as_hidden(member) {
            return None;
        }
        let mut member_info = Member::default();
        member_info.name = member.get_display_name().unwrap();
        member_info.is_const = member.get_type().unwrap().is_const_qualified();
        member_info.access = match member.get_accessibility().unwrap() {
            Accessibility::Public => AccessModifier::Public,
            Accessibility::Protected => AccessModifier::Protected,
            Accessibility::Private => AccessModifier::Private,
        };
        member_info.var_type = member.get_type().unwrap().get_display_name();
        // remove the `const` if present
        if let Some(stripped) = member_info.var_type.strip_prefix("const ") {
            member_info.var_type = stripped.to_owned();
        };

        member_info.offset = owner
            .get_type()
            .unwrap()
            .get_offsetof(&member.get_name().unwrap())
            .unwrap_or(0)
            / 8;

            member_info.brief = member.get_comment_brief().unwrap_or_default();

        Some(member_info)
    }

    fn parse_static_member(member: &Entity, owner: &Entity) -> Option<Member> {
        let option = Self::parse_member(member, owner);
        if let Some(mut member) = option {
            member.is_static = true;
            Some(member)
        } else {
            None
        }
    }
}

#[cfg(test)]
mod tests {
    use super::HeaderParser;

    #[test]
    fn parse_test() {
        let parser = HeaderParser::new();
        let test_header = parser.parse_header_collection(&["src/header_tool/test_header.h"]);
        println!("{test_header:#?}")
    }
}
