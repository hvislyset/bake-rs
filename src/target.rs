use std::{collections::HashMap, fmt, fs, path::Path, time::SystemTime};

use curl::easy::Easy;

use crate::{action::Action, error::BakeError};

#[derive(Debug, Clone)]
pub struct Target {
    pub name: String,
    pub dependencies: Option<Vec<Dependency>>,
    pub actions: Vec<Action>,
}

#[derive(Debug, Clone)]
pub enum Dependency {
    File(String),
    Url(String),
}

impl Target {
    pub fn new(name: String, dependencies: Option<Vec<Dependency>>, actions: Vec<Action>) -> Self {
        Self {
            name,
            dependencies,
            actions,
        }
    }

    pub fn process_target(&self, targets: &HashMap<String, Target>) -> Result<(), BakeError> {
        if self.is_outdated(targets)? {
            self.build_target()?;

            return Ok(());
        }

        println!("{} is up to date", self.name);

        Ok(())
    }

    fn is_outdated(&self, targets: &HashMap<String, Target>) -> Result<bool, BakeError> {
        if self.dependencies.is_none() {
            return Ok(true);
        }

        let mut stack = Vec::new();
        let mut target_requires_rebuilding = false;

        stack.push(self);

        while !stack.is_empty() {
            let target = stack.pop().unwrap();

            if target.dependencies.is_none() {
                target_requires_rebuilding = true;
            } else {
                let target_path = Path::new(&target.name);
                let dependencies = target.dependencies.as_ref().unwrap();
                let mut dependency_requires_rebuilding = false;

                for dependency in dependencies {
                    match dependency {
                        Dependency::File(file) => {
                            if targets.contains_key(file) {
                                stack.push(targets.get(file).unwrap());
                            }

                            let file_path = Path::new(file);

                            if file_path.exists() {
                                if !target_path.exists() {
                                    dependency_requires_rebuilding = true;
                                    target_requires_rebuilding = true;
                                } else {
                                    let file_modified =
                                        fs::metadata(file).unwrap().modified().unwrap();
                                    let target_modified =
                                        fs::metadata(&target.name).unwrap().modified().unwrap();

                                    if target_modified < file_modified {
                                        dependency_requires_rebuilding = true;
                                        target_requires_rebuilding = true;
                                    }
                                }
                            } else {
                                dependency_requires_rebuilding = true;
                                target_requires_rebuilding = true;
                            }
                        }
                        Dependency::Url(url) => {
                            if targets.contains_key(url) {
                                stack.push(targets.get(url).unwrap());
                            }

                            let target_path = Path::new(&target.name);

                            if target_path.exists() {
                                let mut handle = Easy::new();

                                handle.url(url)?;
                                handle.nobody(true)?;
                                handle.show_header(true)?;
                                handle.fetch_filetime(true)?;
                                handle.perform()?;

                                let last_modified = handle.filetime()?.unwrap_or(i64::max_value());

                                let target_modified = fs::metadata(target_path)
                                    .unwrap()
                                    .modified()
                                    .unwrap()
                                    .duration_since(SystemTime::UNIX_EPOCH)
                                    .unwrap()
                                    .as_secs()
                                    as i64;

                                if target_modified < last_modified {
                                    dependency_requires_rebuilding = true;
                                    target_requires_rebuilding = true;
                                }
                            } else {
                                dependency_requires_rebuilding = true;
                                target_requires_rebuilding = true;
                            }
                        }
                    }
                }

                if dependency_requires_rebuilding && self.name != target.name {
                    target.build_target()?;
                }
            }
        }

        if target_requires_rebuilding {
            return Ok(true);
        }

        Ok(false)
    }

    fn build_target(&self) -> Result<bool, BakeError> {
        for action in &self.actions {
            let result = action.execute().map_err(|_| {
                BakeError::Target("Could not execute action for target".to_string())
            })?;

            if !result {
                return Ok(false);
            }
        }

        Ok(true)
    }
}

impl fmt::Display for Dependency {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Dependency::File(file) => write!(f, "{}", file),
            Dependency::Url(url) => write!(f, "{}", url),
        }
    }
}
