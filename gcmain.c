#ifndef GC_MAIN
#define GC_MAIN

#if defined(AIX) || defined(linux)
/* to cope with Boehm GC... */

#define MAIN real_main

#if defined(DEBIAN)
#include "gc/private/gc_priv.h"
#else
#include "private/gc_priv.h"
#endif
int real_main(int, char **, char **);

int
main(int argc, char **argv, char **envp)
{
    int dummy;
    GC_stackbottom = (ptr_t) (&dummy);
    return (real_main(argc, argv, envp));
}
#else
#define MAIN main
#endif

#endif
