/* $Id: regex.h,v 1.4 2002/01/10 04:55:07 ukai Exp $ */
#define REGEX_MAX	64
#define STORAGE_MAX	256


typedef unsigned short longchar;

typedef struct regexchar {
    union {
	longchar *pattern;
	struct regex *sub;
    } p;
    unsigned char mode;
} regexchar;


typedef struct regex {
    regexchar re[REGEX_MAX];
    longchar storage[STORAGE_MAX];
    char *position;
    char *lposition;
    struct regex *alt_regex;
} Regex;


Regex *newRegex(char *ex, int igncase, Regex *regex, char **error_msg);

int RegexMatch(Regex *re, char *str, int len, int firstp);

void MatchedPosition(Regex *re, char **first, char **last);


/* backward compatibility */
char *regexCompile(char *ex, int igncase);

int regexMatch(char *str, int len, int firstp);

void matchedPosition(char **first, char **last);
