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

extern char *conv_entity(int ch);
extern int getescapechar(char **s);
extern char *getescapecmd(char **s);
extern char *allocStr(const char *s, int len);
extern int strCmp(const void *s1, const void *s2);
extern char *currentdir(void);
extern char *cleanupName2(char *name, int flag);
#define cleanupName(name) cleanupName2((name), TRUE)
extern char *expandPath(char *name);
extern char *strcasestr(char *s1, char *s2);
extern int strcasemstr(char *str, char *srch[], char **ret_ptr);
extern char *remove_space(char *str);
extern int non_null(char *s);
extern void cleanup_line(Str s, int mode);
extern char *html_quote_char(char c);
extern char *html_quote(char *str);
extern char *html_unquote(char *str);
extern char *file_quote(char *str);
extern char *file_unquote(char *str);
extern char *url_quote(char *str);
extern char *url_unquote(char *str);
extern Str Str_form_quote(Str x);
extern Str Str_form_unquote(Str x);
extern char *shell_quote(char *str);

#define New(type)	((type*)GC_MALLOC(sizeof(type)))
#define NewAtom(type)	((type*)GC_MALLOC_ATOMIC(sizeof(type)))
#define New_N(type,n)	((type*)GC_MALLOC((n)*sizeof(type)))
#define NewAtom_N(type,n)	((type*)GC_MALLOC_ATOMIC((n)*sizeof(type)))
#define New_Reuse(type,ptr,n)   ((type*)GC_REALLOC((ptr),(n)*sizeof(type)))

#endif				/* INDEP_H */
