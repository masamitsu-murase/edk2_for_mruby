/*
 * Platform Selection for Stackless
 */

#if defined(STACKLESS_MSFT_IA32)
#include "switch_x86_msvc.h"
#elif defined(STACKLESS_MSFT_X64)
#include "switch_x64_msvc.h"
#elif defined(STACKLESS_GCC_IA32)
#include "switch_x86_unix.h" /* gcc on X86 */
#elif defined(STACKLESS_GCC_X64)
#include "switch_amd64_unix.h" /* gcc on amd64 */
#endif

/* default definitions if not defined in above files */

/* adjust slots to typical size of a few recursions on your system */

#ifndef CSTACK_SLOTS
#define CSTACK_SLOTS        1024
#endif

/* how many cstacks to cache at all */

#ifndef CSTACK_MAXCACHE
#define CSTACK_MAXCACHE     100
#endif

/* a good estimate how much the cstack level differs between
   initialisation and main C-Python(r) code. Not critical, but saves time.
   Note that this will vanish with the greenlet approach. */

#ifndef CSTACK_GOODGAP
#define CSTACK_GOODGAP      4096
#endif

/* stack size in pointer to trigger stack spilling */

#ifndef CSTACK_WATERMARK
#define CSTACK_WATERMARK 16384
#endif

/* define direction of stack growth */

#ifndef CSTACK_DOWNWARDS
#define CSTACK_DOWNWARDS 1   /* 0 for upwards */
#endif

/**************************************************************

  Don't change definitions below, please.

 **************************************************************/

#if CSTACK_DOWNWARDS == 1
#define CSTACK_COMPARE(a, b) (a) < (b)
#define CSTACK_SUBTRACT(a, b) (a) - (b)
#else
#define CSTACK_COMPARE(a, b) (a) > (b)
#define CSTACK_SUBTRACT(a, b) (b) - (a)
#endif

#define CSTACK_SAVE_NOW(tstate, stackvar) \
	((tstate)->st.cstack_root != NULL ? \
	 CSTACK_SUBTRACT((tstate)->st.cstack_root, \
	 (intptr_t*)&(stackvar)) > CSTACK_WATERMARK : 1)
