/* $Id: indep.h,v 1.11 2003/01/08 17:24:12 ukai Exp $ */
#ifndef INDEP_H
#define INDEP_H
#include "gc.h"
#include "Str.h"
#include "config.h"

#ifndef TRUE
#define TRUE 1
#endif				/* TRUE */
#ifndef FALSE
#define FALSE 0
#endif				/* FALSE */

#define PAGER_MODE	0
#define HTML_MODE	1
#define HEADER_MODE	2

extern clen_t strtoclen(const char *s);
extern char *conv_entity(int ch);
extern int getescapechar(char **s);
extern char *getescapecmd(char **s);
extern char *allocStr(const char *s, int len);
extern int strCmp(const void *s1, const void *s2);
extern char *currentdir(void);
extern char *cleanupName(char *name);
extern char *expandPath(char *name);
#ifndef HAVE_STRCHR
extern char *strchr(const char *s, int c);
#endif				/* not HAVE_STRCHR */
#ifndef HAVE_STRCASECMP
extern int strcasecmp(const char *s1, const char *s2);
extern int strncasecmp(const char *s1, const char *s2, size_t n);
#endif				/* not HAVE_STRCASECMP */
#ifndef HAVE_STRCASESTR
extern char *strcasestr(const char *s1, const char *s2);
#endif
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
extern Str Str_url_unquote(Str x, int is_form);
extern Str Str_form_quote(Str x);
#define Str_form_unquote(x) Str_url_unquote((x), TRUE)
extern char *shell_quote(char *str);

extern char *w3m_auxbin_dir();
extern char *w3m_lib_dir();
extern char *w3m_etc_dir();
extern char *w3m_help_dir();

#define New(type)	((type*)GC_MALLOC(sizeof(type)))
#define NewAtom(type)	((type*)GC_MALLOC_ATOMIC(sizeof(type)))
#define New_N(type,n)	((type*)GC_MALLOC((n)*sizeof(type)))
#define NewAtom_N(type,n)	((type*)GC_MALLOC_ATOMIC((n)*sizeof(type)))
#define New_Reuse(type,ptr,n)   ((type*)GC_REALLOC((ptr),(n)*sizeof(type)))

#endif				/* INDEP_H */
