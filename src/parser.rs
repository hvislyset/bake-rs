use std::collections::HashMap;
use std::{env, fmt};

use regex::Regex;

use crate::action::Action;
use crate::error::BakeError;
use crate::ffi;
use crate::target::{Dependency, Target};
use crate::variable::Variable;

const RESERVED_IDENTIFIERS: &[&str; 4] = &["PID", "PPID", "PWD", "RAND"];

pub struct Parser {
    variables: HashMap<String, String>,
    current_target: Option<Target>,
    default_target: Option<String>,
    is_target: bool,
}

#[derive(Debug)]
pub struct ParserResult {
    pub targets_map: HashMap<String, Target>,
    pub variables: HashMap<String, String>,
    pub default_target: String,
}

impl Parser {
    pub fn new(default_target: Option<String>) -> Self {
        Self {
            variables: HashMap::new(),
            current_target: None,
            default_target,
            is_target: false,
        }
    }

    pub fn parse(
        &mut self,
        lines: Vec<String>,
        ignore: bool,
        stdout: bool,
        silent: bool,
    ) -> Result<ParserResult, BakeError> {
        let mut targets: HashMap<String, Target> = HashMap::new();

        for line in lines {
            if line.contains('=') || line.contains(":=") {
                let variable = self.handle_variable(&self.handle_variable_expansion(&line)?)?;

                self.variables.insert(variable.key, variable.value);

                continue;
            }

            if line.contains(':') {
                if self.is_target {
                    self.is_target = false;

                    let current_target = self.current_target.as_ref().ok_or_else(|| {
                        BakeError::Parser("Couldn't retrieve current target".to_string())
                    })?;
                    let current_target_name = &current_target.name;

                    targets.insert(current_target_name.to_string(), current_target.clone());
                }

                let target = self.handle_target(&line)?;

                if self.default_target.is_none() {
                    self.default_target = Some(target.name.to_string())
                }

                self.current_target = Some(target);
                self.is_target = true;

                continue;
            }

            if line.starts_with('\t') {
                if !self.is_target {
                    return Err(BakeError::Parser(
                        "Tab detected outside of target".to_string(),
                    ));
                }

                let act = self.handle_action(
                    &self.handle_variable_expansion(&line)?,
                    ignore,
                    stdout,
                    silent,
                )?;

                if let Some(ref mut current_target) = self.current_target {
                    current_target.actions.push(act);
                }

                continue;
            }
        }

        if let Some(ref current_target) = self.current_target {
            targets.insert(current_target.name.to_string(), current_target.clone());
        }

        Ok(ParserResult::new(
            targets,
            self.variables.clone(),
            self.default_target
                .as_ref()
                .unwrap_or(&"".to_string())
                .to_string(),
        ))
    }

    fn handle_variable_expansion(&self, target: &str) -> Result<String, BakeError> {
        let re =
            Regex::new(r"\$\((\s*[a-zA-Z]*\s*)\)*").expect("Regular expression failed to compile");

        if re.is_match(target) {
            let mut new_line = target.to_string();

            for capture in re.captures_iter(target) {
                new_line =
                    new_line.replace(&capture[0], &self.handle_variable_lookup(&capture[1])?);
            }

            return Ok(new_line);
        }

        Ok(target.to_string())
    }

    fn handle_variable_lookup(&self, key: &str) -> Result<String, BakeError> {
        // Check to see if the variable is user defined
        if let Some(value) = self.variables.get(key) {
            return Ok(value.to_string());
        }

        // If the variable is not defined in the file, check the process environment
        if let Ok(value) = env::var(key) {
            return Ok(value);
        }

        // If the variable is a reserved identifier, call their respective libc functions
        if RESERVED_IDENTIFIERS.contains(&key) {
            return Ok(unsafe {
                ffi::call_libc(key).map_err(|_| {
                    BakeError::Parser("Error retrieving variable definition".to_string())
                })?
            });
        }

        // If the variable doesn't exist, return an empty string
        Ok("".to_string())
    }

    fn handle_variable(&self, line: &str) -> Result<Variable, BakeError> {
        let (key, value) = {
            if line.contains(":=") {
                let (key, value) = line.split_once(":=").ok_or_else(|| {
                    BakeError::Parser("Error delimiting variable definition line".to_string())
                })?;

                (key, value)
            } else {
                let (key, value) = line.split_once('=').ok_or_else(|| {
                    BakeError::Parser("Error delimiting variable definition line".to_string())
                })?;

                (key, value)
            }
        };

        Ok(Variable::new(
            key.trim().to_string(),
            value.trim().to_string(),
        ))
    }

    fn handle_target(&self, line: &str) -> Result<Target, BakeError> {
        let (target, dependencies) = line.split_once(':').ok_or_else(|| {
            BakeError::Parser("Error delimiting target definition line".to_string())
        })?;

        let target_name = self.handle_variable_expansion(target)?.trim().to_string();
        let dependencies = self.handle_dependencies(&self.handle_variable_expansion(dependencies)?);

        Ok(Target::new(target_name, dependencies, Vec::new()))
    }

    fn handle_dependencies(&self, deps: &str) -> Option<Vec<Dependency>> {
        let dependencies: Vec<&str> = deps.split_whitespace().collect();

        if !dependencies.is_empty() {
            Some(
                dependencies
                    .iter()
                    .map(|dep| self.handle_dependency(dep))
                    .collect(),
            )
        } else {
            None
        }
    }

    fn handle_dependency(&self, dependency: &str) -> Dependency {
        if dependency.starts_with("http://")
            || dependency.starts_with("https://")
            || dependency.starts_with("file://")
        {
            Dependency::Url(dependency.to_string())
        } else {
            Dependency::File(dependency.to_string())
        }
    }

    fn handle_action(
        &self,
        action: &str,
        ignore: bool,
        stdout: bool,
        silent: bool,
    ) -> Result<Action, BakeError> {
        let action_line = action.trim();

        let (modifier, rest) = {
            if action_line.starts_with('@') {
                let (_, rest) = action_line.split_once('@').ok_or_else(|| {
                    BakeError::Parser("Error delimiting action line modifier".to_string())
                })?;

                (Some('@'), rest)
            } else if action_line.starts_with('-') {
                let (_, rest) = action_line.split_once('-').ok_or_else(|| {
                    BakeError::Parser("Error delimiting action line modifier".to_string())
                })?;
                (Some('-'), rest)
            } else {
                (None, action_line)
            }
        };

        Ok(Action::new(
            rest.to_string(),
            modifier,
            ignore,
            stdout,
            silent,
        ))
    }
}

impl ParserResult {
    fn new(
        targets_map: HashMap<String, Target>,
        variables: HashMap<String, String>,
        default_target: String,
    ) -> Self {
        Self {
            targets_map,
            variables,
            default_target,
        }
    }
}

impl fmt::Display for ParserResult {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let mut targets = String::new();
        let mut variables = String::new();

        for (target_name, target) in self.targets_map.iter() {
            let dependencies = {
                if target.dependencies.is_some() {
                    let deps: Vec<String> = target
                        .dependencies
                        .as_ref()
                        .unwrap()
                        .iter()
                        .map(|dep| dep.to_string())
                        .collect();

                    deps.join(" ")
                } else {
                    "".to_string()
                }
            };

            targets.push_str(format!("\t{}: {}\n", target_name, dependencies).as_str());

            for action in &target.actions {
                targets.push_str(format!("\t\t{}\n", action).as_str())
            }
            targets.push('\n');
        }

        for (variable_name, variable) in self.variables.iter() {
            variables.push_str(format!("\t{}: {}\n", variable_name, variable).as_str())
        }

        write!(f, "TARGETS:\n{}\nVARIABLES:\n{}", targets, variables)
    }
}
