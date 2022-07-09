use clap::Parser;

#[derive(Parser, Debug)]
#[clap(author, version, about, long_about = Some("A lightweight version of the Unix utility make with support for project dependencies that are web-based resources."))]
pub struct Args {
    #[clap(value_parser)]
    /// Target to build. Defaults to the first defined target in the Bakefile if not provided
    pub target_name: Option<String>,
    #[clap(short = 'C')]
    /// Before opening and reading the Bakefile, change directory to the indicated directory
    pub directory_name: Option<String>,
    #[clap(short = 'f', default_value("Bakefile"))]
    /// Instead of reading from the default Bakefile, read from the indicated file
    pub filename: String,
    #[clap(short = 'i')]
    /// Ignore unsuccessful termination of actions; continue executing a target's actions even if any fail
    pub ignore: bool,
    #[clap(short = 'n')]
    /// Print each action to stdout before it is executed, does not execute the actions. Assumes that each action would have executed successfully
    pub stdout: bool,
    #[clap(short = 'p')]
    /// After reading the Bakefile, print out its internal representation
    pub print_internal: bool,
    #[clap(short = 's')]
    /// Execute silently, do not print each action to stdout before it is executed
    pub silent: bool,
}
