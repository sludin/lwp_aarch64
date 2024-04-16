

void lwp_wrapper( lwpfun entry, void *arg );


tid_t lwp_create( lwpfun entry, void *arg ) {

        
  /* Set up the thread context */
  thread t        = allocate_thread();
  t->stacksize    = g_stackwords;                      /* In words */
  t->tid          = g_nextid++;
  t->status       = MKTERMSTAT( LWP_LIVE, 0 );
  
#ifdef LWP_X64 

  /* TODO: Implement the X86_64 context handling */
  
#endif
  
#ifdef LWP_ARM64

  /* Unlike X86_64, ARM64v8/AArch64 does not store the return address on the stack
   * and implcitly pop it when you ret. It puts the return address in x30 (lr)
   * and the frame pointer in X29 (fp). So it is harder to just trick the
   * processor into 'returning' somewhere else.  In the below code, the magic
   * for ARM64 will end up correctly branching to lwp_wrapper, but as x30 will
   * be lwp_wrapper, lwp_wrapper will return to itself. One way out of this is to
   * declare that the function that is passed in X30 must not return. In our case
   * it calls lwp_exit which will initiare a switch, so it works.
   *
   * You could play games with making the fiber entry points be naked and writing
   * custom preludes and postludes, but I think that is unnecessary in this case
   * and WAY out of scope for an OS class, even a senior level one.  Though I'd
   * love to see CS/CPE students coming out of school with the skills.
   *
   * Also... As the assingment will need to be run on the cal poly linux/unix
   * server, the students who want to user their M1/2/3 macs will have to write
   * something like the below code AND write the X86_64 code above. Maybe give
   * them the below snippet to help them on their laptops during development and
   * leave the linux version as the exercise.
   *  
   * Relevant ABI Notes:
   * - sp, x29, and x30 are stack pointer, frame pointer and return address respectively
   * - x0-7 are function arguments ( magic only supports x0 and x1 at the moment )
   *
   * Good resource: https://mariokartwii.com/armv8/
   * 
   * */
  word_t *stack_top = (word_t*)(t->stack + t->stacksize);  /* Points to the top of the stack */
  t->state.sp  = (unsigned long)stack_top;    /* sp points to the stack */
  t->state.x29 = (unsigned long)stack_top;    /* there is no previous frame pointer so it == sp */  
  t->state.x30 = (unsigned long)lwp_wrapper;  /* The function to swicth to.  In my case it is a wrapper */
  t->state.x0  = (unsigned long)entry;        /* The fiber entry point */
  t->state.x1  = (unsigned long)arg;          /* The argument for the fiber */
#endif


#ifdef LWP_ARM64_EXP

  /* Experimental code to try and trick the OS into changing the backtrace
     Use the above code to use LWP on your apple silicon mac, but feel free to
     play with these ideas  */
  word_t *stack_top = (word_t*)(t->stack + t->stacksize);  /* Points to the top of the stack */
  t->state.sp  = (unsigned long)stack_top;
  t->state.x29 = (unsigned long)stack_top;   /* This does not allow back chaining ATM */
  t->state.x30 = (unsigned long)_lwp_wrapper;
  t->state.x18 = (unsigned long)entry;
  t->state.x19 = (unsigned long)arg;
  t->state.x21 = (unsigned long)lwp_exit;
  t->state.x20 = (unsigned long)lwp_wrapper;
#endif  


void lwp_wrapper( lwpfun entry, void *arg ) {
  int rval = entry( arg );
  lwp_exit( rval );
}

/* Experimental wrapper to skip the prologue paired with the above
   experimental section.   */
#ifdef LWP_ARM64_EXP
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlanguage-extension-token"
void _lwp_wrapper( void ) {
  asm(
  "mov x0, x18\n"
  "mov x1, x19\n"
  "mov x30, x21\n"
  "br  x20"
      );
}
#pragma GCC diagnostic pop
#endif
