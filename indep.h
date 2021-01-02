/* $Id: indep.h,v 1.16 2003/09/22 21:02:19 ukai Exp $ */
#ifndef INDEP_H
#define INDEP_H
#include "alloc.h"
#include "Str.h"
#include "config.h"

#ifndef TRUE
#define TRUE 1
#endif				/* TRUE */
#ifndef FALSE
#define FALSE 0
#endif				/* FALSE */

struct growbuf {
    char *ptr;
    int length;
    int area_size;
    void *(*realloc_proc) (void *, size_t);
    void (*free_proc) (void *);
};

#define RAW_MODE	0
#define PAGER_MODE	1
#define HTML_MODE	2
#define HEADER_MODE	3

extern unsigned char QUOTE_MAP[];
extern char *HTML_QUOTE_MAP[];
#define HTML_QUOTE_MASK   0x07	/* &, <, >, ", ' */
#define SHELL_UNSAFE_MASK 0x08	/* [^A-Za-z0-9_./:\200-\377] */
#define URL_QUOTE_MASK    0x10	/* [\0- \177-\377] */
#define FILE_QUOTE_MASK   0x30	/* [\0- #%&+:?\177-\377] */
#define URL_UNSAFE_MASK   0x70	/* [^A-Za-z0-9_$\-.] */
#define GET_QUOTE_TYPE(c) QUOTE_MAP[(int)(unsigned char)(c)]
#define is_html_quote(c)   (GET_QUOTE_TYPE(c) & HTML_QUOTE_MASK)
#define is_shell_unsafe(c) (GET_QUOTE_TYPE(c) & SHELL_UNSAFE_MASK)
#define is_url_quote(c)    (GET_QUOTE_TYPE(c) & URL_QUOTE_MASK)
#define is_file_quote(c)   (GET_QUOTE_TYPE(c) & FILE_QUOTE_MASK)
#define is_url_unsafe(c)   (GET_QUOTE_TYPE(c) & URL_UNSAFE_MASK)
#define html_quote_char(c) HTML_QUOTE_MAP[(int)is_html_quote(c)]

extern clen_t strtoclen(const char *s);
extern char *conv_entity(unsigned int ch);
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
int strmatchlen(const char *s1, const char *s2, int maxlen);
extern char *remove_space(char *str);
extern int non_null(char *s);
extern void cleanup_line(Str s, int mode);
extern char *html_quote(char *str);
extern char *html_unquote(char *str);
extern char *file_quote(char *str);
extern char *file_unquote(char *str);
extern char *url_quote(char *str);
extern Str Str_url_unquote(Str x, int is_form, int safe);
extern Str Str_form_quote(Str x);
#define Str_form_unquote(x) Str_url_unquote((x), TRUE, FALSE)
extern char *shell_quote(char *str);
#define xmalloc(s) xrealloc(NULL, s)
extern void *xrealloc(void *ptr, size_t size);
extern void xfree(void *ptr);
extern void *w3m_GC_realloc_atomic(void *ptr, size_t size);
extern void w3m_GC_free(void *ptr);
extern void growbuf_init(struct growbuf *gb);
extern void growbuf_init_without_GC(struct growbuf *gb);
extern void growbuf_clear(struct growbuf *gb);
extern Str growbuf_to_Str(struct growbuf *gb);
extern void growbuf_reserve(struct growbuf *gb, int leastarea);
extern void growbuf_append(struct growbuf *gb, const char *src, int len);
#define GROWBUF_ADD_CHAR(gb,ch) ((((gb)->length>=(gb)->area_size)?growbuf_reserve(gb,(gb)->length+1):(void)0),(void)((gb)->ptr[(gb)->length++] = (ch)))

extern char *w3m_auxbin_dir();
extern char *w3m_lib_dir();
extern char *w3m_etc_dir();
extern char *w3m_conf_dir();
extern char *w3m_help_dir();

#define NewWithoutGC(type)	((type*)xmalloc(sizeof(type)))
#define NewWithoutGC_N(type,n)	((type*)xmalloc((n)*sizeof(type)))
#define NewWithoutGC_Reuse(type,ptr,n)	((type*)xrealloc(ptr,(n)*sizeof(type)))

#endif				/* INDEP_H */
