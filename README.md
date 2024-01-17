# 8086 Emulator
## Brief
An emulator for the x86-16 (8086) processor, implemented in C.

## Description of Project State
- Limited CLI support.
  - Filename can be argued.
  - Disassembly flag can be specified.
- Limited disassembly support.
  - What can be disassembled:
    - register-to-register mov instructions;
    - register-to-memory mov instructions, including register-to-memory mov instructions with 8-bit or 16-bit displacement;
    - memory-to-register mov instructions, including memory-to-register mov instructions with 8-bit or 16-bit displacement;
    - direct addressing mov instructions;
    - immediate-to-register mov instructions, both 8-bit immediate-to-register and 16-bit immediate-to-register.
- No emulation support.
