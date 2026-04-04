# Pandora

**Pandora** is a lightweight package management and build system designed to simplify the installation, management, and compilation of software packages in AtlasLinux's environment. It supports dependency resolution, automatic builds, and custom compiler wrappers for controlled compilation.

## Features

* Install, remove, update, and list packages.
* Automatically handles package dependencies.
* Builds packages using custom compiler wrappers.
* Creates isolated environment directories for include, lib, and etc.
* Simple CLI interface.

## Installation

Clone the repository and build Pandora:

```bash
git clone https://github.com/AtlasLinux/protodora.git
cd protodora
make
```

This will:

* Compile the main Pandora binary.
* Build the compiler wrappers `pcc` and `pc++`.
* Create necessary directories (`include/`, `lib/`, `etc/`) in the Pandora environment.
* Configure your shell environment by updating `$PATH` and `$PANDORA`.

## Usage

```bash
pandora <command> [args]
```

Available commands:

* `install <package>` – Installs a package and its dependencies.
* `remove <package>` – Removes an installed package.
* `update <package>` – Reinstalls and updates a package.
* `list [pattern]` – Lists all installed packages or filters by a pattern.
* `help` – Displays usage instructions.

## Directory Structure

Pandora maintains an isolated environment under `$PANDORA` (default: `~/.PANDORA`):

```
.PANDORA/
├── bin/       # Pandora executable and compiler wrappers.
├── include/   # Header files for compiled packages.
├── lib/       # Library files for compiled packages.
├── etc/       # Configuration files.
```

## Compiler Wrappers

Pandora provides `pcc` and `pc++` wrappers that automatically inject include paths and libraries from the Pandora environment:

```bash
pcc <source_files>
pc++ <source_files>
```

These wrappers ensure packages are built consistently with Pandora-managed dependencies.
