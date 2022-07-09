mod action;
mod args;
mod error;
mod ffi;
mod parser;
mod read;
mod target;
mod variable;

use std::env;
use std::path::Path;

use anyhow::Result;
use clap::Parser;

use error::BakeError;

fn main() -> Result<()> {
    let args = args::Args::parse();

    if let Some(directory_name) = args.directory_name {
        let root = Path::new(&directory_name);
        env::set_current_dir(&root)
            .map_err(|_| BakeError::IO("No such file or directory".to_string()))?;
    }

    let lines = read::read_bakefile(&args.filename)
        .map_err(|_| BakeError::IO("No Bakefile found".to_string()))?;

    let mut parser = parser::Parser::new(args.target_name);
    let parser_result = parser.parse(lines, args.ignore, args.stdout, args.silent)?;

    if args.print_internal {
        println!("{}", parser_result);

        return Ok(());
    }

    if parser_result.default_target.is_empty() {
        return Ok(());
    }

    let targets = parser_result.targets_map;
    let target = targets.get(&parser_result.default_target).ok_or_else(|| {
        BakeError::Target(format!(
            "No rule to make target: {}",
            &parser_result.default_target
        ))
    })?;

    Ok(target.process_target(&targets)?)
}
