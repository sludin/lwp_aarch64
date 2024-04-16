# ARM Magic

Unlike X86_64, ARM64/AArch64 does not store the return address on the stack
and implcitly pop it when you ret. It puts the return address in `x30` (lr)
and the frame pointer in `x29` (fp) which is pushed and popped on the stack
during function's prologue and postlude. So it is harder to just trick the
processor into 'returning' somewhere else.  In the below code, the magic
for ARM64 will end up correctly branching to `lwp_wrapper`, but as `x30` will
be `lwp_wrapper`, `lwp_wrapper` will return to itself. One way out of this is to
declare that the function that is passed in `x30` must not return. In our case
it calls `lwp_exit` which will initiate a context switch, so it works.

You could play games with making the fiber entry points be naked and writing
custom preludes and postludes, but I think that is unnecessary in this case
and WAY out of scope for an OS class, even a senior level one.  Though I'd
love to see CS/CPE students coming out of school with the skills.

Also... As the assignment will need to be run on the cal poly linux/unix
server, the students who want to use their ARM macs will have to write
`lwp_create` code for both AArch64 and X86_64. Maybe give
them the ARM snippet to help them on their laptops during development and
leave the linux version as the exercise.

## ARM64 ABI Conventions
- `sp`, `x29`, and `x30` are stack pointer, frame pointer and return address respectively
- `x0-7` are function arguments ( magic only supports `x0` and `x1` at the moment )

## Resources

- [AArch64/ARM64 Tutorial](https://mariokartwii.com/armv8/)  
- [OSX Calling conventions](https://github.com/apple/swift/blob/main/docs/ABI/CallConvSummary.rst#arm64.)  
- [Fiber in C++: Understanding the Basics](https://agraphicsguynotes.com/posts/fiber_in_cpp_understanding_the_basics/#arm64-architecture) This _might_ make it too simple for students though it is a google search away  
- [Introduction to ARM64v8](https://book.hacktricks.xyz/macos-hardening/macos-security-and-privilege-escalation/macos-apps-inspecting-debugging-and-fuzzing/arm64-basic-assembly)  
- [Writing ARM64 code for Apple platforms](https://developer.apple.com/documentation/xcode/writing-arm64-code-for-apple-platforms)  

## Register MAP

  | ARM64 Register map | |
  | -------------------- | ------ |
  |`x0-x7`|    Parameters and results |
  |`x8`|       indirect result register (just ignore for our purposes) |
  |`x9-x15`|   Caller saved registers |
  |`x16-x17`|  prologe scratch registers |
  |`x18`|      Reserved - platform defiened register |
  |`x19-x28`|  Callee saved registers |
  |`x29 (fp)`| Frame pointer |
  |`x30 (lr)`| link register ( return address ) |
  |`v0-v7`|    Floating point / vector args and return ( maybe v0 and v1 should be saved in some cases? ) |
  |`v8-v15`|   callee saved registers |
  |`v16-v31`|  scratch registers ( caller saved ) |
  |`sp`|       stack pointer |
   
We will save and restore: `x19-x30`, `sp`, `v8-v15` ( 64 bit version d8-15), and `x0-x1`

## Inpiration and code ideas
- [tiny_fiber.h](https://gist.github.com/JiayinCao/07475d3423952b702d1efc5268b0df4e)
- [minicoro](https://github.com/edubart/minicoro/tree/main)

  
