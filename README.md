# EmbeddedPUS
Minimal ECSS Packet Utilisation Standard (PUS) protocol implementation in C for small-scale space missions.


## Standards Compliance

- **ECSS-E-ST-70-41C**: Packet Utilisation Standard (PUS)

## Features

### Core Protocol Implementation


### Design Principles


## Project Structure

```
EmbeddedSpacePacket/
├── include/
│   └── 
├── src/
│   └── 
├── examples/
│   └── 
├── tests/
│   ├── cunit.h              # Minimal test framework
│   └── unit_tests.c         # Unit tests
├── scripts/
│   └── coverage_html.sh     # Coverage report
├── build/                   # Build artifacts 
├── Makefile
└── README.md
```

## Building

### Build Everything

```bash
make
```

Builds the static library, the example binary and the test binary in `build/`.

### Build Library Only

```bash
make lib
# Produces: build/
```

### Build Example

```bash
make example
./build/examples/example
```

### Run Tests

```bash
make ctest
```

### Coverage (HTML)

Requires `gcovr` installed in your system:

```bash
sudo apt install gcovr
```

Generate coverage report:

```bash
make coverage-html
```

Output report:

```bash
build/coverage/index.html
```

### Clean

```bash
make clean
```

## Quick Start

### Step 1

```c

```

### Step 2

```c

```

### Step X


## API Reference

### Lifecycle

```c

```

### Building a Packet

```c

```

### Utilities

```c

```

### Types

```c

```

## Memory Usage (Estimated)

- **Library (stripped)**: 
- **Serialization buffer**:
- **No heap usage**: all allocations are caller-supplied

## CCSDS XXX — Notes

## Limitations

## References

## License

See LICENSE file.