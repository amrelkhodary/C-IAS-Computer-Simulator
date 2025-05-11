# IAS Computer Simulator

## Introduction

The IAS Computer, developed at the Institute for Advanced Study in Princeton (1945-1951) under John von Neumann's leadership, established fundamental concepts that shape computing to this day. This simulator lets you write and execute programs as they would have run on this groundbreaking machine, providing insight into computing's origins.

## Installation

```bash
# Clone the repository
git clone https://github.com/amrelkhodary/C-IAS-Computer-Simulator.git
cd C-IAS-Computer-Simulator

# Run the installation script
chmod +x install.sh
./install.sh
```

The installer will:
- Compile the source code and install the binary to `/usr/local/bin` (requires sudo)
- Allow you to run IAS programs using the `ias` command

## Running Programs

```bash
ias path/to/your/program.ias
```

## IAS Program Structure

IAS programs follow a simple, historically-informed structure:

```
# This is a comment

data:
# address, value
900, 10    # First number to add
901, 20    # Second number to add
902, 0     # Result location

program:
# opcode address
LOADMX 900    # Load first value into accumulator
ADDMX 901     # Add second value
STORMX 902    # Store result
```

## The IAS Instruction Set

The instruction set reflects the elegance and fundamental nature of early computing:

| Instruction | Description |
|-------------|-------------|
| `LOADMQ` | Load the Multiplier-Quotient register |
| `LOADMQMX` | Load MQ with memory contents |
| `STORMX` | Store accumulator contents to memory |
| `LOADMX` | Load accumulator with memory contents |
| `LOADNMX` | Load accumulator with negative of memory contents |
| `LOADAMX` | Load accumulator with absolute value of memory contents |
| `LOADNAMX` | Load accumulator with negative of absolute value of memory contents |
| `JUMP` | Jump to left instruction at specified address |
| `CJUMP` | Conditional jump if accumulator is non-negative |
| `ADDMX` | Add memory contents to accumulator |
| `ADDAMX` | Add absolute value of memory contents to accumulator |
| `SUBMX` | Subtract memory contents from accumulator |
| `SUBAMX` | Subtract absolute value of memory contents from accumulator |
| `MULMX` | Multiply accumulator by memory contents |
| `DIVMX` | Divide accumulator by memory contents |
| `LSH` | Left shift accumulator |
| `RSH` | Right shift accumulator |
| `STORAMX` | Store address register at memory location |

## Learning Through Examples

### Example 1: Addition
```
# Simple addition program
data:
900, 42    # First number
901, 27    # Second number
902, 0     # Result storage

program:
LOADMX 900    # Load first number
ADDMX 901     # Add second number
STORMX 902    # Store result
```

### Example 2: Computing a Sum from 1 to N
```
# Calculate sum of integers from 1 to N
data:
900, 10    # N = 10
901, 0     # Sum (initialized to 0)
902, 1     # Constant 1 (increment)
903, 1     # Counter (starts at 1)

program:
# Initialize
LOADMX 901    # Load initial sum (0)
  
# Main loop
LOOP:
  ADDMX 903   # Add current counter value
  STORMX 901  # Store running sum
  
  # Increment counter
  LOADMX 903
  ADDMX 902   # Add 1
  STORMX 903  # Update counter
  
  # Check if counter <= N
  SUBMX 900
  CJUMP END   # If counter > N, exit
  JUMP LOOP   # Otherwise, continue

END:
  # Result is in memory location 901
```

### Example 3: Finding Maximum Value
```
# Find maximum of two values
data:
900, 15    # First value
901, 27    # Second value
902, 0     # Will contain maximum

program:
LOADMX 900       # Load first value
SUBMX 901        # Subtract second value
CJUMP FIRST_BIGGER  # Jump if result is non-negative

# Second is bigger
LOADMX 901
STORMX 902
JUMP END

FIRST_BIGGER:
LOADMX 900
STORMX 902

END:
# Maximum is now in memory location 902
```

## Historical Context

The IAS computer represents a pivotal moment in computing history. Its architecture directly influenced IBM's first scientific computer and indirectly shaped virtually all computers that followed. Key innovations included:

- Binary representation of data and instructions
- Stored-program concept (program as data)
- Memory with a single address space for both instructions and data
- Conditional execution flow

By using this simulator, you're experiencing the concepts that formed the foundation of our digital world.

## Troubleshooting

For issues or assistance:
- Check execution logs in `$HOME/IAS_LOGS`
- Verify program syntax follows the format shown in examples
- Contact: amrelkhodarybusiness@gmail.com

## Further Resources

- ["First Draft of a Report on the EDVAC"](https://en.wikipedia.org/wiki/First_Draft_of_a_Report_on_the_EDVAC) - Von Neumann's seminal paper
- [The IAS Computer at the Institute for Advanced Study](https://www.ias.edu/electronic-computer-project) - Historical background

---
