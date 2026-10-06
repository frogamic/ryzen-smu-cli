# Ryzen SMU CLI

`rsmuctl`

A small command-line tool for reading and setting per-core Curve Optimizer voltage offsets on AMD Ryzen CPUs, using the [`ryzen_smu`](https://github.com/amkillam/ryzen_smu) kernel driver and its `libsmu` userspace library.

## Requirements

- A Linux PC with the `ryzen_smu` kernel module loaded.
- Root privileges.
- A supported AMD CPU, currently only Vermeer (Ryzen 5000 series) is tested and supported.
- To build: `gcc`, `make`, `glibc` and the `libsmu` sources. Or just `nix`.

> **Warning:** This interfaces directly with the CPU's System Management Unit. Bad values can cause instability, crashes, data loss or even hardware damage. Use at your own risk, the author takes no responsibility for potential damages that may result from the use or misuse of this tool.

## Usage

```sh
sudo rsmuctl [-v]... [-r] [-o VALUE]...
```

| Option | Description |
| --- | --- |
| `-o`, `--voffset=VALUE` | Set an offset. `VALUE` is either an all-core offset (`-10`) or `CORE:OFFSET` (`3:-25`). May be repeated. |
| `-r`, `--reset` | Reset the offset on all cores. Applied before any other changes. |
| `-v`, `--verbose` | Increase log verbosity. May be repeated. |
| `-?`, `--help` | Show help. |
| `-V`, `--version` | Show version. |

- Offsets are Curve Optimizer values. Negative values undervolt, positive overvolts.
- The usable range is limited by your CPU and firmware, so check the output table to see what was actually applied.
- A per-core offset specified by `core:offset` takes precedence over the all-core offset for that core.
- Each core, the all-core offset, and `--reset` may each be given only once.
- Offsets do not persist across reboots or sleep, so they need to be reapplied each boot - [see here](https://github.com/frogamic/nix-machines/blob/88167cc/modules/amdcpu.nix#L63) for an example systemd oneshot service.
- Run with no options to read the current offsets without changing anything (still requires root to interface with the ryzen_smu kernel module).

### Examples

```sh
# Read current offsets
sudo rsmuctl

# Apply -30 to every core
sudo rsmuctl -o -30

# Apply -10 to every core, with core 3 and 7 overridden to -25
sudo rsmuctl -o -10 -o 3:-25 -o 7:-25

# Reset everything, then apply a new core offset to core 5 only
sudo rsmuctl -r -o 5:-15
```

### Output

After applying any changes, or being run with no changes, `rsmuctl` prints the offset it reads back from each core:

```
AMD Ryzen 7 5800X3D 8-Core Processor (Vermeer), 8 cores/16 threads
Core  voffset
  0     -30
  1     -30
  2     -30
  ...
```

### Exit Status

0 on success, non-zero if any error occurred or invalid input supplied.

### Verbosity

Only error logging is shown by default. More `v`s can be passed, up to 4, for trace level output. Logs go to stderr. The CPU summary and offset table go to stdout.

## Installing

This can be added to your NixOS config using the following minimal module. This will build the cli using the version of `ryzen-smu` to match your current kernel. Double check the version and hash here before using it blindly!

```nix
{ config, pkgs, lib, ... }:
let
  ryzen-smu-cli-src = pkgs.fetchFromGitHub {
    owner = "frogamic";
    repo = "ryzen-smu-cli";
    rev = "0.0.4";
    hash = "sha256-wOecovXSFdzE4XmUQB7glT773LbuoCH73dOXmC8bQMQ=";
  };
  ryzen-smu-cli = pkgs.callPackage "${ryzen-smu-cli-src}/package.nix" {
    linuxPackages = config.boot.kernelPackages;
  };
in
{
  hardware.cpu.amd.ryzen-smu.enable = true;
  environment.systemPackages = [ ryzen-smu-cli ];
}
```

## Building with Nix

The repository is a flake:

```sh
nix build
sudo ./result/bin/rsmuctl
```

The flake also provides development helpers:

```sh
nix run .#format   # clang-format over src/
nix run .#iwyu     # include-what-you-use, via bear
```

### Building without Nix

Checkout `https://github.com/amkillam/ryzen_smu` (or a fork) somewhere local.

```sh
make -C src LIBSMU_DIR=/path/to/ryzen_smu/lib
```

The binary is written to `src/rsmuctl`. `LIBSMU_DIR` defaults to `../ryzen_smu/lib`.

| Variable | Default | Description |
| --- | --- | --- |
| `LIBSMU_DIR` | `../ryzen_smu/lib` | Directory containing `libsmu.c` and `libsmu.h` |
| `VERSION` | `0.0.1` | Version string reported by `--version` |
| `TARGET` | `rsmuctl` | Output binary name |
| `PREFIX` | (none) | Install prefix, used by `make install` |

```sh
sudo make -C src install PREFIX=/usr/local LIBSMU_DIR=/path/to/ryzen_smu/lib
```

## Acknowledgements

- [`ryzen_smu`](https://github.com/leogx9r/ryzen_smu) and the [amkillam fork](https://github.com/amkillam/ryzen_smu) for the kernel driver, `libsmu` and example code.
- [`Ryzen-5800x3d-linux-undervolting`](https://github.com/svenlange2/Ryzen-5800x3d-linux-undervolting) for the inspiration and Python implementation of the mailbox commands this tool is based on.
- The overclock.net community, in particular PJVol, for the original reverse engineering of the PBO2 Curve Optimizer commands.

## AI Usage Statement

All code in this repository was written by hand, or adapted from libsmu example code. An AI (Claude) was used via web chat to search, provide answers and suggestions (which were reviewed and adapted by hand) and check the code once written. This README was also partially AI written because I don't like writing READMEs.

## License

Copyright (C) 2026 Dominic Shelton. Licensed under GPL-3.0-or-later, see LICENSE.
