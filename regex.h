#define REGEX_MAX	64
#define STORAGE_MAX	256

#ifndef NULL
#define NULL	0
#endif				/* not NULL */

#define	RE_NORMAL	0
#define RE_MATCHMODE	0x07
#define RE_ANY		0x01
#define RE_WHICH	0x02
#define RE_EXCEPT	0x04
#define RE_ANYTIME	0x08
#define RE_BEGIN	0x10
#define RE_END		0x20
#define RE_IGNCASE      0x40
#define RE_ENDMARK	0x80

typedef unsigned short longchar;


typedef struct {

    longchar *pattern;

    unsigned char mode;

} regexchar;


typedef struct {

    regexchar re[REGEX_MAX];

    longchar storage[STORAGE_MAX];

    char *position;

    char *lposition;

} Regex;


Regex *newRegex(char *ex, int igncase, Regex * regex, char **error_msg);

int RegexMatch(Regex * re, char *str, int len, int firstp);

void MatchedPosition(Regex * re, char **first, char **last);


/* backward compatibility */
char *regexCompile(char *ex, int igncase);

int regexMatch(char *str, int len, int firstp);

void matchedPosition(char **first, char **last);
