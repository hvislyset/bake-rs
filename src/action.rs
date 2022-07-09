use std::{env, fmt};
use std::{io::Error, process::Command};

#[derive(Debug, Clone)]
pub struct Action {
    pub command: String,
    pub modifier: Option<char>,
    pub ignore: bool,
    pub stdout: bool,
    pub silent: bool,
}

impl Action {
    pub fn new(
        command: String,
        modifier: Option<char>,
        ignore: bool,
        stdout: bool,
        silent: bool,
    ) -> Self {
        Self {
            command,
            modifier,
            ignore,
            stdout,
            silent,
        }
    }

    pub fn execute(&self) -> Result<bool, Error> {
        let silent = self.modifier == Some('@') || self.silent;
        let always_success = self.modifier == Some('-') || self.ignore;

        if self.stdout {
            println!("{}", self.command);

            return Ok(true);
        }

        if !silent {
            println!("{}", self.command);
        }

        let output =
            match Command::new(env::var("SHELL").unwrap_or_else(|_| "/bin/bash".to_string()))
                .args(vec!["-c", &self.command])
                .output()
            {
                Ok(output) => output,
                Err(_) => {
                    if always_success {
                        return Ok(true);
                    }

                    return Ok(false);
                }
            };

        if output.status.success() {
            if !output.stdout.is_empty() {
                let stdout = String::from_utf8(output.stdout).unwrap();
                println!("{}", stdout);
            }

            Ok(true)
        } else {
            if always_success {
                return Ok(true);
            }

            println!("{}", String::from_utf8(output.stderr).unwrap());

            Ok(false)
        }
    }
}

impl fmt::Display for Action {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}{}", self.modifier.unwrap_or(' '), self.command)
    }
}
