# lingmo-debinstaller

A graphical Debian package installer for LingmoOS.

## Features

- **Install** — open any `.deb` file, inspect its full metadata (package name,
  version, architecture, maintainer, installed size, section, dependencies,
  description) and install it with one click.
- **Manage** — browse every package dpkg knows as installed, search by name or
  description, remove packages, and update individual packages to the latest
  available version.
- **Rich progress** — an indeterminate progress bar and busy indicator run
  during every operation; the latest output line from dpkg/apt is shown in
  real time.
- **Confirmation dialog** — removal asks for confirmation before proceeding.
- **Command-line** — pass a `.deb` path as an argument and it opens directly
  in the Install view.

## Requirements

- LingmoSDK
- `dpkg` and `dpkg-deb` (standard on any Debian/Ubuntu system)
- `apt-get` (for update and remove)
- `pkexec` / polkit (for privilege escalation)

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

## Usage

```bash
lingmo-debinstaller            # open the app
lingmo-debinstaller file.deb   # open with a specific package pre-loaded
```

## License

GPL-3.0-or-later — © 2026 LingmoOS Contributors
