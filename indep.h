#ifndef INDEP_H
#define INDEP_H
#include "gc.h"
#include "Str.h"

#ifndef TRUE
#define TRUE 1
#endif				/* TRUE */
#ifndef FALSE
#define FALSE 0
#endif				/* FALSE */

#define PAGER_MODE	0
#define HTML_MODE	1
#define HEADER_MODE	2

extern char *conv_latin1(int ch);
extern int getescapechar(char **s);
extern char *getescapecmd(char **s);
extern char *allocStr(const char *s, int len);
extern int strCmp(const void *s1, const void *s2);
extern void copydicname(char *s, char *fn);
extern char *currentdir(void);
extern char *cleanupName(char *name);
extern char *strcasestr(char *s1, char *s2);
extern int strcasemstr(char *str, char *srch[], char **ret_ptr);
extern char *cleanup_str(char *s);
extern char *remove_space(char *str);
extern char *htmlquote_char(char c);
extern char *htmlquote_str(char *str);
extern Str form_quote(Str x);
extern Str form_unquote(Str x);
extern char *expandPath(char *name);
extern int non_null(char *s);
extern void cleanup_line(Str s, int mode);

#define New(type)	((type*)GC_MALLOC(sizeof(type)))
#define NewAtom(type)	((type*)GC_MALLOC_ATOMIC(sizeof(type)))
#define New_N(type,n)	((type*)GC_MALLOC((n)*sizeof(type)))
#define NewAtom_N(type,n)	((type*)GC_MALLOC_ATOMIC((n)*sizeof(type)))
#define New_Reuse(type,ptr,n)   ((type*)GC_REALLOC((ptr),(n)*sizeof(type)))

#endif				/* INDEP_H */
