# Bake

A lightweight version of the Unix utility [make](<https://en.wikipedia.org/wiki/Make_(software)>) with support for project dependencies that are web-based resources.

This is a past project for UWA's Systems Programming unit, refer to the original project specification [here](https://teaching.csse.uwa.edu.au/units/CITS2002/past-projects/p2018/summary.php).

## Setup

```
cargo build --release
```

## Usage

```
target/release/bake [OPTIONS] [TARGET_NAME]
```

A sample project has been included which can be built using bake

```
target/release/bake -C example
```

## License

[GNU General Public License v3.0](https://choosealicense.com/licenses/gpl-3.0/)
