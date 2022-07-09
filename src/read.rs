use std::{
    fs::File,
    io::{BufRead, BufReader, Error},
};

pub fn read_bakefile(path: &str) -> Result<Vec<String>, Error> {
    let mut lines = Vec::new();

    let file = File::open(path)?;
    let reader = BufReader::new(file);
    let mut buf = String::new();

    for line in reader.lines() {
        let line_buf = line?;

        if line_buf.is_empty() || line_buf.starts_with('#') {
            continue;
        }

        buf.push_str(&line_buf);

        if line_buf.ends_with('\\') {
            buf.pop();

            continue;
        }

        lines.push(buf.clone());
        buf.clear();
    }

    Ok(lines)
}
