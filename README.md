# Performance-Aware-Programming
Solutions to homework assignments of the Performance Aware Programming series by Casey Muratori

## sim86
A full decoder of 16-bit 8086 instructions. Is also able to actually execute and emulate a tiny subset of the x86 instruction set. The segmented memory model is also fully simulated.

Large parts of the actual decoding of instructions was heavily refactored to closely match the reference implementation given for the course.

The decoder can successfully runs all listings (challenges included) except for listing 55, where only the rectangle and top of bottom lines are painted. This is due to the fact that the left and right lines are addressed using the `bx` register, whereas all other pixels are addressed using the `bp` register. As my implementation simulates segmented memory, the vertical lines end up being written to the data segment, while the rest of the image is written to the stack segment.

To fix this, modify the two lines of listing 2:
```x86asm
mov byte [bx + 1], 255 ; Left line
mov byte [bx + 61*4 + 1], 255 ; Right  line
```
To override the default segment (`ds`) to `ss`
```x86asm
mov byte ss:[bx + 1], 255 ; Left line
mov byte ss:[bx + 61*4 + 1], 255 ; Right  line
```