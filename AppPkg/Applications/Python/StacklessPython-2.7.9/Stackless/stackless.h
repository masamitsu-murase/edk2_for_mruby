#ifndef Py_STACKLESS_H
#define Py_STACKLESS_H
#ifdef __cplusplus
extern "C" {
#endif

/* this option might go into a global config.
   It says "we want to compile stackless", but it
   is turned off again if our platform doesn't
   implement it.
 */

/****************************************************************************

  Stackless Internal Configuration

  Some preliminary description can be found in the draft document readme.txt

  The configuration is dependant of one #define:
  STACKLESS means that we want to create a stackless version.
  Without STACKLESS set, you will get the standard C-Python(r) application.
  If STACKLESS is set, the necessary fields and initializations are
  expanded, provided that the platform/compiler is supported by special
  header files. They implement the critical assembly part.
  If your platform is configured in this file but not supported,
  you will get a compiler error.

 ****************************************************************************/

/*
 * every platform needs to define its own interface here.
 * If this isn't defined, stackless is simply not compiled in.
 * Repeat the following sequence for every platform you support...
 * ...and then write your support code and mention it in the
 * common slp_platformselect.h file.
 */
#ifndef STACKLESS
#define STACKLESS
#endif

#ifdef STACKLESS_OFF
#undef STACKLESS
    /* an option to switch it off */
#elif defined(STACKLESS_MSFT_IA32)
    /* MS Visual Studio on X86 */
#elif defined(STACKLESS_MSFT_X64)
    /* microsoft on 64 bit x64 thingies */
#elif defined(STACKLESS_GCC_IA32)
    /* gcc on X86 */
#elif defined(STACKLESS_GCC_X64)
    /* gcc on AMD64 */
#else
    /* no supported platform */
#undef STACKLESS
#endif

#ifdef __cplusplus
}
#endif
#endif /* Py_STACKLESS_H */
