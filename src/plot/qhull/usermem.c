/*<html><pre>  -<a                             href="qh-user.htm"
  >-------------------------------</a><a name="TOP">-</a>

   usermem.c
   qh_exit(), qh_free(), and qh_malloc()

   See README.txt.

   If you redefine one of these functions you must redefine all of them.
   If you recompile and load this file, then usermem.o will not be loaded
   from qhull.a or qhull.lib

   See libqhull.h for data structures, macros, and user-callable functions.
   See user.c for qhull-related, redefinable functions
   see user.h for user-definable constants
   See userprintf.c for qh_fprintf and userprintf_rbox.c for qh_fprintf_rbox

   Please report any errors that you fix to qhull@qhull.org
*/

#include "libqhull.h"

#include <stdarg.h>
#include <stdlib.h>

/*-<a                             href="qh-user.htm#TOC"
  >-------------------------------</a><a name="qh_exit">-</a>

  qh_exit( exitcode )
    exit program

  notes:
    qh_exit() is called when qh_errexit() and longjmp() are not available.

    This is the only use of exit() in Qhull
    To replace qh_exit with 'throw', see libqhullcpp/usermem_r-cpp.cpp
*/
void qh_exit(int exitcode) {
    exit(exitcode);
} /* exit */

/*-<a                             href="qh-user.htm#TOC"
  >-------------------------------</a><a name="qh_fprintf_stderr">-</a>

  qh_fprintf_stderr( msgcode, format, list of args )
    fprintf to stderr with msgcode (non-zero)

  notes:
    qh_fprintf_stderr() is called when qh.ferr is not defined, usually due to an initialization error

    It is typically followed by qh_errexit().

    Redefine this function to avoid using stderr

    Use qh_fprintf [userprintf.c] for normal printing
*/
void qh_fprintf_stderr(int msgcode, const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    if (msgcode)
        fprintf(stderr, "QH%.4d ", msgcode);
    vfprintf(stderr, fmt, args);
    va_end(args);
} /* fprintf_stderr */

/*-<a                             href="qh-user.htm#TOC"
>-------------------------------</a><a name="qh_free">-</a>

  qh_free( mem )
    free memory

  notes:
    same as free()
    No calls to qh_errexit()
*/
void qh_free(void *mem) {
    free(mem);
} /* free */

/*-<a                             href="qh-user.htm#TOC"
    >-------------------------------</a><a name="qh_malloc">-</a>

    qh_malloc( mem )
      allocate memory

    notes:
      same as malloc()
*/
void *qh_malloc(size_t size) {
    return malloc(size);
} /* malloc */


