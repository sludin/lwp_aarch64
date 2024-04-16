  
#if defined(__aarch64__)
  #define LWP_ARM64
#elif defined(__x86_64__) || defined(_M_X64)
  #define LWP_X64
#else
  #error "Target architecture is not supported."
#endif


  
#ifdef LWP_X64
  
.text
#ifdef __APPLE__
.globl _swap_rfiles
_swap_rfiles:
#else
.globl swap_rfiles
.type swap_rfiles, @function
swap_rfiles:
#endif

  # void swap_rfiles(rfile *old, rfile *new)
  # 
  # "old" will be in rdi
  # "new" will be in rsi
  #
  pushq %rbp    # set up a frame pointer
  movq %rsp,%rbp
  
  # save the old context (if old != NULL)
  cmpq  $0,%rdi
  je load

  movq %rax,   (%rdi) # store rax into old->rax so we can use it

  # Now store the Floating Point State
  leaq 128(%rdi),%rax # get the address
  fxsave (%rax) 

  movq %rbx,  8(%rdi) # now the rest of the registers
  movq %rcx, 16(%rdi) # etc.
  movq %rdx, 24(%rdi)
  movq %rsi, 32(%rdi)
  movq %rdi, 40(%rdi)
  movq %rbp, 48(%rdi) 
  movq %rsp, 56(%rdi)
  movq %r8,  64(%rdi)
  movq %r9,  72(%rdi)
  movq %r10, 80(%rdi)
  movq %r11, 88(%rdi)
  movq %r12, 96(%rdi)
  movq %r13,104(%rdi)
  movq %r14,112(%rdi)
  movq %r15,120(%rdi)

  # load the new one (if new != NULL)
load: cmpq  $0,%rsi
  je done

  # First restore the Floating Point State
  leaq 128(%rsi),%rax # get the address
  fxrstor (%rax)
  
  movq    (%rsi),%rax # retreive rax from new->rax
  movq   8(%rsi),%rbx # etc.
  movq  16(%rsi),%rcx
  movq  24(%rsi),%rdx
  movq  40(%rsi),%rdi
  movq  48(%rsi),%rbp
  movq  56(%rsi),%rsp
  movq  64(%rsi),%r8 
  movq  72(%rsi),%r9 
  movq  80(%rsi),%r10
  movq  88(%rsi),%r11
  movq  96(%rsi),%r12
  movq 104(%rsi),%r13
  movq 112(%rsi),%r14
  movq 120(%rsi),%r15
  movq  32(%rsi),%rsi # must do rsi last, since it's our pointer

done: leave
  ret
  
#else /* ARM64 */


.text
#ifdef __APPLE__
.globl _swap_rfiles
_swap_rfiles:
#else
.globl swap_rfiles
.type swap_rfiles, @function
swap_rfiles:
#endif

# void swap_rfiles(rfile *old, rfile *new)
#
# "old" will be in x0
# "new" will be in x1
#

  # ARM64 Register map:
  #   x0-x7:    Parameters and results
  #   x8:       indirect result register (just ignore for our purposes)
  #   x9-x15:   Caller saved registers
  #   x16-x17:  prologe scratch registers
  #   x18:      Reserved - platform defiened register
  #   x19-x28:  Callee saved registers
  #   x29 (fp): Frame pointer
  #   x30 (lr): link register ( return address )
  #   v0-v7:    Floating point / vector args and return ( maybe v0 and v1 should be saved in some cases? )
  #   v8-v15:   callee saved registers
  #   v16-v31:  scratch registers ( caller saved )
  #   sp:       stack pointer
  #
  # We will save and restore: x19-x30, sp, v8-v15 ( 64 bit version d8-15), and x0-x1
  #
  cmp x0, 0
  beq load

  stp x19, x20, [x0, 0x10] 
  stp x21, x22, [x0, 0x20]
  stp x23, x24, [x0, 0x30]
  stp x25, x26, [x0, 0x40]
  stp x27, x28, [x0, 0x50]
  stp x29, x30, [x0, 0x60]
  stp d8,  d9,  [x0, 0x70]
  stp d10, d11, [x0, 0x80]
  stp d12, d13, [x0, 0x90]
  stp d14, d15, [x0, 0xa0]
  mov x2, sp
  str x2,       [x0, 0xb0]

load:
  cmp x1, 0
  beq done
  
  ldp x19, x20, [x1, 0x10] 
  ldp x21, x22, [x1, 0x20]
  ldp x23, x24, [x1, 0x30]
  ldp x25, x26, [x1, 0x40]
  ldp x27, x28, [x1, 0x50]
  ldp x29, x30, [x1, 0x60]
  ldp d8,  d9,  [x1, 0x70]
  ldp d10, d11, [x1, 0x80]
  ldp d12, d13, [x1, 0x90]
  ldp d14, d15, [x1, 0xa0]
  ldr x2,       [x1, 0xb0]
  mov sp, x2
  ldp x0, x1,   [x1, 0x00] // paramters for entry point

done: 
  br x30


#endif


