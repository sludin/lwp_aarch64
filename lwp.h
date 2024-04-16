#ifndef LWPH
#define LWPH
#include <sys/types.h>

#include <assert.h>
#include <stddef.h>

#if defined(__aarch64__)
  #define LWP_ARM64
#elif defined(__x86_64__) || defined(_M_X64)
  #define LWP_X64
#else
    #error "Target architecture is not supported."
#endif


#if defined(__x86_64)
#include <fp.h>
typedef struct __attribute__ ((aligned(16))) __attribute__ ((packed))
registers {
  unsigned long rax;            /* the sixteen architecturally-visible regs. */
  unsigned long rbx;
  unsigned long rcx;
  unsigned long rdx;
  unsigned long rsi;
  unsigned long rdi;
  unsigned long rbp;
  unsigned long rsp;
  unsigned long r8;
  unsigned long r9;
  unsigned long r10;
  unsigned long r11;
  unsigned long r12;
  unsigned long r13;
  unsigned long r14;
  unsigned long r15;
  struct fxsave fxsave;   /* space to save floating point state */
} rfile;
#else
typedef unsigned long long Register;

static_assert(sizeof(Register) == 8, "Incorrect register size");

#define CHECK_OFFSET(REGISTER, OFFSET)    \
    static_assert(offsetof(struct registers, REGISTER) == OFFSET, \
                  "Incorrect register offset")

#define LWP_REG_X0    0x00
#define LWP_REG_X1    0x08
#define LWP_REG_X19   0x10
#define LWP_REG_X20   0x18
#define LWP_REG_X21   0x20
#define LWP_REG_X22   0x28
#define LWP_REG_X23   0x30
#define LWP_REG_X24   0x38
#define LWP_REG_X25   0x40
#define LWP_REG_X26   0x48
#define LWP_REG_X27   0x50
#define LWP_REG_X28   0x58
#define LWP_REG_X29   0x60
#define LWP_REG_X30   0x68
#define LWP_REG_V8    0x70
#define LWP_REG_V9    0x78
#define LWP_REG_V10   0x80
#define LWP_REG_V11   0x88
#define LWP_REG_V12   0x90
#define LWP_REG_V13   0x98
#define LWP_REG_V14   0xa0
#define LWP_REG_V15   0xa8
#define LWP_REG_SP    0xb0

typedef struct __attribute__ ((aligned(16))) __attribute__ ((packed))
registers {
    Register    x0;
    Register    x1;
    Register    x19;
    Register    x20;
    Register    x21;
    Register    x22;
    Register    x23;
    Register    x24;
    Register    x25;
    Register    x26;
    Register    x27;
    Register    x28;
    Register    x29;   
    Register    x30;   
    Register    v8;
    Register    v9;
    Register    v10;
    Register    v11;
    Register    v12;
    Register    v13;
    Register    v14;
    Register    v15;
    Register    sp;
} rfile;


// Making sure our offsets are all correct
CHECK_OFFSET(x0,  LWP_REG_X0); 
CHECK_OFFSET(x1,  LWP_REG_X1); 
CHECK_OFFSET(x19, LWP_REG_X19);
CHECK_OFFSET(x20, LWP_REG_X20);
CHECK_OFFSET(x21, LWP_REG_X21);
CHECK_OFFSET(x22, LWP_REG_X22);
CHECK_OFFSET(x23, LWP_REG_X23);
CHECK_OFFSET(x24, LWP_REG_X24);
CHECK_OFFSET(x25, LWP_REG_X25);
CHECK_OFFSET(x26, LWP_REG_X26);
CHECK_OFFSET(x27, LWP_REG_X27);
CHECK_OFFSET(x28, LWP_REG_X28);
CHECK_OFFSET(x29, LWP_REG_X29);
CHECK_OFFSET(x30, LWP_REG_X30);
CHECK_OFFSET(v8,  LWP_REG_V8);
CHECK_OFFSET(v9,  LWP_REG_V9);
CHECK_OFFSET(v10, LWP_REG_V10);
CHECK_OFFSET(v11, LWP_REG_V11);
CHECK_OFFSET(v12, LWP_REG_V12);
CHECK_OFFSET(v13, LWP_REG_V13);
CHECK_OFFSET(v14, LWP_REG_V14);
CHECK_OFFSET(v15, LWP_REG_V15);
CHECK_OFFSET(sp,  LWP_REG_SP);

// Both x64 and arm64 require stack memory pointer to be 16 bytes aligned
#define LWP_STACK_ALIGNMENT    16

#endif

typedef unsigned long tid_t;
#define NO_THREAD 0             /* an always invalid thread id */

typedef struct threadinfo_st *thread;
typedef struct threadinfo_st {
  tid_t         tid;            /* lightweight process id  */
  unsigned long *stack;         /* Base of allocated stack */
  size_t        stacksize;      /* Size of allocated stack */
  rfile         state;          /* saved registers         */
  unsigned int  status;         /* exited? exit status?    */
  thread        lib_one;        /* Two pointers reserved   */
  thread        lib_two;        /* for use by the library  */
  thread        sched_one;      /* Two more for            */
  thread        sched_two;      /* schedulers to use       */
  thread        exited;         /* and one for lwp_wait()  */
} context;

typedef int (*lwpfun)(void *);  /* type for lwp function */

/* Tuple that describes a scheduler */
typedef struct scheduler {
  void   (*init)(void);            /* initialize any structures     */
  void   (*shutdown)(void);        /* tear down any structures      */
  void   (*admit)(thread new);     /* add a thread to the pool      */
  void   (*remove)(thread victim); /* remove a thread from the pool */
  thread (*next)(void);            /* select a thread to schedule   */
  int    (*qlen)(void);            /* number of ready threads       */
} *scheduler;

/* lwp functions */
/**
 * Creates a new lightweight process. It executes the given function with the given
 * argument. lwp_create() returns the thread if of the new thread or NO_THREAD if the
 * thread cannot be created.
 *
 * @param  entry An int (*lwpfun)(void *) that is the entry point for the thread
 * @param  arg   An argument that willbe passed to the entry function
 * @return       The lightweight thread id
 */
extern tid_t lwp_create(lwpfun,void *);

/**
 * Called by a thread to indicate they are done.
 *
 * @param  status The returned status code
 */
extern void  lwp_exit(int status);

/**
 * Return the thread id of the current/calling thread
 *
 * @return  The thread id
 */
extern tid_t lwp_gettid(void);

/**
 * Called by a thread to suspend its execution.  The schdeuler will return the
 * next thread for execution and control will be passed to it.
 *
 */
extern void  lwp_yield(void);

/**
 * Starts the LWP system.Converts the calling thread into a LWP and
 * lwp_yield()s to whichever thread the scheduler chooses
 */
extern void  lwp_start(void);

/**
 * Waits for a thread to terminate, deallocates its resources, and reports
 * its termincation status id status is non-NULL.  Returns the tid of the
 * terminated therad or NO_THREAD.
 * @param status Pointer where to store returned status, or NULL
 * @return Thread id of the terminated thread
 */
extern tid_t lwp_wait(int *);

/**
 * Causes the LWP package to use the given scheduler to choose the next process to
 * run.  Transfers all thread from the old scheduler to the new one in next() order.
 * If schduler is NULL, the library swill return to round robin schduling.
 * @param fun Pointer to a structure of function pointers for the scheduler
 */
extern void  lwp_set_scheduler(scheduler fun);

/**
 * Returns the pointer to the current scheduler
 * @return Pointer to the current scheduler
 */
extern scheduler lwp_get_scheduler(void);

/**
 * Returns the thread corresponding to the given thread ID, or NULL if the ID
 * is invalid.
 * @param Thread it to look up
 * @return Pointer to thead with id tio
 */
extern thread tid2thread(tid_t tid);

/* for lwp_wait */
#define TERMOFFSET        8
#define MKTERMSTAT(a,b)   ( (a)<<TERMOFFSET | ((b) & ((1<<TERMOFFSET)-1)) )
#define LWP_TERM          1
#define LWP_LIVE          0
#define LWPTERMINATED(s)  ( (((s)>>TERMOFFSET)&LWP_TERM) == LWP_TERM )
#define LWPTERMSTAT(s)    ( (s) & ((1<<TERMOFFSET)-1) )

/* prototypes for asm functions */
void swap_rfiles(rfile *old, rfile *new);
void _lwp_wrapper(void);

void dump_stats( void );

#endif
