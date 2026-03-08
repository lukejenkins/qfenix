# QFenix

[![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

An enhanced fork of [linux-msm/qdl](https://github.com/linux-msm/qdl) — a Swiss Army knife
for Qualcomm-based modems and devices. Flash firmware, read/write NV items, browse EFS,
send/receive SMS, interactive AT console, and more — all from a single binary.

## Download

Pre-built static binaries for Linux (x64, arm64, armv7), macOS (universal), and
Windows (x64, x86, arm64) are available under
[Releases](https://github.com/iamromulan/qfenix/releases).

## Quick Start

```bash
# Flash firmware (auto-detect programmer, XMLs, and storage type)
qfenix flash /path/to/firmware/

# List connected devices (EDL, DIAG, AT, ADB, PCIe)
qfenix list

# Backup EFS to XQCN (default, QPST-compatible)
qfenix efsbackup -o backup.xqcn

# Interactive AT console
qfenix atconsole

# Send an SMS
qfenix smssend +15551234567 "Hello from qfenix"

# Read all partitions save to file and make xml (-L /loader/search/path)
qfenix readall -L ./ -o ./
```

## Key Features

- **Auto firmware detection** — `-F` recursively finds programmer ELF, rawprogram/patch XMLs, and detects storage type from any directory layout and -L is used to search for the loader automatically from a starting path.
- **Full DIAG protocol** — NV read/write, complete EFS management, backup/restore to XQCN or TAR
- **SMS & USSD** — Send/receive SMS, delete messages, send USSD queries over AT commands
- **AT console** — Interactive terminal for AT commands with real-time URC display (Linux, macOS, Windows)
- **PCIe/MHI transport** — Flash PCIe modems (T99W175, T99W373, T99W640, etc.) on Linux and Windows
- **DIAG-to-EDL auto-switch** — Detects DIAG mode and switches to EDL automatically
- **GPT operations** — Print partitions, A/B slots, read/erase/dump partitions by label
- **Wide device support** — Quectel, Sierra, Telit, Fibocom, Foxconn/Dell, Simcom, MeiG, and more
- **Document-order XML execution** — Operations run in XML order, enabling backup-erase-flash workflows
- **Cross-platform** — Linux, macOS, and Windows from a single codebase

---

## Common Workflows

### Flash firmware

```bash
# Auto-detect everything from a firmware directory
qfenix -F /path/to/firmware/

# Traditional mode
qfenix prog_firehose_ddr.elf rawprogram*.xml patch*.xml

# Target a specific device
qfenix --serial=0AA94EFD -F /path/to/firmware/       # by USB serial
qfenix --serial=COM49 -F /path/to/firmware/           # by COM port (Windows)

# PCIe modem
qfenix --pcie -F /path/to/firmware/

# Erase-all workflow
qfenix flash -e -F /path/to/firmware/
```

The device must be in EDL mode. QFenix auto-switches from DIAG to EDL
(disable with `--no-auto-edl`).

### EFS backup & restore

```bash
# Backup to XQCN (default — includes NV items, QPST-compatible)
qfenix efsbackup
qfenix efsbackup -o my_backup.xqcn

# Backup to TAR (probe-based path coverage)
qfenix efsbackup -t -o backup.tar
qfenix efsbackup -t --quick -o quick.tar     # tree walk only, faster

# Restore (format auto-detected)
qfenix efsrestore backup.xqcn
qfenix efsrestore backup.tar

# Offline format conversion (no device needed)
qfenix xqcn2tar backup.xqcn output.tar
qfenix tar2xqcn backup.tar output.xqcn
```

### NV items & EFS

```bash
# NV items
qfenix nvread 0                              # read NV item 0 (ESN)
qfenix nvread 6828 --index=0                 # with subscription index
qfenix nvwrite 6828 0102030405               # write hex data

# EFS filesystem
qfenix efsls /                               # list directory
qfenix efspull /nv/item_files/file.bin ./     # download file
qfenix efspush ./file.bin /nv/item_files/     # upload file
qfenix efsrm -r /custapp/mydir               # recursive delete
```

### SMS & AT commands

```bash
# SMS
qfenix smssend +15551234567 "Hello"          # send SMS
qfenix smsread                               # list received SMS
qfenix smsread -j                            # JSON output
qfenix smsrm all                             # delete all messages
qfenix smsstatus                             # storage status

# USSD
qfenix ussd "*#06#"                          # query IMEI

# Raw AT command
qfenix atcmd AT+COPS?                        # single command

# Interactive AT console
qfenix atconsole                             # auto-detect port
qfenix atconsole -p COM9                     # specify port
```

### Partitions & GPT

```bash
# Inspect
qfenix printgpt prog_firehose_ddr.elf
qfenix getslot prog_firehose_ddr.elf
qfenix storageinfo prog_firehose_ddr.elf

# Read partitions
qfenix read modem -L /path/to/firmware/
qfenix readall -L /path/to/firmware/ -o /path/to/output/

# Erase
qfenix erase modemst1 modemst2 -L /path/to/firmware/
qfenix eraseall -L /path/to/firmware/
```

---

## Full Command Reference

Every subcommand supports `--help` for detailed usage.

### Flashing & Storage

These commands require a programmer ELF and a device in EDL mode.

| Command | Description |
|---------|-------------|
| *(default)* / `flash` | Flash firmware via Firehose protocol |
| `printgpt` | Print GPT partition tables (supports `--make-xml` for XML generation) |
| `storageinfo` | Query storage hardware info |
| `reset` | Reset, power off, or reboot into EDL |
| `getslot` | Show active A/B slot |
| `setslot` | Set active A/B slot |
| `read` | Read partition(s) by label |
| `readall` | Dump all partitions (supports `--single-file` for full storage dump) |
| `erase` | Erase partition(s) by label or raw sector range |
| `eraseall` | Erase all partitions |

### DIAG Operations

Work directly over the DIAG serial port — no programmer or EDL mode needed.

| Command | Description |
|---------|-------------|
| `diag2edl` | Switch device from DIAG to EDL mode |
| `nvread` | Read NV item (supports `--index` for subscriptions) |
| `nvwrite` | Write NV item from hex data |
| `efsls` | List EFS directory |
| `efspull` | Download file from EFS |
| `efspush` | Upload file to EFS |
| `efsrm` | Delete file or directory (`-r` for recursive) |
| `efsstat` | Show file/directory metadata |
| `efsmkdir` | Create EFS directory |
| `efschmod` | Change permissions |
| `efsln` | Create symlink |
| `efsrl` | Read symlink target |
| `efsdump` | Dump EFS factory image |
| `efsbackup` | Backup EFS to XQCN (default) or TAR (`-t`) |
| `efsrestore` | Restore EFS from XQCN or TAR (auto-detected) |
| `xqcn2tar` | Convert XQCN to TAR (offline, no device) |
| `tar2xqcn` | Convert TAR to XQCN (offline, no device) |

### AT / SMS / USSD

Work over the AT serial port — no programmer or EDL mode needed.

| Command | Description |
|---------|-------------|
| `atcmd` | Send a raw AT command |
| `atconsole` | Interactive AT console with real-time URC display |
| `smssend` | Send SMS via PDU mode |
| `smsread` | List/read SMS messages (`-j` for JSON, `-r` for raw PDU) |
| `smsrm` | Delete SMS by index or all |
| `smsstatus` | Query SMS storage status |
| `ussd` | Send USSD/USSI query |

### Other

| Command | Description |
|---------|-------------|
| `list` | List connected EDL, DIAG, AT, ADB, and PCIe devices |
| `ramdump` | Extract RAM dumps via Sahara |
| `ks` | Keystore/Sahara over serial device nodes |

---

## Supported Devices

QFenix includes a VID/PID database for automatic detection of:

- **Qualcomm** reference designs (SDX55, SDX65, SDX72, SDX75)
- **Quectel** (EM05, EM06, EM12, EM060K, EM120K, RM520N, RM255C, RG650V, etc.)
- **Sierra Wireless** (EM74xx, EM9190, EM9191, EM9291)
- **Telit** (LM960, FN980, FN990, FM990, LE910C4)
- **Fibocom** (FM150, FM160)
- **Foxconn/Dell** (DW5820e, DW5930e, DW5931e, DW5934e / T99W175, T99W373, T99W640)
- **Simcom** (SIM8200EA, SIM8380G)
- **MeiG Smart** (SRM825, SRM930)
- **Sony, ZTE, LG, Netgear, Huawei** EDL/DIAG devices

PCIe modems (MHI-based) are detected via friendly name matching on Windows.

## Build from Source

```bash
# Linux
sudo apt install libxml2-dev libusb-1.0-0-dev help2man
make

# macOS (Homebrew)
brew install libxml2 pkg-config libusb help2man
make

# macOS (MacPorts)
sudo port install libxml2 pkgconfig libusb help2man
make

# Windows (MSYS2 MinGW64 terminal)
pacman -S base-devel git mingw-w64-x86_64-{gcc,make,pkg-config,libusb,libxml2}
make

# Run tests
make tests
```

## License

BSD 3-Clause. See [LICENSE](LICENSE). Fork of [linux-msm/qdl](https://github.com/linux-msm/qdl).
